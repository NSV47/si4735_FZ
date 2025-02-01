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
    canvas_draw_icon(canvas, 102, 0, &I_RDS);

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

#if 0
static void timer_callback(void* context) { // FuriMessageQueue* event_queue
    // Проверяем, что контекст не нулевой
    furi_assert(context); // furi_assert(event_queue);

    // Приводим context к типу FuriMessageQueue*
    FuriMessageQueue* event_queue = (FuriMessageQueue*)context;

    // UNUSED(event_queue);

    si4735Event event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}
#endif

// This function is called each time the timer expires (i.e. once per 1000 ms (1s) in this example)
static void event_loop_timer_callback(void* context) {
    furi_assert(context);
    si4735App* app = context;

    // Print the countdown value
    // FURI_LOG_I(TAG, "T-00:00:%02lu", app->countdown_value);

    show_RDS_hum_2(app);
#if 0
    if(app->countdown_value == 0) {
        // If the countdown reached 0, print the final line and stop the event loop
        FURI_LOG_I(TAG, "Blast off to adventure!");
        // After this call, the control will be returned back to event_loop_timers_app_run()
        furi_event_loop_stop(app->event_loop);

    } else {
        // Decrement the countdown value
        app->countdown_value -= 1;
    }
#endif
}

static void si4735_app_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);

    FuriMessageQueue* event_queue = ctx;
    
    // si4735Event event = {.type = EventTypeInput, .input = *input_event};

    furi_message_queue_put(event_queue, input_event, FuriWaitForever); // input_event // &event
}

si4735App* si4735_app_alloc() {
    si4735App* app = malloc(sizeof(si4735App));

    app->view_port = view_port_alloc();

    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent)); // InputEvent // si4735Event

    view_port_draw_callback_set(app->view_port, si4735_app_draw_callback, app); // NULL // app
    view_port_input_callback_set(app->view_port, si4735_app_input_callback, app->event_queue);

    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    // app->timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, app->event_queue);
    // Create an event loop instance.
    app->event_loop = furi_event_loop_alloc();
    // Create a software timer instance.
    // The timer is bound to the event loop instance and will execute in its context.
    // Here, the timer type is periodic, i.e. it will restart automatically after expiring.
    app->timer = furi_event_loop_timer_alloc(
        app->event_loop, event_loop_timer_callback, FuriEventLoopTimerTypePeriodic, app);

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

    // furi_timer_free(app->timer);
    // IMPORTANT: All event loop timers MUST be deleted BEFORE deleting the event loop itself.
    // Failure to do so will result in a crash.
    furi_event_loop_timer_free(app->timer);
    // With all timers deleted, it's safe to delete the event loop.
    furi_event_loop_free(app->event_loop);

    furi_message_queue_free(app->event_queue);

    furi_record_close(RECORD_NOTIFICATION);

    furi_record_close(RECORD_GUI);

    free(app->PTy_buffer);

    free(app);
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

    InputEvent event; // InputEvent // si4735Event

    // furi_timer_start(app->timer, 40); // 40
    // Timers can be started either before the event loop is run, or in any
    // callback function called by a running event loop.
    furi_event_loop_timer_start(app->timer, 40); // COUNTDOWN_INTERVAL_MS

    while (1) {
        // This call will block until furi_event_loop_stop() is called.
        furi_event_loop_run(app->event_loop);
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
        // furi_check(furi_message_queue_get(app->event_queue, &event, FuriWaitForever) == FuriStatusOk); // FuriWaitForever
        if (furi_message_queue_get(app->event_queue, &event, 100) == FuriStatusOk) {
            if (event.type == InputTypePress) { // EventTypeInput
                if (event.key == InputKeyBack){ // .input
                    // si4734_powerdown();
                    break;
                }else if(event.key == InputKeyUp){ // .input
                    si4734_volume(7);//громче
                    // vol++;
                }else if(event.key == InputKeyDown){ // .input
                    si4734_volume(-7);//тише
                }else if(event.key == InputKeyOk){ // .input
                    // show_freq(9920, 0);
                }else if(event.key == InputKeyRight){ // .input
                    app->freq_khz++;
                }else if(event.key == InputKeyLeft){ // .input
                    app->freq_khz--;
                }
            // Наше событие — это сработавший таймер
            }
            #if 0 
            else if(event.type == EventTypeTick) {
                // Отправляем нотификацию мигания синим светодиодом
                // notification_message(app->notifications, &sequence_blink_blue_100);
                // FURI_LOG_I(TAG, "timer action");
                show_RDS_hum_2(app);
            }
            #endif
        }
    #endif
    }

    si4735_app_free(app);
    return 0;
}