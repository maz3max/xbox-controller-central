/*
 * Copyright (c) 2023 Maximilian Deubel
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <errno.h>

#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/settings/settings.h>
#include <zephyr/logging/log.h>
#include <zephyr/init.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include <zephyr/zbus/zbus.h>

#include "xbox_controller_ble/report_structs.h"

#include "indicator.h"
#include <dk_buttons_and_leds.h>

LOG_MODULE_REGISTER(xbox_ble, CONFIG_XBOX_CONTROLLER_BLE_LOG_LEVEL);

static void start_scan(void);
static void connect_to_device(const bt_addr_le_t *addr);

static struct bt_conn *default_conn;
static bool pairing_active;
static struct bt_gatt_discover_params discover_params;
static struct bt_gatt_subscribe_params subscribe_params;
static struct bt_uuid_16 uuid = BT_UUID_INIT_16(0);

uint16_t hids_info_attr_handle;
uint16_t hids_ctrl_attr_handle;
uint16_t hids_report_map_attr_handle;
uint16_t hids_report_attr_handle;
uint16_t hids_report_write_handle;

static uint8_t received_report[16];
static bool controller_connected_value;

#define NAME_LEN 30
#define STICK_MIDDLE 32767

ZBUS_CHAN_DEFINE(controller_report,
                 struct xbox_controller_report,
                 NULL, NULL, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.lstick_x = STICK_MIDDLE, .lstick_y = STICK_MIDDLE, .rstick_x = STICK_MIDDLE, .rstick_y = STICK_MIDDLE));

ZBUS_CHAN_DEFINE(controller_connected, bool, NULL, NULL, ZBUS_OBSERVERS_EMPTY, false);

bool bt_addr_le_is_bonded(uint8_t id, const bt_addr_le_t *addr);

static uint8_t notify_func(struct bt_conn *conn,
                           struct bt_gatt_subscribe_params *params,
                           const void *data, uint16_t length)
{
        if (!data)
        {
                LOG_INF("[UNSUBSCRIBED]");
                params->value_handle = 0U;
                return BT_GATT_ITER_STOP;
        }

        if (length != 16)
        {
                LOG_ERR("Received report of unsupported length: %d", length);
                return BT_GATT_ITER_CONTINUE;
        }

        memcpy(received_report, data, MIN(ARRAY_SIZE(received_report), length));

        const struct xbox_controller_report *report = data;

        zbus_chan_pub(&controller_report, report, K_NO_WAIT);

        return BT_GATT_ITER_CONTINUE;
}

static uint8_t discover_func(struct bt_conn *conn,
                             const struct bt_gatt_attr *attr,
                             struct bt_gatt_discover_params *params)
{
        int err;

        if (!attr)
        {
                LOG_INF("Discover complete");
                (void)memset(params, 0, sizeof(*params));
                return BT_GATT_ITER_STOP;
        }

        LOG_INF("[ATTRIBUTE] handle %u", attr->handle);

        if (!bt_uuid_cmp(discover_params.uuid, BT_UUID_HIDS))
        {
                LOG_INF("Search HIDS_INFO");
                memcpy(&uuid, BT_UUID_HIDS_INFO, sizeof(uuid));
                discover_params.uuid = &uuid.uuid;
                discover_params.start_handle = attr->handle + 1;
                discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

                err = bt_gatt_discover(conn, &discover_params);
                if (err)
                {
                        LOG_ERR("Discover failed (err %d)", err);
                        return BT_GATT_ITER_STOP;
                }
        }
        else if (!bt_uuid_cmp(discover_params.uuid, BT_UUID_HIDS_INFO))
        {
                LOG_INF("Search HIDS_CTRL");
                memcpy(&uuid, BT_UUID_HIDS_CTRL_POINT, sizeof(uuid));
                discover_params.uuid = &uuid.uuid;
                discover_params.start_handle = attr->handle + 1;
                discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

                hids_info_attr_handle = bt_gatt_attr_value_handle(attr);

                err = bt_gatt_discover(conn, &discover_params);
                if (err)
                {
                        LOG_ERR("Discover failed (err %d)", err);
                        return BT_GATT_ITER_STOP;
                }
        }
        else if (!bt_uuid_cmp(discover_params.uuid, BT_UUID_HIDS_CTRL_POINT))
        {
                LOG_INF("Search HIDS_REPORT_MAP");
                memcpy(&uuid, BT_UUID_HIDS_REPORT_MAP, sizeof(uuid));
                discover_params.uuid = &uuid.uuid;
                discover_params.start_handle = attr->handle + 1;
                discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

                hids_ctrl_attr_handle = bt_gatt_attr_value_handle(attr);

                err = bt_gatt_discover(conn, &discover_params);
                if (err)
                {
                        LOG_ERR("Discover failed (err %d)", err);
                        return BT_GATT_ITER_STOP;
                }
        }
        else if (!bt_uuid_cmp(discover_params.uuid, BT_UUID_HIDS_REPORT_MAP))
        {
                LOG_INF("Search HIDS_REPORT");
                memcpy(&uuid, BT_UUID_HIDS_REPORT, sizeof(uuid));
                discover_params.uuid = &uuid.uuid;
                discover_params.start_handle = attr->handle + 1;
                discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

                hids_report_map_attr_handle = bt_gatt_attr_value_handle(attr);

                err = bt_gatt_discover(conn, &discover_params);
                if (err)
                {
                        LOG_ERR("Discover failed (err %d)", err);
                        return BT_GATT_ITER_STOP;
                }
        }
        else if (!bt_uuid_cmp(discover_params.uuid, BT_UUID_HIDS_REPORT))
        {
                struct bt_gatt_chrc *chrc = attr->user_data;
                if (chrc->properties & BT_GATT_CHRC_WRITE_WITHOUT_RESP)
                {
                        hids_report_write_handle = bt_gatt_attr_value_handle(attr);
                        LOG_INF("hids_report_write_handle [%d]", hids_report_write_handle);
                }
                if (chrc->properties & BT_GATT_CHRC_NOTIFY)
                {
                        memcpy(&uuid, BT_UUID_GATT_CCC, sizeof(uuid));
                        discover_params.uuid = &uuid.uuid;
                        discover_params.start_handle = attr->handle + 2;
                        discover_params.type = BT_GATT_DISCOVER_DESCRIPTOR;
                        subscribe_params.value_handle = bt_gatt_attr_value_handle(attr);

                        hids_report_attr_handle = bt_gatt_attr_value_handle(attr);

                        err = bt_gatt_discover(conn, &discover_params);
                        if (err)
                        {
                                LOG_ERR("Discover failed (err %d)", err);
                                return BT_GATT_ITER_STOP;
                        }
                }
        }
        else if (!bt_uuid_cmp(discover_params.uuid, BT_UUID_GATT_CCC))
        {
                subscribe_params.notify = notify_func;
                subscribe_params.value = BT_GATT_CCC_NOTIFY;
                subscribe_params.ccc_handle = attr->handle;

                err = bt_gatt_subscribe(conn, &subscribe_params);
                if (err && err != -EALREADY)
                {
                        LOG_ERR("Subscribe failed (err %d)", err);
                        return BT_GATT_ITER_STOP;
                }
                else
                {
                        LOG_INF("[SUBSCRIBED]");
                }

                memcpy(&uuid, BT_UUID_HIDS_REPORT, sizeof(uuid));
                discover_params.uuid = &uuid.uuid;
                discover_params.start_handle = attr->handle + 1;
                discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;
                err = bt_gatt_discover(conn, &discover_params);
                if (err)
                {
                        LOG_ERR("Discover failed (err %d)", err);
                }
        }

        return BT_GATT_ITER_STOP;
}

static void connect_to_device(const bt_addr_le_t *addr)
{
        if (bt_le_scan_stop())
        {
                return;
        }

        int err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
                                    BT_LE_CONN_PARAM_DEFAULT, &default_conn);
        if (err)
        {
                LOG_ERR("Create conn failed (%u)", err);
                start_scan();
        }
}

// parse BLE ADV data and copy name into user_data
static bool data_cb(struct bt_data *data, void *user_data)
{
        char *name = user_data;
        uint8_t len;

        switch (data->type)
        {
        case BT_DATA_NAME_SHORTENED:
        case BT_DATA_NAME_COMPLETE:
                len = MIN(data->data_len, NAME_LEN - 1);
                memcpy(name, data->data, len);
                name[len] = '\0';
                return false;
        default:
                return true;
        }
}

static void scan_recv(const struct bt_le_scan_recv_info *info,
                      struct net_buf_simple *buf)
{
        char le_addr[BT_ADDR_LE_STR_LEN];
        char name[NAME_LEN];

        bt_addr_le_to_str(info->addr, le_addr, sizeof(le_addr));

        if (bt_addr_le_is_bonded(BT_ID_DEFAULT, info->addr))
        {
                LOG_INF("found bonded device %s, connecting", le_addr);
                connect_to_device(info->addr);
                return;
        }

        if (!pairing_active)
        {
                return;
        }

        /* only parse devices in close proximity */
        if (info->rssi < -70)
        {
                return;
        }

        (void)memset(name, 0, sizeof(name));

        bt_data_parse(buf, data_cb, name);

        if (strcmp("Xbox Wireless Controller", name) == 0)
        {
                connect_to_device(info->addr);
        }
}

