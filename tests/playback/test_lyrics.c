#include "check.h"
#include "memory_source.h"
#include "playback/lyrics.h"

#include <stdio.h>
#include <string.h>

static Lyrics lyrics;
static MemorySource memory;

static ByteSource load(const char *text, bool *loaded)
{
    memory = (MemorySource){.bytes = (const uint8_t *)text, .size = (uint32_t)strlen(text)};
    ByteSource source = memory_source(&memory);
    bool result = lyrics_load(&lyrics, source, memory.size);
    if (loaded != NULL) {
        *loaded = result;
    }
    return source;
}

static void line_is(ByteSource source, uint16_t index, uint32_t time_ms, const char *expected)
{
    char text[LYRICS_CAPACITY];
    lyrics_text(&lyrics, source, index, text, sizeof text);
    CHECK_EQUAL(time_ms, lyrics.lines[index].time_ms);
    CHECK_EQUAL(0, strcmp(expected, text));
}

static void an_empty_file_has_no_lyrics(void)
{
    bool loaded = true;
    load("", &loaded);
    CHECK(!loaded);
    CHECK_EQUAL(0, lyrics.count);
    CHECK_EQUAL(LYRICS_NONE, lyrics_line_at(&lyrics, 5000));
}

static void timed_lines_are_indexed_in_order(void)
{
    bool loaded = false;
    ByteSource source = load("[00:01.00]one\n[00:02.50]two\n[01:03.25]three\n", &loaded);
    CHECK(loaded);
    CHECK_EQUAL(3, lyrics.count);
    line_is(source, 0, 1000, "one");
    line_is(source, 1, 2500, "two");
    line_is(source, 2, 63250, "three");
}

static void fractions_of_one_two_and_three_digits_all_count(void)
{
    ByteSource source =
        load("[00:01.5]a\n[00:02.05]b\n[00:03.007]c\n[00:04]d\n[00:05:25]e\n", NULL);
    CHECK_EQUAL(5, lyrics.count);
    line_is(source, 0, 1500, "a");
    line_is(source, 1, 2050, "b");
    line_is(source, 2, 3007, "c");
    line_is(source, 3, 4000, "d");
    line_is(source, 4, 5250, "e");
}

static void lines_out_of_order_are_sorted_by_time(void)
{
    ByteSource source = load("[00:09.00]late\n[00:01.00]early\n[00:05.00]middle\n", NULL);
    line_is(source, 0, 1000, "early");
    line_is(source, 1, 5000, "middle");
    line_is(source, 2, 9000, "late");
}

static void a_line_with_several_stamps_repeats_for_each(void)
{
    ByteSource source = load("[00:10.00][00:30.00][00:20.00]chorus\n[00:05.00]verse\n", NULL);
    CHECK_EQUAL(4, lyrics.count);
    line_is(source, 0, 5000, "verse");
    line_is(source, 1, 10000, "chorus");
    line_is(source, 2, 20000, "chorus");
    line_is(source, 3, 30000, "chorus");
}

static void metadata_tags_are_not_lyrics(void)
{
    ByteSource source =
        load("[ti:A Title]\n[ar:An Artist]\n[by:someone]\n[00:01.00]sung\n[length: 03:20]\n", NULL);
    CHECK_EQUAL(1, lyrics.count);
    line_is(source, 0, 1000, "sung");
}

static void an_offset_tag_is_ignored_like_any_other_metadata(void)
{
    ByteSource source = load("[offset:+500]\n[00:01.00]one\n", NULL);
    CHECK_EQUAL(1, lyrics.count);
    line_is(source, 0, 1000, "one");
}

static void line_endings_and_a_byte_order_mark_are_handled(void)
{
    ByteSource source = load("\xEF\xBB\xBF[00:01.00]crlf\r\n[00:02.00]cr\r[00:03.00]lf\n", NULL);
    CHECK_EQUAL(3, lyrics.count);
    line_is(source, 0, 1000, "crlf");
    line_is(source, 1, 2000, "cr");
    line_is(source, 2, 3000, "lf");
}

static void lines_without_a_stamp_are_ignored(void)
{
    ByteSource source = load("just text\n\n   \n[00:01.00]sung\nmore text\n", NULL);
    CHECK_EQUAL(1, lyrics.count);
    line_is(source, 0, 1000, "sung");
}

static void an_empty_timed_line_is_kept_to_clear_the_screen(void)
{
    ByteSource source = load("[00:01.00]sung\n[00:05.00]\n[00:09.00]again\n", NULL);
    CHECK_EQUAL(3, lyrics.count);
    line_is(source, 1, 5000, "");
}

static void a_bracket_inside_the_text_stays_in_the_text(void)
{
    ByteSource source = load("[00:01.00]sing [twice] now\n[00:02.00][x]odd\n", NULL);
    line_is(source, 0, 1000, "sing [twice] now");
    line_is(source, 1, 2000, "[x]odd");
}

static void the_current_line_follows_the_position(void)
{
    load("[00:02.00]a\n[00:04.00]b\n[00:06.00]c\n", NULL);
    CHECK_EQUAL(LYRICS_NONE, lyrics_line_at(&lyrics, 0));
    CHECK_EQUAL(LYRICS_NONE, lyrics_line_at(&lyrics, 1999));
    CHECK_EQUAL(0, lyrics_line_at(&lyrics, 2000));
    CHECK_EQUAL(0, lyrics_line_at(&lyrics, 3999));
    CHECK_EQUAL(1, lyrics_line_at(&lyrics, 4000));
    CHECK_EQUAL(2, lyrics_line_at(&lyrics, 600000));
}

