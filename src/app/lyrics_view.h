#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "library/library.h"
#include "playback/lyrics.h"
#include "player.h"

enum {
    LYRICS_VIEW_MAGIC = 0x4C595243,
    LYRICS_VIEW_TEXT_CAPACITY = 128,
    LYRICS_VIEW_ROWS = 5,
    LYRICS_VIEW_CURRENT_ROW = 2,
    LYRICS_VIEW_PATH_CAPACITY = PLAYER_PATH_CAPACITY + 8,
    LYRICS_VIEW_UNSET = -2
};

typedef struct LyricsView {
    uint32_t magic;
    Lyrics lyrics;
    bool loaded;
    int32_t line;
    uint32_t changed_ms;
    char path[LYRICS_VIEW_PATH_CAPACITY];
    char rows[LYRICS_VIEW_ROWS][LYRICS_VIEW_TEXT_CAPACITY];
} LyricsView;

LyricsView *lyrics_view_memory(void);
bool lyrics_view_ready(const LyricsView *view);
void lyrics_view_clear(LyricsView *view);
void lyrics_view_open(LyricsView *view, const Library *library, uint16_t track_index);
bool lyrics_view_follow(LyricsView *view, uint32_t position_ms, uint32_t now_ms);
