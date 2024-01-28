/*
 * Copyright (c) 2023 Maximilian Deubel
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/settings/settings.h>
#include <zephyr/logging/log.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include "indicator.h"

#define FREQ_FAST K_MSEC(125)
#define FREQ_SLOW K_MSEC(1000)

static k_timeout_t blink_freq;

static void led_work_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(led_work, led_work_handler);
static const struct gpio_dt_spec indicator_led = GPIO_DT_SPEC_GET(DT_ALIAS(led_indicator), gpios);

static void led_work_handler(struct k_work *work)
{
        gpio_pin_toggle_dt(&indicator_led);
        k_work_reschedule(&led_work, blink_freq);
}

void set_indicator_on()
{
        k_work_cancel_delayable(&led_work);
        gpio_pin_set_dt(&indicator_led, 1);
}
void set_indicator_off()
{
        k_work_cancel_delayable(&led_work);
        gpio_pin_set_dt(&indicator_led, 0);
}
void set_indicator_blink_rapid()
{
        blink_freq = FREQ_FAST;
        k_work_reschedule(&led_work, blink_freq);
}
void set_indicator_blink_slow()
{
        blink_freq = FREQ_SLOW;
        k_work_reschedule(&led_work, blink_freq);
}

static int indicator_init(const struct device *dev)
{
        gpio_pin_configure_dt(&indicator_led, GPIO_OUTPUT_HIGH);
        return 0;
}
SYS_INIT(indicator_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
