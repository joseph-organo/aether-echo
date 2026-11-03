#include "playback/lyrics.h"

enum {
    CHUNK = 256,
    STAMPS_PER_LINE = 8,
    TAG_CAPACITY = 24,
    MAX_MINUTE_DIGITS = 3,
    MAX_SECOND_DIGITS = 2,
    FRACTION_DIGITS = 3
};

typedef enum State {
    STATE_LINE_START,
    STATE_TAG,
    STATE_AFTER_TAG,
    STATE_TEXT,
    STATE_SKIP
} State;

typedef struct Parser {
    Lyrics *lyrics;
    State state;
    char tag[TAG_CAPACITY];
    uint32_t tag_length;
    uint32_t stamps[STAMPS_PER_LINE];
    uint32_t stamp_count;
    uint32_t text_start;
} Parser;

static bool is_digit(char character)
{
    return character >= '0' && character <= '9';
}

static bool is_blank(char character)
{
    return character == ' ' || character == '\t';
}

static uint32_t digit_run(const char *text, uint32_t length, uint32_t *index, uint32_t *value)
{
    uint32_t digits = 0;
    *value = 0;
    while (*index < length && is_digit(text[*index])) {
        if (digits < 9) {
            *value = *value * 10u + (uint32_t)(text[*index] - '0');
        }
        digits++;
        (*index)++;
    }
    return digits;
}

static bool parse_fraction(const char *tag, uint32_t length, uint32_t *index, uint32_t *fraction)
{
    *fraction = 0;
    if (*index >= length) {
        return true;
    }
    if (tag[*index] != '.' && tag[*index] != ':') {
        return false;
    }
    (*index)++;
    uint32_t scale = 100;
    uint32_t digits = 0;
    while (*index < length && is_digit(tag[*index])) {
        if (digits < FRACTION_DIGITS) {
            *fraction += (uint32_t)(tag[*index] - '0') * scale;
            scale /= 10u;
        }
        digits++;
        (*index)++;
    }
    return digits > 0 && *index == length;
}

static bool parse_time(const char *tag, uint32_t length, uint32_t *time_ms)
{
    uint32_t index = 0;
    uint32_t minutes;
    uint32_t digits = digit_run(tag, length, &index, &minutes);
    if (digits == 0 || digits > MAX_MINUTE_DIGITS || index >= length || tag[index] != ':') {
        return false;
    }
    index++;
    uint32_t seconds;
    digits = digit_run(tag, length, &index, &seconds);
    if (digits == 0 || digits > MAX_SECOND_DIGITS) {
        return false;
    }
    uint32_t fraction;
    if (!parse_fraction(tag, length, &index, &fraction)) {
        return false;
    }
    *time_ms = (minutes * 60u + seconds) * 1000u + fraction;
    return true;
}

static void finish_tag(Parser *parser, uint32_t next_offset)
{
    uint32_t time_ms;
    if (parse_time(parser->tag, parser->tag_length, &time_ms)) {
        if (parser->stamp_count < STAMPS_PER_LINE) {
            parser->stamps[parser->stamp_count++] = time_ms;
        }
        parser->text_start = next_offset;
        parser->state = STATE_AFTER_TAG;
        return;
    }
    if (parser->stamp_count > 0) {
        parser->state = STATE_TEXT;
        return;
    }
    parser->state = STATE_SKIP;
}

static void end_line(Parser *parser, uint32_t end)
{
    Lyrics *lyrics = parser->lyrics;
    if (parser->state != STATE_SKIP && parser->state != STATE_LINE_START) {
        uint32_t length = end > parser->text_start ? end - parser->text_start : 0;
        for (uint32_t stamp = 0; stamp < parser->stamp_count && lyrics->count < LYRICS_CAPACITY;
             stamp++) {
            lyrics->lines[lyrics->count++] = (LyricsLine){
                .time_ms = parser->stamps[stamp],
                .offset = parser->text_start,
                .length = (uint16_t)(length > UINT16_MAX ? UINT16_MAX : length),
            };
        }
    }
    parser->state = STATE_LINE_START;
    parser->stamp_count = 0;
    parser->tag_length = 0;
}

static void start_tag(Parser *parser)
{
    parser->state = STATE_TAG;
    parser->tag_length = 0;
}

