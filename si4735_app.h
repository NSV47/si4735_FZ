#pragma once

#include <furi.h>
#include <furi_hal.h>

#include <furi_hal_gpio.h>

#include <gui/gui.h>

#include "si4735_api.h"

#include "si4735_icons.h"

#include <notification/notification_messages.h>

#if 0 // объявил в si4735_api.h
struct si4735App {
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;

    // DrawMode draw_mode;

    const GpioPin* output_pin;

    // bool input_value;
    bool output_value;
};

typedef struct si4735App si4735App;
#endif