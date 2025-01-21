#include "si4735_app.h"

#include <furi.h>
#include <gui/gui.h>

#include <gui/elements.h>

#include <input/input.h>

static void si4735_app_draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);

    canvas_clear(canvas);

    // canvas_draw_icon(canvas, 0, 29, &I_amperka_ru_logo_128x35px);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 4, 8, "RUN");

    // canvas_set_font(canvas, FontSecondary);
    // elements_multiline_text_aligned(canvas, 127, 15, AlignRight, AlignTop, "Some long long long long \n aligned multiline text");
}

static void si4735_app_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);

    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

si4735App* si4735_app_alloc() {
    si4735App* app = malloc(sizeof(si4735App));

    app->view_port = view_port_alloc();

    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    view_port_draw_callback_set(app->view_port, si4735_app_draw_callback, NULL);
    view_port_input_callback_set(app->view_port, si4735_app_input_callback, app->event_queue);

    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    // app->input_pin = &gpio_ext_pa6;
    app->output_pin = &gpio_ext_pa7;

    // furi_hal_gpio_init(app->input_pin, GpioModeInput, GpioPullUp, GpioSpeedVeryHigh);
    furi_hal_gpio_init(app->output_pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    return app;
}

void si4735_app_free(si4735App* app) {
    furi_assert(app);

    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);

    furi_message_queue_free(app->event_queue);

    furi_record_close(RECORD_GUI);
}

int32_t si4735_app(void *p) {
    UNUSED(p);
    si4735App* app = si4735_app_alloc();

    // furi_delay_ms(10000);
    si4734_reset(app);
    for(uint32_t i=0;i<0x5ff;i++)__asm__("nop");
    si4734_fm_mode();

    InputEvent event;

    while (1) {
        // furi_hal_gpio_write(app->output_pin, app->output_value);

        if (furi_message_queue_get(app->event_queue, &event, 100) == FuriStatusOk) {
            if (event.type == InputTypePress) {
                if (event.key == InputKeyBack)
                    break;
            }
        }
    }

    si4735_app_free(app);
    return 0;
}