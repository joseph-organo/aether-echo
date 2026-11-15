#include "ui/message_screen.h"

#include <stddef.h>

#include "ui/header.h"
#include "ui/label.h"
#include "ui/navigation.h"

enum {
    MESSAGE_LINES = 4,
    LINE_CAPACITY = 128,
    LINE_GAP = 2,
    ART_TOP = 26,
    ART_HEIGHT = 64,
    ART_GAP = 14,
    STORAGE_WIDTH = 88,
    STORAGE_BORDER = 4,
    BAR_COUNT = 5,
    BAR_WIDTH = 10,
    BAR_STEP = 18,
    LEVEL_WIDTH = 200,
    LEVEL_HEIGHT = 4,
    LEVEL_BOTTOM = 18
};

static const uint8_t BAR_HEIGHTS[BAR_COUNT] = {22, 42, 58, 36, 48};

static void draw_art(Canvas *canvas, const Theme *theme, Rect content, MessageArt art)
{
    Color accent = theme->colors.selection_background;
    int center = content.x + content.width / 2;
    int top = content.y + ART_TOP;
    if (art == MESSAGE_ART_STORAGE) {
        Rect body = {center - STORAGE_WIDTH / 2, top + 6, STORAGE_WIDTH, ART_HEIGHT - 12};
        canvas_fill_rect(canvas, body, accent);
        canvas_fill_rect(canvas,
                         (Rect){body.x + STORAGE_BORDER, body.y + STORAGE_BORDER,
                                body.width - 2 * STORAGE_BORDER, body.height - 2 * STORAGE_BORDER},
                         theme->colors.background);
        canvas_fill_rect(canvas, (Rect){body.x + 12, body.y + body.height - 20, 10, 10}, accent);
        canvas_fill_rect(canvas, (Rect){body.x + 34, body.y + body.height - 17, 42, 4}, accent);
        return;
    }
    int left = center - (BAR_COUNT * BAR_STEP - (BAR_STEP - BAR_WIDTH)) / 2;
    for (int bar = 0; bar < BAR_COUNT; bar++) {
        canvas_fill_rect(canvas,
                         (Rect){left + bar * BAR_STEP, top + ART_HEIGHT - BAR_HEIGHTS[bar],
                                BAR_WIDTH, BAR_HEIGHTS[bar]},
                         accent);
    }
}

static void draw_level(Canvas *canvas, const Theme *theme, Rect content, int percent)
{
    int filled = LEVEL_WIDTH * (percent < 0 ? 0 : percent > 100 ? 100 : percent) / 100;
    Rect track = {content.x + (content.width - LEVEL_WIDTH) / 2,
                  content.y + content.height - LEVEL_BOTTOM, LEVEL_WIDTH, LEVEL_HEIGHT};
    canvas_fill_rect(canvas, track, theme->colors.divider);
    canvas_fill_rect(canvas, (Rect){track.x, track.y, filled, track.height},
                     theme->colors.selection_background);
}

static ScreenAction handle_key(void *context, KeyEvent event, uint32_t now_ms)
{
    (void)context;
    (void)now_ms;
    return navigation_exit_action(event);
}

static void update(void *context, uint32_t now_ms)
{
    (void)context;
    (void)now_ms;
}

static size_t word_end(const char *text, size_t from)
{
    while (text[from] != '\0' && text[from] != ' ') {
        from++;
    }
    return from;
}

static void copy_span(char *line, const char *text, size_t count)
{
    size_t length = count < LINE_CAPACITY - 1 ? count : LINE_CAPACITY - 1;
    for (size_t index = 0; index < length; index++) {
        line[index] = text[index];
    }
    line[length] = '\0';
}

static size_t next_line(const Font *font, const char *text, int width, bool last, char *line)
{
    size_t fitted = word_end(text, 0);
    while (text[fitted] == ' ') {
        size_t end = word_end(text, fitted + 1);
        copy_span(line, text, end);
        if (!last && text_width(font, line) > width) {
            break;
        }
        fitted = end;
    }
    copy_span(line, text, fitted);
    while (text[fitted] == ' ') {
        fitted++;
    }
    return fitted;
}

static void draw(void *context, Canvas *canvas)
{
    const MessageScreen *message = context;
    const Theme *theme = message->config.theme;
    Rect content = theme_content_area(theme);
    TextStyle style = {.font = theme->fonts.body, .color = theme->colors.text_secondary};
    canvas_fill_rect(canvas, content, theme->colors.background);
    header_draw(canvas, theme, message->config.title);
    int padding = theme->metrics.padding;
    int width = content.width - 2 * padding;
    char lines[MESSAGE_LINES][LINE_CAPACITY];
    int count = 0;
    const char *rest = message->config.message;
    bool fits = text_width(style.font, rest) <= width;
    while (*rest != '\0' && count < MESSAGE_LINES) {
        rest +=
            next_line(style.font, rest, width, fits || count == MESSAGE_LINES - 1, lines[count]);
        count++;
    }
    int line_height = font_line_height(style.font) + LINE_GAP;
    int top = content.y + (content.height - count * line_height) / 2;
    bool art = message->config.art != MESSAGE_ART_NONE && !theme_is_portrait(theme);
    if (art) {
        draw_art(canvas, theme, content, message->config.art);
        top = content.y + ART_TOP + ART_HEIGHT + ART_GAP;
        if (message->config.art == MESSAGE_ART_DAC) {
            draw_level(canvas, theme, content, message->config.level_percent);
        }
    }
    for (int index = 0; index < count; index++) {
        Rect row = {content.x + padding, top + index * line_height, width, line_height};
        Rect area = label_centered_area(style.font, row, lines[index]);
        label_draw(canvas, style, (Label){.area = area, .text = lines[index]});
    }
}

Screen *message_screen_init(MessageScreen *message, MessageConfig config)
{
    *message = (MessageScreen){.config = config};
    message->screen = (Screen){
        .title = config.title,
        .context = message,
        .enter = NULL,
        .handle_key = handle_key,
        .update = update,
        .draw = draw,
    };
    return &message->screen;
}
