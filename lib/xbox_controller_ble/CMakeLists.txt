zephyr_library()
zephyr_library_sources(ble.c)
zephyr_library_sources_ifdef(CONFIG_GPIO led.c)
