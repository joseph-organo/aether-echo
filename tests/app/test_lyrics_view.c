#include <string.h>

#include "check.h"
#include "fake_hal.h"
#include "fixture_library.h"
#include "lyrics_view.h"

static const char TRACK_PATH[] = "AC DC/Back In Black/1-02 - Shoot to Thrill.flac";
static const char LYRICS_PATH[] = "/AC DC/Back In Black/1-02 - Shoot to Thrill.lrc";
static const char LYRICS[] = "[ti:Shoot]\n[00:01.00]first\n[00:05.00]second\n[00:09.00]third\n";

static FixtureLibrary fixture;
static LyricsView view;

static uint16_t the_track(const Library *library)
{
    char path[LYRICS_VIEW_PATH_CAPACITY];
    uint32_t count = library_count(library, LIBRARY_TRACKS);
    for (uint32_t index = 0; index < count; index++) {
        LibraryTrack track;
        if (library_read_track(library, (uint16_t)index, &track) &&
            library_read_string(library, track.path, path, sizeof path) &&
            strcmp(path, TRACK_PATH) == 0) {
            return (uint16_t)index;
        }
    }
    return UINT16_MAX;
}

static const Library *fresh_library(void)
{
    fake_hal_reset();
    return fixture_library_open(&fixture);
}

static void a_track_with_a_lyrics_file_beside_it_loads_the_lines(void)
{
    const Library *library = fresh_library();
    fake_hal_add_file(LYRICS_PATH, (const uint8_t *)LYRICS, (uint32_t)strlen(LYRICS));
    lyrics_view_open(&view, library, the_track(library));
    CHECK(lyrics_view_ready(&view));
    CHECK_EQUAL(3, view.lyrics.count);
}

static void the_lines_follow_the_playback_position(void)
{
    const Library *library = fresh_library();
    fake_hal_add_file(LYRICS_PATH, (const uint8_t *)LYRICS, (uint32_t)strlen(LYRICS));
    lyrics_view_open(&view, library, the_track(library));
    CHECK(lyrics_view_follow(&view, 0, 10));
    CHECK_EQUAL(0, strcmp("", view.rows[LYRICS_VIEW_CURRENT_ROW]));
    CHECK_EQUAL(0, strcmp("first", view.rows[LYRICS_VIEW_CURRENT_ROW + 1]));
    CHECK(lyrics_view_follow(&view, 1500, 20));
    CHECK_EQUAL(0, strcmp("first", view.rows[LYRICS_VIEW_CURRENT_ROW]));
    CHECK_EQUAL(0, strcmp("second", view.rows[LYRICS_VIEW_CURRENT_ROW + 1]));
    CHECK(lyrics_view_follow(&view, 9500, 30));
    CHECK_EQUAL(0, strcmp("third", view.rows[LYRICS_VIEW_CURRENT_ROW]));
    CHECK_EQUAL(0, strcmp("", view.rows[LYRICS_VIEW_CURRENT_ROW + 1]));
    CHECK_EQUAL(0, strcmp("second", view.rows[LYRICS_VIEW_CURRENT_ROW - 1]));
    CHECK_EQUAL(0, strcmp("first", view.rows[LYRICS_VIEW_CURRENT_ROW - 2]));
    CHECK(lyrics_view_follow(&view, 0, 40));
    CHECK_EQUAL(0, strcmp("second", view.rows[LYRICS_VIEW_CURRENT_ROW + 2]));
}

static void the_change_time_only_moves_when_the_line_changes(void)
{
    const Library *library = fresh_library();
    fake_hal_add_file(LYRICS_PATH, (const uint8_t *)LYRICS, (uint32_t)strlen(LYRICS));
    lyrics_view_open(&view, library, the_track(library));
    lyrics_view_follow(&view, 1500, 100);
    lyrics_view_follow(&view, 2500, 200);
    CHECK_EQUAL(100, view.changed_ms);
    lyrics_view_follow(&view, 5500, 300);
    CHECK_EQUAL(300, view.changed_ms);
}

static void seeking_back_shows_the_earlier_line_again(void)
{
    const Library *library = fresh_library();
    fake_hal_add_file(LYRICS_PATH, (const uint8_t *)LYRICS, (uint32_t)strlen(LYRICS));
    lyrics_view_open(&view, library, the_track(library));
    lyrics_view_follow(&view, 9500, 10);
    lyrics_view_follow(&view, 5500, 20);
    CHECK_EQUAL(0, strcmp("second", view.rows[LYRICS_VIEW_CURRENT_ROW]));
    CHECK_EQUAL(0, strcmp("third", view.rows[LYRICS_VIEW_CURRENT_ROW + 1]));
}

static void a_track_without_a_lyrics_file_has_nothing_to_show(void)
{
    const Library *library = fresh_library();
    lyrics_view_open(&view, library, the_track(library));
    CHECK(!lyrics_view_ready(&view));
    CHECK(!lyrics_view_follow(&view, 1500, 10));
}

static void opening_a_new_track_forgets_the_old_lines(void)
{
    const Library *library = fresh_library();
    fake_hal_add_file(LYRICS_PATH, (const uint8_t *)LYRICS, (uint32_t)strlen(LYRICS));
    lyrics_view_open(&view, library, the_track(library));
    lyrics_view_follow(&view, 1500, 10);
    lyrics_view_open(&view, library, UINT16_MAX);
    CHECK(!lyrics_view_ready(&view));
    CHECK_EQUAL(0, strcmp("", view.rows[LYRICS_VIEW_CURRENT_ROW]));
}

static void overwritten_memory_stops_the_lyrics_instead_of_reading_garbage(void)
{
    const Library *library = fresh_library();
    fake_hal_add_file(LYRICS_PATH, (const uint8_t *)LYRICS, (uint32_t)strlen(LYRICS));
    lyrics_view_open(&view, library, the_track(library));
    view.magic = 0;
    CHECK(!lyrics_view_ready(&view));
    CHECK(!lyrics_view_follow(&view, 1500, 10));
}

int main(void)
{
    static const TestCase cases[] = {
        TEST_CASE(a_track_with_a_lyrics_file_beside_it_loads_the_lines),
        TEST_CASE(the_lines_follow_the_playback_position),
        TEST_CASE(the_change_time_only_moves_when_the_line_changes),
        TEST_CASE(seeking_back_shows_the_earlier_line_again),
        TEST_CASE(a_track_without_a_lyrics_file_has_nothing_to_show),
        TEST_CASE(opening_a_new_track_forgets_the_old_lines),
        TEST_CASE(overwritten_memory_stops_the_lyrics_instead_of_reading_garbage),
    };
    return test_run_all(cases, TEST_CASE_COUNT(cases));
}