static struct bt_le_scan_cb scan_callbacks = {
    .recv = scan_recv,
};

static void start_scan(void)
{
        int err;

        err = bt_le_scan_start(BT_LE_SCAN_ACTIVE, NULL);
        if (err)
        {
                LOG_ERR("Scanning failed to start (err %d)", err);
                return;
        }

        if (pairing_active)
        {
                set_indicator_blink_rapid();
        }
        else
        {
                set_indicator_blink_slow();
        }

        LOG_INF("Scanning successfully started");
}

static void connected(struct bt_conn *conn, uint8_t err)
{

        char addr[BT_ADDR_LE_STR_LEN];

        bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

        if (err)
        {
                LOG_ERR("Failed to connect to %s (%u)", addr, err);

                bt_conn_unref(default_conn);
                default_conn = NULL;

                start_scan();
                return;
        }

        LOG_INF("Connected: %s", addr);

        // pair and bond
        err = bt_conn_set_security(conn, BT_SECURITY_L2);
        if (err && err != -EBUSY)
        {
                bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
        }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
        char addr[BT_ADDR_LE_STR_LEN];

        if (conn != default_conn)
        {
                return;
        }

        bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

        LOG_INF("Disconnected: %s (reason 0x%02x)", addr, reason);

        bt_conn_unref(default_conn);
        default_conn = NULL;

        controller_connected_value = false;
        zbus_chan_pub(&controller_connected, &controller_connected_value, K_NO_WAIT);
        start_scan();
}

