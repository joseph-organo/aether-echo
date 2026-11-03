#include "lyrics_view.h"

#include <stddef.h>

#include "hal/storage.h"
#include "hal/system.h"

enum {
    SLASH_AND_EXTENSION = 5
};

static const char EXTENSION[] = ".lrc";

_Static_assert(sizeof(LyricsView) <= HAL_SPARE_MEMORY_BYTES,
               "lyrics view must fit the spare memory");

static bool read_file(void *context, uint32_t offset, void *buffer, uint32_t length)
{
    const HalFile *file = context;
    return hal_file_read(*file, offset, buffer, length);
}

static size_t text_length(const char *text)
{
    size_t length = 0;
    while (text[length] != '\0') {
        length++;
    }
    return length;
}

static size_t extension_start(const char *path, size_t length)
{
    size_t index = length;
    while (index > 0) {
        index--;
        if (path[index] == '.') {
            return index;
        }
        if (path[index] == '/') {
            break;
        }
    }
    return length;
}

static bool lyrics_path(const Library *library, uint16_t track_index, char *path, size_t capacity)
{
    LibraryTrack track;
    if (!library_read_track(library, track_index, &track) ||
        !library_read_string(library, track.path, path + 1, capacity - SLASH_AND_EXTENSION)) {
        return false;
    }
    path[0] = '/';
    size_t length = text_length(path);
    size_t start = extension_start(path, length);
    for (size_t index = 0; index < sizeof EXTENSION; index++) {
        path[start + index] = EXTENSION[index];
    }
    return true;
}

LyricsView *lyrics_view_memory(void)
{
    return hal_system_spare_memory();
}

bool lyrics_view_ready(const LyricsView *view)
{
    return view->magic == LYRICS_VIEW_MAGIC && view->loaded;
}

void lyrics_view_clear(LyricsView *view)
{
    view->magic = LYRICS_VIEW_MAGIC;
    lyrics_clear(&view->lyrics);
    view->loaded = false;
    view->line = LYRICS_VIEW_UNSET;
    view->changed_ms = 0;
    view->current[0] = '\0';
    view->next[0] = '\0';
}

void lyrics_view_open(LyricsView *view, const Library *library, uint16_t track_index)
{
    lyrics_view_clear(view);
    HalFile file;
    if (!lyrics_path(library, track_index, view->path, sizeof view->path) ||
        !hal_file_open(&file, view->path)) {
        return;
    }
    ByteSource source = {.read = read_file, .context = &file};
    view->loaded = lyrics_load(&view->lyrics, source, file.size);
    hal_file_close(&file);
}

bool lyrics_view_follow(LyricsView *view, uint32_t position_ms, uint32_t now_ms)
{
    if (!lyrics_view_ready(view)) {
        return false;
    }
    int32_t line = lyrics_line_at(&view->lyrics, position_ms);
    if (line == view->line) {
        return true;
    }
    view->current[0] = '\0';
    view->next[0] = '\0';
    HalFile file;
    if (hal_file_open(&file, view->path)) {
        ByteSource source = {.read = read_file, .context = &file};
        if (line >= 0) {
            lyrics_text(&view->lyrics, source, (uint16_t)line, view->current, sizeof view->current);
        }
        if (line + 1 < (int32_t)view->lyrics.count) {
            lyrics_text(&view->lyrics, source, (uint16_t)(line + 1), view->next, sizeof view->next);
        }
        hal_file_close(&file);
    }
    view->line = line;
    view->changed_ms = now_ms;
    return true;
}
