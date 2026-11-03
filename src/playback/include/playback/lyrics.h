#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "library/byte_source.h"

enum {
    LYRICS_CAPACITY = 256,
    LYRICS_NONE = -1
};

typedef struct LyricsLine {
    uint32_t time_ms;
    uint32_t offset;
    uint16_t length;
} LyricsLine;

typedef struct Lyrics {
    LyricsLine lines[LYRICS_CAPACITY];
    uint16_t count;
} Lyrics;

void lyrics_clear(Lyrics *lyrics);
bool lyrics_load(Lyrics *lyrics, ByteSource source, uint32_t size);
int32_t lyrics_line_at(const Lyrics *lyrics, uint32_t position_ms);
uint32_t lyrics_text(const Lyrics *lyrics, ByteSource source, uint16_t index, char *text,
                     uint32_t capacity);