static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
        int ret;

        if (err)
        {
                LOG_ERR("Security failed: level %d err %d", level, err);
                ret = bt_conn_disconnect(conn, err);
                if (ret)
                {
                        LOG_ERR("Failed to disconnect %d", ret);
                }
        }
        else
        {
                LOG_DBG("Security changed: level %d", level);
                if (level >= BT_SECURITY_L2)
                {
                        // discover HID service attributes and subscribe to HID reports
                        LOG_INF("Search HIDS");
                        memcpy(&uuid, BT_UUID_HIDS, sizeof(uuid));
                        discover_params.uuid = &uuid.uuid;
                        discover_params.func = discover_func;
                        discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
                        discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
                        discover_params.type = BT_GATT_DISCOVER_PRIMARY;
                        err = bt_gatt_discover(default_conn, &discover_params);
                        if (err)
                        {
                                LOG_ERR("Discover failed(err %d)", err);
                                bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
                                return;
                        }
                        controller_connected_value = true;
                        zbus_chan_pub(&controller_connected, &controller_connected_value, K_NO_WAIT);
                        set_indicator_on();
                }
        }
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
    .security_changed = security_changed,
};

static void pairing_cancel(struct bt_conn *conn)
{
        pairing_active = true;
}

static void pairing_confirm(struct bt_conn *conn)
{
        if (pairing_active)
        {
                bt_conn_auth_pairing_confirm(conn);
        }
        else
        {
                bt_conn_auth_cancel(conn);
        }
}

static struct bt_conn_auth_cb conn_auth_callbacks = {
    .cancel = pairing_cancel,
    .pairing_confirm = pairing_confirm,
};

static void pairing_complete(struct bt_conn *conn, bool bonded)
{
        LOG_INF("Pairing complete");
        pairing_active = false;
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
        LOG_ERR("Pairing failed (%d), trigger disconnect", reason);
        bt_conn_disconnect(default_conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}

struct bt_conn_auth_info_cb conn_auth_info_callbacks = {
    .pairing_complete = pairing_complete,
    .pairing_failed = pairing_failed,
};

static void bond_check(const struct bt_bond_info *info, void *user_data)
{
        char addr_buf[BT_ADDR_LE_STR_LEN];

        bt_addr_le_to_str(&info->addr, addr_buf, BT_ADDR_LE_STR_LEN);

        LOG_DBG("Stored bonding found: %s", addr_buf);
        pairing_active = false;
}

static void button_handler(uint32_t button_state, uint32_t has_changed)
{
        uint32_t button = button_state & has_changed;

        if (button & DK_BTN1_MSK)
        {
                if (!pairing_active)
                {
                        pairing_active = true;
                        if (default_conn)
                        {
                                bt_conn_disconnect(default_conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
                                start_scan();
                        }
                        bt_unpair(BT_ID_DEFAULT, NULL);
                        set_indicator_blink_rapid();
                }
        }
}

static int xbox_controller_ble_init(const struct device *dev)
{
        int err;

        pairing_active = true;

        dk_buttons_init(button_handler);

        LOG_INF("Scan callbacks register");
        bt_le_scan_cb_register(&scan_callbacks);

        err = bt_conn_auth_cb_register(&conn_auth_callbacks);
        if (err)
        {
                LOG_ERR("Failed to register authorization callbacks: %d", err);
                return err;
        }

        err = bt_conn_auth_info_cb_register(&conn_auth_info_callbacks);
        if (err)
        {
                LOG_ERR("Failed to register authorization callbacks: %d", err);
                return err;
        }

        err = bt_enable(NULL);
        if (err)
        {
                LOG_ERR("Bluetooth init failed (err %d)", err);
                return err;
        }

        settings_subsys_init();
        settings_load();
        bt_foreach_bond(BT_ID_DEFAULT, bond_check, NULL);

        LOG_INF("Bluetooth initialized");

        start_scan();

        return 0;
}

int request_rumble(struct xbox_controller_report_output *report)
{
        struct bt_conn *conn = bt_conn_ref(default_conn);
        if (controller_connected_value && (hids_report_write_handle != 0))
        {
                return bt_gatt_write_without_response(conn, hids_report_write_handle, report, sizeof(struct xbox_controller_report_output), false);
        }
        bt_conn_unref(conn);
        return -EIO;
}

SYS_INIT(xbox_controller_ble_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
