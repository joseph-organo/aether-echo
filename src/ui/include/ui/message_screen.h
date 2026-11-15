#pragma once

#include "ui/screen.h"
#include "ui/theme.h"

typedef enum MessageArt {
    MESSAGE_ART_NONE,
    MESSAGE_ART_STORAGE,
    MESSAGE_ART_DAC
} MessageArt;

typedef struct MessageConfig {
    const char *title;
    const char *message;
    const Theme *theme;
    MessageArt art;
    int level_percent;
} MessageConfig;

typedef struct MessageScreen {
    Screen screen;
    MessageConfig config;
} MessageScreen;

Screen *message_screen_init(MessageScreen *message, MessageConfig config);
