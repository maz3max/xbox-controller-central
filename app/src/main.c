/*
 * Copyright (c) 2023 Maximilian Deubel
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include "app_version.h"

#include <zephyr/zbus/zbus.h>

#include "xbox_controller_ble/report_structs.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

static bool connected;

ZBUS_CHAN_DECLARE(controller_report);
ZBUS_CHAN_DECLARE(controller_connected);

ZBUS_SUBSCRIBER_DEFINE(controller_connected_subscriber, 1);
void subscriber_task(void)
{
        const struct zbus_channel *chan;

        while (!zbus_sub_wait(&controller_connected_subscriber, &chan, K_FOREVER)) {
                if (&controller_connected == chan) {
                        // Indirect message access
                        zbus_chan_read(&controller_connected, &connected, K_NO_WAIT);
                        LOG_INF("Connected: %d", connected);
                }
        }
}
K_THREAD_DEFINE(subscriber_task_id, 512, subscriber_task, NULL, NULL, NULL, K_LOWEST_APPLICATION_THREAD_PRIO, 0, 0);


int main(void)
{
	struct xbox_controller_report report = {0};
	struct xbox_controller_report_output rumblereq = {
		//.reportId = 0x3,
		.DcEnableActuators = 0b0001,
		.Duration = 20,
		.Magnitude = {80,80,80,80},
	};
	

	LOG_INF("Zephyr Example Application %s\n", APP_VERSION_STR);

	zbus_chan_add_obs(&controller_connected, &controller_connected_subscriber, K_FOREVER);

	k_sleep(K_MSEC(1000));

	while (true)
	{
		if (connected){
			zbus_chan_read(&controller_report, &report, K_FOREVER);
			uint8_t *r = (uint8_t *)&report;
			LOG_INF("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8], r[9], r[10], r[11], r[12], r[13], r[14], r[15]);
			for (size_t i = 0; i < 4; ++i) {
				rumblereq.DcEnableActuators = 1 << i;
				request_rumble(&rumblereq);
				k_sleep(K_MSEC(500));
			}
		}
	}

	return 0;
}