static void leading_blanks_and_word_stamps_are_removed_from_the_text(void)
{
    ByteSource source = load("[00:01.00]   <00:01.10>Hel<00:01.40>lo  <b>x\n", NULL);
    line_is(source, 0, 1000, "Hello  <b>x");
}

static void chinese_japanese_and_korean_text_comes_back_whole(void)
{
    ByteSource source =
        load("[00:01.00]\xE4\xBD\xA0\xE5\xA5\xBD\n[00:02.00]\xEC\x95\x88\xEB\x85\x95\n", NULL);
    line_is(source, 0, 1000, "\xE4\xBD\xA0\xE5\xA5\xBD");
    line_is(source, 1, 2000, "\xEC\x95\x88\xEB\x85\x95");
}

static void a_short_buffer_never_cuts_a_character_in_half(void)
{
    ByteSource source = load("[00:01.00]\xE4\xBD\xA0\xE5\xA5\xBD\xE4\xB8\x96\n", NULL);
    char text[8];
    CHECK_EQUAL(6, lyrics_text(&lyrics, source, 0, text, sizeof text));
    CHECK_EQUAL(0, strcmp("\xE4\xBD\xA0\xE5\xA5\xBD", text));
    char tiny[3];
    CHECK_EQUAL(0, lyrics_text(&lyrics, source, 0, tiny, sizeof tiny));
    CHECK_EQUAL(0, tiny[0]);
}

static void a_line_longer_than_a_read_chunk_keeps_its_place(void)
{
    char file[1200];
    int used = snprintf(file, sizeof file, "[00:01.00]");
    for (int index = 0; index < 600; index++) {
        file[used++] = (char)('a' + index % 26);
    }
    snprintf(file + used, sizeof file - (size_t)used, "\n[00:02.00]after\n");
    ByteSource source = load(file, NULL);
    CHECK_EQUAL(2, lyrics.count);
    CHECK_EQUAL(600, lyrics.lines[0].length);
    char text[LYRICS_CAPACITY];
    CHECK_EQUAL(LYRICS_CAPACITY - 1, lyrics_text(&lyrics, source, 0, text, sizeof text));
    CHECK_EQUAL('a', text[0]);
    CHECK_EQUAL('z', text[25]);
    line_is(source, 1, 2000, "after");
}

static void a_huge_file_stops_at_the_capacity(void)
{
    static char file[LYRICS_CAPACITY * 24];
    size_t used = 0;
    for (int index = 0; index < LYRICS_CAPACITY + 40; index++) {
        used += (size_t)snprintf(file + used, sizeof file - used, "[%02d:%02d.00]line\n",
                                 index / 60, index % 60);
        if (used + 24 > sizeof file) {
            break;
        }
    }
    bool loaded = false;
    load(file, &loaded);
    CHECK(loaded);
    CHECK_EQUAL(LYRICS_CAPACITY, lyrics.count);
}

static void a_file_that_cannot_be_read_gives_no_lyrics(void)
{
    ByteSource source = load("[00:01.00]one\n", NULL);
    memory.failing = true;
    CHECK(!lyrics_load(&lyrics, source, memory.size));
    CHECK_EQUAL(0, lyrics.count);
}

static void asking_for_a_missing_line_gives_an_empty_text(void)
{
    ByteSource source = load("[00:01.00]one\n", NULL);
    char text[16] = "junk";
    CHECK_EQUAL(0, lyrics_text(&lyrics, source, 9, text, sizeof text));
    CHECK_EQUAL(0, text[0]);
}

int main(void)
{
    static const TestCase cases[] = {
        TEST_CASE(an_empty_file_has_no_lyrics),
        TEST_CASE(timed_lines_are_indexed_in_order),
        TEST_CASE(fractions_of_one_two_and_three_digits_all_count),
        TEST_CASE(lines_out_of_order_are_sorted_by_time),
        TEST_CASE(a_line_with_several_stamps_repeats_for_each),
        TEST_CASE(metadata_tags_are_not_lyrics),
        TEST_CASE(an_offset_tag_is_ignored_like_any_other_metadata),
        TEST_CASE(line_endings_and_a_byte_order_mark_are_handled),
        TEST_CASE(lines_without_a_stamp_are_ignored),
        TEST_CASE(an_empty_timed_line_is_kept_to_clear_the_screen),
        TEST_CASE(a_bracket_inside_the_text_stays_in_the_text),
        TEST_CASE(the_current_line_follows_the_position),
        TEST_CASE(leading_blanks_and_word_stamps_are_removed_from_the_text),
        TEST_CASE(chinese_japanese_and_korean_text_comes_back_whole),
        TEST_CASE(a_short_buffer_never_cuts_a_character_in_half),
        TEST_CASE(a_line_longer_than_a_read_chunk_keeps_its_place),
        TEST_CASE(a_huge_file_stops_at_the_capacity),
        TEST_CASE(a_file_that_cannot_be_read_gives_no_lyrics),
        TEST_CASE(asking_for_a_missing_line_gives_an_empty_text),
    };
    return test_run_all(cases, TEST_CASE_COUNT(cases));
}
