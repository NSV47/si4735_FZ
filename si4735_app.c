#include "si4735_app.h"

#define AM_MODE 0
#define __FM_MODE 1
#define __SSB_MODE 2
#define SYNC_MODE 3

uint16_t old_freq=0;

static void si4735_app_draw_callback(Canvas* canvas, void* ctx) {
    // UNUSED(ctx);
    furi_assert(ctx);
    si4735App* app = ctx;

    canvas_clear(canvas);

    // canvas_draw_icon(canvas, 0, 29, &I_amperka_ru_logo_128x35px);
    canvas_draw_icon(canvas, 0, 0, &I_main_interface);

    // canvas_set_font(canvas, FontPrimary);
    // canvas_draw_str(canvas, 4, 8, "RUN");

    // canvas_set_font(canvas, FontSecondary);
    // elements_multiline_text_aligned(canvas, 127, 15, AlignRight, AlignTop, "Some long long long long \n aligned multiline text");

    // uint16_t freq_khz;
    // app->freq_khz = 9920;
    char string[30];
    snprintf(string, 10, "%d", app->freq_khz * app->multiplier_freq); // app->freq_khz // app->multiplier_freq
    // FURI_LOG_I(TAG, string);
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str(canvas, 35, 49, string);
    // elements_multiline_text_aligned(canvas, 45, 38, AlignRight, AlignTop, string);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 110, 49, "kHz");
    canvas_set_font(canvas, FontSecondary);
    // snprintf(string, 30, "SNR:%2ddB SI: %2duVdB", app->snr, app->rssi);
    snprintf(string, 30, "SNR:%2ddB", app->snr);
    canvas_draw_str(canvas, 73, 31, string);
    snprintf(string, 30, "RSSI:%2duVdB", app->rssi);
    canvas_draw_str(canvas, 71, 21, string);
    // snprintf(string, 30, "status x%x %dKHz   ", app->status, app->coef * app->n);
    snprintf(string, 30, "x%x", app->status);
    canvas_draw_str(canvas, 2, 42, string); // 4, 36
    snprintf(string, 30, "%dKHz   ", app->coef * app->n);
    canvas_draw_str(canvas, 2, 50, string); // 4, 36

    canvas_draw_str(canvas, 2, 9, app->PTy_buffer);
    canvas_draw_str(canvas, 2, 18, app->PSName);
}

static void timer_callback(void* context) { // FuriMessageQueue* event_queue
    // Проверяем, что контекст не нулевой
    furi_assert(context); // furi_assert(event_queue);

    // Приводим context к типу FuriMessageQueue*
    FuriMessageQueue* event_queue = (FuriMessageQueue*)context;

    // UNUSED(event_queue);

    si4735Event event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

static void si4735_app_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);

    FuriMessageQueue* event_queue = ctx;
    
    si4735Event event = {.type = EventTypeInput, .input = *input_event};

    furi_message_queue_put(event_queue, &event, FuriWaitForever); // input_event
}

si4735App* si4735_app_alloc() {
    si4735App* app = malloc(sizeof(si4735App));

    app->view_port = view_port_alloc();

    app->event_queue = furi_message_queue_alloc(8, sizeof(si4735Event)); // InputEvent

    view_port_draw_callback_set(app->view_port, si4735_app_draw_callback, app); // NULL // app
    view_port_input_callback_set(app->view_port, si4735_app_input_callback, app->event_queue);

    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    app->timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, app->event_queue);

    // app->input_pin = &gpio_ext_pa6;
    app->output_pin = &gpio_ext_pa7;

    // furi_hal_gpio_init(app->input_pin, GpioModeInput, GpioPullUp, GpioSpeedVeryHigh);
    furi_hal_gpio_init(app->output_pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    app->PTy_buffer = (char*) malloc(30);

    return app;
}

void si4735_app_free(si4735App* app) {
    furi_assert(app);

    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);

    furi_timer_free(app->timer);

    furi_message_queue_free(app->event_queue);

    furi_record_close(RECORD_NOTIFICATION);

    furi_record_close(RECORD_GUI);

    free(app->PTy_buffer);
}

int32_t si4735_app(void *p) {
    UNUSED(p);
    si4735App* app = si4735_app_alloc();

    uint8_t status,rev,snr,rssi,resp1,resp2;
    UNUSED(resp1);
    UNUSED(resp2);
    UNUSED(rev);
    int16_t bfo=0;
    uint8_t freq_of; // int8_t freq_of
    // furi_delay_ms(10000);
    si4734_reset(app);
    for(uint32_t i=0;i<0x5ff;i++)__asm__("nop");
    // si4734_fm_mode(); // просто запускает кварц
    reciver_set_mode(app, __FM_MODE);

    si4735Event event; // InputEvent

    furi_timer_start(app->timer, 40);

    while (1) {
        show_freq(app, app->freq_khz, app->offset);
        status=get_recivier_signal_status(&snr,&rssi,&freq_of);
        show_reciver_full_status(app, app->freq_khz,bfo,snr,rssi,status);
        // furi_hal_gpio_write(app->output_pin, app->output_value);
    #if 0
        if(old_freq!=app->freq_khz){
			old_freq=app->freq_khz;
			if(reciver_mode==1) {
                status=si4734_fm_set_freq(encoder);
            }else if(reciver_mode==2) {
                status=si4734_ssb_set_freq(encoder);
            }
			else {
                status=si4734_am_set_freq(encoder);
            }
		}
    #endif
    #if 1
        // Выбираем событие из очереди в переменную event (ждем бесконечно долго, если очередь пуста)
        // и проверяем, что у нас получилось это сделать
        furi_check(furi_message_queue_get(app->event_queue, &event, FuriWaitForever) == FuriStatusOk);
    // if (furi_message_queue_get(app->event_queue, &event, 100) == FuriStatusOk) {
        if (event.type == EventTypeInput) {
            if (event.input.key == InputKeyBack){
                // si4734_powerdown();
                break;
            }else if(event.input.key == InputKeyUp){
                si4734_volume(7);//громче
                // vol++;
            }else if(event.input.key == InputKeyDown){
                si4734_volume(-7);//тише
            }else if(event.input.key == InputKeyOk){
                // show_freq(9920, 0);
            }else if(event.input.key == InputKeyRight){
                app->freq_khz++;
            }else if(event.input.key == InputKeyLeft){
                app->freq_khz--;
            }
        // Наше событие — это сработавший таймер
        } else if(event.type == EventTypeTick) {
            // Отправляем нотификацию мигания синим светодиодом
            // notification_message(app->notifications, &sequence_blink_blue_100);
            // FURI_LOG_I(TAG, "timer action");
            show_RDS_hum_2(app);
        }
    // }
    #endif
    }

    si4735_app_free(app);
    return 0;
}