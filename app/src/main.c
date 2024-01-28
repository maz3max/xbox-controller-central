/*
 * Copyright (c) 2023 Maximilian Deubel
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include "app_version.h"

#include <zephyr/zbus/zbus.h>

#include "xbox_controller_ble/report_structs.h"
#include "xbox_controller_ble/hid_descr.h"

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_hid.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

ZBUS_CHAN_DECLARE(controller_report);
ZBUS_CHAN_DECLARE(controller_connected);

ZBUS_SUBSCRIBER_DEFINE(controller_connected_subscriber, 1);
ZBUS_SUBSCRIBER_DEFINE(controller_report_subscriber, 1);

static enum usb_dc_status_code usb_status;
static void status_cb(enum usb_dc_status_code status, const uint8_t *param)
{
	usb_status = status;
}

static void rumble_ready(const struct device *dev)
{
	outputReport03_t report_in = {0};
	uint32_t ret_bytes = 0;
	uint8_t *r = (uint8_t *)&report_in;
	struct xbox_controller_report_output *report_out = (void *)(r + 1);
	int ret = hid_int_ep_read(dev, r, sizeof(outputReport03_t), &ret_bytes);

	if (!ret)
	{
		LOG_INF("< %02x%02x%02x%02x%02x%02x%02x%02x%02x",
			r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8]);
		request_rumble(report_out);
	}
}

static void convert_in_report(struct xbox_controller_report const *in, inputReport01_t *out)
{
	out->reportId = 1;
	out->BTN_GamePadButton1 = in->a;
	out->BTN_GamePadButton2 = in->b;
	out->BTN_GamePadButton3 = in->x;
	out->BTN_GamePadButton4 = in->y;
	out->BTN_GamePadButton5 = in->lb;
	out->BTN_GamePadButton6 = in->rb;
	out->BTN_GamePadButton7 = in->select;
	out->BTN_GamePadButton8 = in->start;
	out->BTN_GamePadButton9 = in->system;
	out->BTN_GamePadButton10 = in->lstick_btn;
	out->BTN_GamePadButton11 = in->rstick_btn;
	out->GD_GamePadX = in->lstick_x >> 8;
	out->GD_GamePadY = in->lstick_y >> 8;
	out->GD_GamePadZ = in->rstick_x >> 8;
	out->GD_GamePadRx = in->rstick_y >> 8;
	out->GD_GamePadRy = in->lt >> 2;
	out->GD_GamePadRz = in->rt >> 2;
	out->GD_GamePadHatSwitch = in->dpad.raw;
}

int main(void)
{
	struct xbox_controller_report report = {0};
	inputReport01_t report_out = {0};
	int ret;

	LOG_INF("Zephyr Example Application %s\n", APP_VERSION_STR);

	zbus_chan_add_obs(&controller_connected, &controller_connected_subscriber, K_FOREVER);
	zbus_chan_add_obs(&controller_report, &controller_report_subscriber, K_FOREVER);

	const struct device *hid_dev;

	hid_dev = device_get_binding("HID_0");
	if (hid_dev == NULL)
	{
		LOG_ERR("Cannot get USB HID Device");
		return -1;
	}

	struct hid_ops op = {
	    .int_out_ready = rumble_ready,
	};

	usb_hid_register_device(hid_dev,
				hid_report_desc, sizeof(hid_report_desc),
				&op);

	usb_hid_init(hid_dev);

	ret = usb_enable(status_cb);
	if (ret != 0)
	{
		LOG_ERR("Failed to enable USB");
		return -1;
	}

	while (true)
	{
		if (!zbus_sub_wait(&controller_report_subscriber, NULL, K_MSEC(1)))
		{
			zbus_chan_read(&controller_report, &report, K_FOREVER);
			convert_in_report(&report, &report_out);
		}

		uint8_t *r = (uint8_t *)&report_out;
		LOG_DBG("> %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8], r[9]);
		ret = hid_int_ep_write(hid_dev, r, sizeof(report_out), NULL);
		if (ret)
		{
			LOG_DBG("HID write error, %d", ret);
		}
	}

	return 0;
}