static void feed(Parser *parser, uint8_t byte, uint32_t offset)
{
    if (byte == '\n' || byte == '\r') {
        end_line(parser, offset);
        return;
    }
    switch (parser->state) {
    case STATE_LINE_START:
        if (byte == '[') {
            start_tag(parser);
        } else if (!is_blank((char)byte) && !(offset < 3 && byte >= 0x80)) {
            parser->state = STATE_SKIP;
        }
        break;
    case STATE_TAG:
        if (byte == ']') {
            finish_tag(parser, offset + 1u);
        } else if (parser->tag_length + 1u < TAG_CAPACITY) {
            parser->tag[parser->tag_length++] = (char)byte;
        } else {
            parser->state = parser->stamp_count > 0 ? STATE_TEXT : STATE_SKIP;
        }
        break;
    case STATE_AFTER_TAG:
        if (byte == '[') {
            start_tag(parser);
        } else {
            parser->state = STATE_TEXT;
        }
        break;
    case STATE_TEXT:
    case STATE_SKIP:
        break;
    }
}

static void settle(Lyrics *lyrics)
{
    for (uint16_t index = 1; index < lyrics->count; index++) {
        LyricsLine line = lyrics->lines[index];
        uint16_t slot = index;
        while (slot > 0 && lyrics->lines[slot - 1].time_ms > line.time_ms) {
            lyrics->lines[slot] = lyrics->lines[slot - 1];
            slot--;
        }
        lyrics->lines[slot] = line;
    }
}

void lyrics_clear(Lyrics *lyrics)
{
    lyrics->count = 0;
}

bool lyrics_load(Lyrics *lyrics, ByteSource source, uint32_t size)
{
    lyrics_clear(lyrics);
    Parser parser = {.lyrics = lyrics, .state = STATE_LINE_START};
    uint8_t chunk[CHUNK];
    uint32_t offset = 0;
    while (offset < size && lyrics->count < LYRICS_CAPACITY) {
        uint32_t length = size - offset < CHUNK ? size - offset : CHUNK;
        if (!source.read(source.context, offset, chunk, length)) {
            lyrics_clear(lyrics);
            return false;
        }
        for (uint32_t index = 0; index < length; index++) {
            feed(&parser, chunk[index], offset + index);
        }
        offset += length;
    }
    end_line(&parser, offset);
    settle(lyrics);
    return lyrics->count > 0;
}

int32_t lyrics_line_at(const Lyrics *lyrics, uint32_t position_ms)
{
    int32_t low = 0;
    int32_t high = (int32_t)lyrics->count - 1;
    int32_t found = LYRICS_NONE;
    while (low <= high) {
        int32_t middle = low + (high - low) / 2;
        if (lyrics->lines[middle].time_ms <= position_ms) {
            found = middle;
            low = middle + 1;
        } else {
            high = middle - 1;
        }
    }
    return found;
}

static bool word_tag_end(const char *text, uint32_t start, uint32_t length, uint32_t *end)
{
    uint32_t index = start + 1u;
    while (index < length && (is_digit(text[index]) || text[index] == ':' || text[index] == '.')) {
        index++;
    }
    if (index > start + 1u && index < length && text[index] == '>') {
        *end = index + 1u;
        return true;
    }
    return false;
}

static uint32_t tidy(char *text, uint32_t length)
{
    uint32_t used = 0;
    uint32_t index = 0;
    while (index < length && is_blank(text[index])) {
        index++;
    }
    while (index < length) {
        uint32_t end;
        if (text[index] == '<' && word_tag_end(text, index, length, &end)) {
            index = end;
        } else {
            text[used++] = text[index++];
        }
    }
    while (used > 0 && is_blank(text[used - 1u])) {
        used--;
    }
    return used;
}

static uint32_t whole_characters(const char *text, uint32_t length)
{
    uint32_t start = length;
    while (start > 0 && ((uint8_t)text[start - 1u] & 0xC0u) == 0x80u) {
        start--;
    }
    if (start == 0) {
        return length;
    }
    uint8_t lead = (uint8_t)text[start - 1u];
    uint32_t needed = lead >= 0xF0 ? 4u : lead >= 0xE0 ? 3u : lead >= 0xC0 ? 2u : 1u;
    return length - (start - 1u) >= needed ? length : start - 1u;
}

uint32_t lyrics_text(const Lyrics *lyrics, ByteSource source, uint16_t index, char *text,
                     uint32_t capacity)
{
    if (capacity == 0) {
        return 0;
    }
    text[0] = '\0';
    if (index >= lyrics->count) {
        return 0;
    }
    const LyricsLine *line = &lyrics->lines[index];
    uint32_t wanted = line->length < capacity - 1u ? line->length : capacity - 1u;
    if (wanted == 0 || !source.read(source.context, line->offset, text, wanted)) {
        return 0;
    }
    uint32_t used = tidy(text, wanted);
    if (wanted < line->length) {
        used = whole_characters(text, used);
    }
    text[used] = '\0';
    return used;
}
