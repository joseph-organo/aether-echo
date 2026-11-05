#pragma once

#include <stdbool.h>
#include <stdint.h>

enum {
    PANEL_FIRST_VISIBLE_ROW = 49
};

typedef struct PanelBus {
    void (*command)(void *context, uint8_t command);
    void (*data)(void *context, uint8_t value);
    void (*pixels)(void *context, const uint16_t *pixels, uint32_t count);
    void (*delay_ms)(void *context, uint32_t milliseconds);
    void *context;
} PanelBus;

typedef enum PanelRotation {
    PANEL_LANDSCAPE,
    PANEL_PORTRAIT,
    PANEL_PORTRAIT_FLIPPED,
    PANEL_ROTATION_COUNT
} PanelRotation;

typedef struct PanelWindow {
    int x;
    int y;
    int width;
    int height;
    PanelRotation rotation;
} PanelWindow;

void panel_send_init(const PanelBus *bus);
void panel_set_rotation(const PanelBus *bus, PanelRotation rotation);
void panel_begin_write(const PanelBus *bus, PanelWindow window);
