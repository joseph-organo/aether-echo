#include "check.h"
#include "ui/message_screen.h"
#include "ui_test_canvas.h"
#include "ui_test_theme.h"

static MessageScreen message;

static Screen *open_message(void)
{
    MessageConfig config = {.title = "Note", .message = "AB", .theme = &TEST_THEME};
    return message_screen_init(&message, config);
}

static ScreenAction press(Screen *screen, Key key, KeyGesture gesture)
{
    return screen->handle_key(screen->context, (KeyEvent){key, gesture}, 100);
}

static void the_message_title_is_the_screen_title(void)
{
    Screen *screen = open_message();
    CHECK(screen->title == message.config.title);
}

static void back_pops_and_a_long_back_goes_home(void)
{
    Screen *screen = open_message();
    CHECK_EQUAL(SCREEN_ACTION_POP, press(screen, KEY_BACK, KEY_GESTURE_PRESS).type);
    CHECK_EQUAL(SCREEN_ACTION_POP_TO_ROOT, press(screen, KEY_BACK, KEY_GESTURE_LONG_PRESS).type);
}

static void other_keys_do_nothing(void)
{
    Screen *screen = open_message();
    CHECK_EQUAL(SCREEN_ACTION_NONE, press(screen, KEY_PLAY, KEY_GESTURE_PRESS).type);
    CHECK_EQUAL(SCREEN_ACTION_NONE, press(screen, KEY_VOLUME_UP, KEY_GESTURE_PRESS).type);
}

static void the_message_is_drawn_below_the_header(void)
{
    Screen *screen = open_message();
    Canvas canvas = ui_test_canvas();
    screen->draw(screen->context, &canvas);
    Rect content = theme_content_area(&TEST_THEME);
    CHECK(ui_test_area_has_color(&canvas, content, TEST_THEME.colors.text_secondary));
    CHECK(ui_test_area_has_color(&canvas, theme_header_area(&TEST_THEME),
                                 TEST_THEME.colors.header_background));
}

static void a_message_too_wide_for_the_screen_wraps_onto_more_lines(void)
{
    MessageConfig config = {
        .title = "Note", .message = "AB AB AB AB AB AB AB AB AB AB AB AB", .theme = &TEST_THEME};
    Screen *screen = message_screen_init(&message, config);
    Canvas canvas = ui_test_canvas();
    screen->draw(screen->context, &canvas);
    Rect content = theme_content_area(&TEST_THEME);
    int first = -1;
    int last = -1;
    for (int y = content.y; y < content.y + content.height; y++) {
        Rect row = {content.x, y, content.width, 1};
        if (ui_test_area_has_color(&canvas, row, TEST_THEME.colors.text_secondary)) {
            first = first < 0 ? y : first;
            last = y;
        }
    }
    CHECK(first >= 0);
    CHECK(last - first >= font_line_height(TEST_THEME.fonts.body));
}

enum {
    ART_SCREEN_WIDTH = 240,
    ART_SCREEN_HEIGHT = 140
};

static Color art_pixels[ART_SCREEN_WIDTH * ART_SCREEN_HEIGHT];

static Theme art_theme(void)
{
    Theme theme = TEST_THEME;
    theme.metrics.screen_width = ART_SCREEN_WIDTH;
    theme.metrics.screen_height = ART_SCREEN_HEIGHT;
    return theme;
}

static Canvas draw_with_art(const Theme *theme, MessageArt art, int level)
{
    Canvas canvas = canvas_make(art_pixels, (Rect){0, 0, ART_SCREEN_WIDTH, ART_SCREEN_HEIGHT});
    canvas_clear(&canvas, 0);
    MessageConfig config = {.title = "USB", .message = "AB", .theme = theme, .art = art,
                            .level_percent = level};
    Screen *screen = message_screen_init(&message, config);
    screen->draw(screen->context, &canvas);
    return canvas;
}

static void a_plain_message_draws_no_accent_color(void)
{
    Theme theme = art_theme();
    Canvas canvas = draw_with_art(&theme, MESSAGE_ART_NONE, 0);
    CHECK(!ui_test_area_has_color(&canvas, theme_content_area(&theme),
                                  theme.colors.selection_background));
}

static void the_storage_and_dac_art_draw_in_the_accent_color(void)
{
    Theme theme = art_theme();
    Canvas storage = draw_with_art(&theme, MESSAGE_ART_STORAGE, 0);
    CHECK(ui_test_area_has_color(&storage, theme_content_area(&theme),
                                 theme.colors.selection_background));
    Canvas dac = draw_with_art(&theme, MESSAGE_ART_DAC, 0);
    CHECK(ui_test_area_has_color(&dac, theme_content_area(&theme),
                                 theme.colors.selection_background));
}

static void the_dac_volume_bar_fills_with_the_level(void)
{
    Theme theme = art_theme();
    Rect content = theme_content_area(&theme);
    int row = content.y + content.height - 18;
    int left = content.x + (content.width - 200) / 2;
    Canvas empty = draw_with_art(&theme, MESSAGE_ART_DAC, 0);
    CHECK_EQUAL(theme.colors.divider, canvas_pixel_at(&empty, left + 100, row));
    Canvas half = draw_with_art(&theme, MESSAGE_ART_DAC, 50);
    CHECK_EQUAL(theme.colors.selection_background, canvas_pixel_at(&half, left + 50, row));
    CHECK_EQUAL(theme.colors.divider, canvas_pixel_at(&half, left + 150, row));
    Canvas storage = draw_with_art(&theme, MESSAGE_ART_STORAGE, 100);
    CHECK(canvas_pixel_at(&storage, left + 100, row) != theme.colors.selection_background);
}

int main(void)
{
    static const TestCase cases[] = {
        TEST_CASE(the_message_title_is_the_screen_title),
        TEST_CASE(back_pops_and_a_long_back_goes_home),
        TEST_CASE(other_keys_do_nothing),
        TEST_CASE(the_message_is_drawn_below_the_header),
        TEST_CASE(a_message_too_wide_for_the_screen_wraps_onto_more_lines),
        TEST_CASE(a_plain_message_draws_no_accent_color),
        TEST_CASE(the_storage_and_dac_art_draw_in_the_accent_color),
        TEST_CASE(the_dac_volume_bar_fills_with_the_level),
    };
    return test_run_all(cases, TEST_CASE_COUNT(cases));
}
