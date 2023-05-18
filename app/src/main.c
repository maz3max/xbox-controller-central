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

	LOG_INF("Zephyr Example Application %s\n", APP_VERSION_STR);

	zbus_chan_add_obs(&controller_connected, &controller_connected_subscriber, K_FOREVER);

	while (true)
	{
		if (connected){
			zbus_chan_read(&controller_report, &report, K_FOREVER);
			char buf[200];
			snprintk(buf, sizeof(buf), "lstick_x: %hu lstick_y: %hu rstick_x: %hu rstick_y: %hu lt: %hu rt: %hu dpad.raw: %hu a: %u b: %u x: %u y: %u lb: %u rb: %u select: %u start: %u system: %u lstick_btn: %u rstick_btn: %u",
				report.lstick_x, report.lstick_y, report.rstick_x, report.rstick_y, report.lt, report.rt, report.dpad.raw,
				report.a, report.b, report.x, report.y, report.lb, report.rb,
				report.select, report.start, report.system, report.lstick_btn, report.rstick_btn);
			LOG_INF("%s", buf);
		}
		k_sleep(K_MSEC(500));
	}

	return 0;
}
