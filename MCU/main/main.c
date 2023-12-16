/*
 * MIT License
 *
 * Copyright (c) 2023 Hoozz <huxiangjs@foxmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "freertos/event_groups.h"
#include "driver/usb_serial_jtag.h"
#include "sdkconfig.h"
#include "esp_check.h"
#include "hal/usb_serial_jtag_ll.h"

#include "app_gitt.h"
#include "app_wifi.h"
#include "app_nvs.h"
#include "app_spiffs.h"
#include "app_time.h"
#include "app_led.h"
#include "app_relay.h"
#include "app_adc.h"

#define COMMAND_PREFIX			"GITT"
#define TAG				"app-main"

static struct app_gitt app = {
	.g = {
		/* Set default */
		.device = {
			.id = "0000000000000001",
			.name = "ESP32C3 Remote Switch",
		},
	},
	.interval = 5,
	.privkey = "",
	.repertory = "",
	.wifi_ssid = "",
	.wifi_password =  "",
};

#define APP_STATE_SERVER_STOP		0
#define APP_STATE_SERVER_START		1

static volatile uint8_t app_state = APP_STATE_SERVER_STOP;

#define APP_EVENT_SERVER_STOPED		BIT0
#define APP_EVENT_SERVER_STARTED	BIT1

static EventGroupHandle_t app_event_group;

#define APP_RESPONSE_NONE		0
#define APP_RESPONSE_REPORT_STATE	1
#define APP_RESPONSE_SWITCH_PRESS	2

static volatile int app_response = APP_RESPONSE_NONE;

static void app_gitt_recv_callback(char *data)
{
	printf("\nRemote say: %s\n", data);
	if (!memcmp("STATE", data, 5)) {
		app_response = APP_RESPONSE_REPORT_STATE;
		printf("Set response to report_state\n");
	} else if (!memcmp("PRESS", data, 5)) {
		app_response = APP_RESPONSE_SWITCH_PRESS;
		printf("Set response to switch_press\n");
	}
}

static void app_main_task(void *pvParameters)
{
	int ret;
	int count;

	/* Wait wifi available */
	printf("Wait wifi available...\n");
	while (!app_wifi_available())
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	printf("Wifi available\n");

	/* Update time from net */
	app_time_wait_sync();

	/* Auto start */
	app_state = APP_STATE_SERVER_START;

	while (1) {
		switch (app_state) {
		case APP_STATE_SERVER_START:
			app_led_green_on();

			printf("Server started, interval time: %d second\n", app.interval);
			xEventGroupSetBits(app_event_group, APP_EVENT_SERVER_STARTED);

			while (app_state == APP_STATE_SERVER_START) {
				/* Initialize */
				ret = app_gitt_init(&app, app_gitt_recv_callback);
				if (ret) {
					vTaskDelay(1000 / portTICK_PERIOD_MS);
					continue;
				}

				count = 0;
				/* Loop */
				while (app_state == APP_STATE_SERVER_START) {
					if (!count) {
						/* Try update */
						ret = gitt_update_event(&app.g);
						// printf("Update event result: %s\n", GITT_ERRNO_STR(ret));
						if (ret)
							break;

						/* Response */
						if (app_response == APP_RESPONSE_REPORT_STATE) {
							bool state = app_adc_detect();
							printf("Detect state: %s\n", state ? "ON" : "OFF");
							ret = gitt_commit_event(&app.g, state ? "ON" : "OFF");
							printf("Commit event result: %s\n", GITT_ERRNO_STR(ret));
							app_response = APP_RESPONSE_NONE;
							printf("Free heap size: %dbytes\n", esp_get_free_heap_size());
						} else if (app_response == APP_RESPONSE_SWITCH_PRESS) {
							app_relay_on(2000);
							ret = gitt_commit_event(&app.g, "DONE");
							printf("Commit event result: %s\n", GITT_ERRNO_STR(ret));
							app_response = APP_RESPONSE_NONE;
							printf("Free heap size: %dbytes\n", esp_get_free_heap_size());
						}
					}
					vTaskDelay(1000 / portTICK_PERIOD_MS);
					count++;
					if (count >= app.interval)
						count = 0;
				}
			}

			printf("\nServer stoped\n");
			xEventGroupSetBits(app_event_group, APP_EVENT_SERVER_STOPED);
			break;
		case APP_STATE_SERVER_STOP:
			app_led_red_on();
			break;
		}

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	vTaskDelete(NULL);
}

static void config_show(void)
{
	time_t now = 0;
	struct tm timeinfo = { 0 };

	time(&now);
	localtime_r(&now, &timeinfo);

	// printf("Wifi SSID     : %s\n", app.wifi_ssid);
	// printf("Wifi Password : %s\n", app.wifi_password);
	printf("Wifi state    : %s\n", app_wifi_available() ? "connect" : "disconnect");
	printf("Time          : %04d/%02d/%02d %02d:%02d:%02d\n", 1900 + timeinfo.tm_year,
	       timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
	printf("Running time  : %u ms\n", esp_log_timestamp());
	printf("Detect state  : %s\n", app_adc_detect() ? "on" : "off");
	printf("Device name   : %s\n", app.g.device.name);
	printf("Device id     : %s\n", app.g.device.id);
	printf("Loop interval : %d second\n", app.interval);
	printf("Repertory     : %s\n", app.repertory);
	printf("Server state  : %s\n", app_state ? "running" : "stoped");
	printf("Private key   : \n%s\n\n", app.privkey);
}

static void app_show(void)
{
	printf("  ________._________________________\n");
	printf(" /  _____/|   \\__    ___/\\__    ___/\n");
	printf("/   \\  ___|   | |    |     |    |   \n");
	printf("\\    \\_\\  \\   | |    |     |    |   \n");
	printf(" \\______  /___| |____|     |____|   \n");
	printf("        \\/                          \n");
	printf("GITT for ESP32-C3\n");
	printf("MIT License * Copyright (c) 2023 Hoozz <huxiangjs@foxmail.com>\n");
}

static void help_show(void)
{
	printf("Help:\n");
	printf("  wifi                  - Modify wifi information and reconnect\n");
	printf("  heap                  - Show free heap space size\n");
	printf("  privkey               - Update private key;  ESC:exit  Ctrl+S:save\n");
	printf("  repertory <URL>       - Update repository URL\n");
	printf("  show                  - Show configuration information\n");
	printf("  reset                 - Restart the system\n");
	printf("  start                 - Start server\n");
	printf("  stop                  - Stop server\n");
	printf("  help                  - Show help message\n");
}

static void app_server_start(void)
{
	EventBits_t bits;
	uint8_t old_state;

	if (app_state == APP_STATE_SERVER_START) {
		printf("Service is already running\n");
	} else {
		old_state = app_state;
		xEventGroupClearBits(app_event_group, APP_EVENT_SERVER_STARTED);
		app_state = APP_STATE_SERVER_START;
		bits = xEventGroupWaitBits(app_event_group,
					   APP_EVENT_SERVER_STARTED,
					   pdFALSE,
					   pdFALSE,
					   portMAX_DELAY);
		if (!(bits & APP_EVENT_SERVER_STARTED)) {
			printf("Service starts failing\n");
			app_state = old_state;
		}
	}
}

static void app_server_stop(void)
{
	EventBits_t bits;
	uint8_t old_state;

	if (app_state == APP_STATE_SERVER_STOP) {
		printf("Service has stopped\n");
	} else {
		old_state = app_state;
		xEventGroupClearBits(app_event_group, APP_EVENT_SERVER_STOPED);
		app_state = APP_STATE_SERVER_STOP;
		bits = xEventGroupWaitBits(app_event_group,
					   APP_EVENT_SERVER_STOPED,
					   pdFALSE,
					   pdFALSE,
					   portMAX_DELAY);
		if (!(bits & APP_EVENT_SERVER_STOPED)) {
			printf("Service stop failed\n");
			app_state = old_state;
		}
	}
}

static int inline usb_serial_jtag_write_string(const char *str)
{
	int ret = usb_serial_jtag_write_bytes(str, strlen(str), portMAX_DELAY);
	/* Add the following lines to fix the issue that usb_serial_jtag cannot be echoed */
	usb_serial_jtag_ll_txfifo_flush();
	return ret;
}

static int inline usb_serial_jtag_write_byte(const char ch)
{
	int ret = usb_serial_jtag_write_bytes(&ch, 1, portMAX_DELAY);
	/* Add the following lines to fix the issue that usb_serial_jtag cannot be echoed */
	usb_serial_jtag_ll_txfifo_flush();
	return ret;
}

static int inline usb_serial_jtag_read_string(char *buf, int size, char end)
{
	int ret;
	int index = 0;
	char ch;

	while (1) {
		ret = usb_serial_jtag_read_bytes(&ch, 1, portMAX_DELAY);

		if (ret != 1) {
			ESP_LOGE(TAG, "usb_serial_jtag_read_bytes fail");
			return -1;
		}

		if (ch == end || ch == 0x13) {
			/* End or Ctrl+S */
			buf[index] = '\0';
			break;
		} else if (ch == 0x1b) {
			/* ESC */
			return -1;
		} else if (ch == '\b') {
			/* Do nothing */
			continue;
		} if (ch == '\r') {
			usb_serial_jtag_write_string("\r\r\n");
			ch = '\n';
		} else {
			usb_serial_jtag_write_byte(ch);
		}

		buf[index] = ch;
		index++;
		if (index == size) {
			ESP_LOGE(TAG, "Content is too long");
			return -1;
		}
	}

	return index;
}

static int inline usb_serial_jtag_read_byte(char *ch)
{
	return usb_serial_jtag_read_bytes(ch, 1, portMAX_DELAY);
}

static void usb_serial_task(void *arg)
{
	char buff[128];
	uint16_t index = 0;
	int ret;
	uint8_t old_state = 0;

	/* Configure USB SERIAL JTAG */
	usb_serial_jtag_driver_config_t usb_serial_jtag_config = {
		.rx_buffer_size = 1024,
		.tx_buffer_size = 1024,
	};
	ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_jtag_config));
	ESP_LOGI(TAG, "usb_serial_jtag init done");

	app_show();
	help_show();

	while (1) {
		/* Show command prefix */
		if (!index)
			usb_serial_jtag_write_string("\r\r\n" COMMAND_PREFIX "# ");

		ret = usb_serial_jtag_read_byte(buff + index);

		if (ret) {
			if (buff[index] == 0x13) {
				usb_serial_jtag_write_string("^S\r\r\n");
			} else if (buff[index] == 0x1b) {
				usb_serial_jtag_write_string("ESC\r\r\n");
			} else if (buff[index] == '\b') {
				if (!index)
					usb_serial_jtag_write_string("\r\r\n");
			} else if (buff[index] == '\r') {
				usb_serial_jtag_write_string("\r\r\n");

				/* Processing command */
				buff[index] = '\0';
				if (index >= 4 && !memcmp("wifi", buff, 4)) {
					usb_serial_jtag_write_string("SSID: ");
					app.wifi_ssid[0] = '\0';
					usb_serial_jtag_read_string(app.wifi_ssid, sizeof(app.wifi_ssid), '\r');
					usb_serial_jtag_write_string("\r\r\n");

					usb_serial_jtag_write_string("PASSWORD: ");
					app.wifi_password[0] = '\0';
					usb_serial_jtag_read_string(app.wifi_password, sizeof(app.wifi_password), '\r');
					usb_serial_jtag_write_string("\r\r\n");

					/* Connect */
					ret = app_wifi_connect(app.wifi_ssid, app.wifi_password);
					printf("%s\n", ret ? "Connection failed" : "Connection succeeded");

					index = 0;
				} else if (index >= 4 && !memcmp("heap", buff, 4)) {
					printf("Free heap size: %dbytes\n", esp_get_free_heap_size());
					index = 0;
				} else if (index >= 7 && !memcmp("privkey", buff, 7)) {
					char data[1024];

					/* If it has already started, stop it first */
					old_state = app_state;
					if (app_state == APP_STATE_SERVER_START)
						app_server_stop();

					printf("\nInput private key:\n");
					/* Modify private key */
					ret = usb_serial_jtag_read_string(data, sizeof(data), 0);
					if (ret < 0) {
						printf("\nNot saved\n");
					} else {
						strcpy(app.privkey, data);
						app_spiffs_save("privkey", app.privkey, strlen(app.privkey));
						printf("\nSaved\n");
					}

					if (old_state == APP_STATE_SERVER_START)
						app_server_start();

					index = 0;
				} else if (index >= 9 && !memcmp("repertory", buff, 9)) {
					/* If it has already started, stop it first */
					old_state = app_state;
					if (app_state == APP_STATE_SERVER_START)
						app_server_stop();
					ret = sscanf(buff, "%*s%s", app.repertory);
					if (ret < 1) {
						printf("Invalid parameter\n");
					} else {
						app_spiffs_save("repertory", app.repertory, strlen(app.repertory));
						printf("Changed\n");
					}
					if (old_state == APP_STATE_SERVER_START)
						app_server_start();
					index = 0;
				} else if (index >= 5 && !memcmp("start", buff, 5)) {
					app_server_start();
					index = 0;
				} else if (index >= 4 && !memcmp("stop", buff, 4)) {
					app_server_stop();
					index = 0;
				} else if (index >= 4 && !memcmp("show", buff, 4)) {
					config_show();
					index = 0;
				} else if (index >= 5 && !memcmp("reset", buff, 5)) {
					usb_serial_jtag_write_string("Restarting now.\r\r\n");
					esp_restart();
					index = 0;
				} else if (index >= 4 && !memcmp("help", buff, 4)) {
					help_show();
					index = 0;
				} else if (index) {
					printf("Unknown command: %s\n\n", buff);
					help_show();
					index = 0;
				}
			} else {
				/* Write data back to the USB SERIAL JTAG */
				usb_serial_jtag_write_byte(buff[index]);
				index++;
				if (index == sizeof(buff)) {
					ESP_LOGE(TAG, "Command too long");
					index = 0;
				}
			}
		} else {
			ESP_LOGE(TAG, "usb_serial_jtag_read_bytes fail");
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		}
	}

	usb_serial_jtag_driver_uninstall();
	vTaskDelete(NULL);
}

static void chip_info_show(void)
{
	/* Print chip information */
	esp_chip_info_t chip_info;
	uint32_t flash_size;

	esp_chip_info(&chip_info);
	printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
	       CONFIG_IDF_TARGET,
	       chip_info.cores,
	       (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
	       (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
	       (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
	       (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

	unsigned major_rev = chip_info.revision / 100;
	unsigned minor_rev = chip_info.revision % 100;
	printf("silicon revision v%d.%d, ", major_rev, minor_rev);
	if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
		printf("Get flash size failed");
		return;
	}

	printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
	       (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

	printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
}

void app_main(void)
{
	chip_info_show();
	app_nvs_init();
	app_led_init();
	app_relay_init();
	app_spiffs_init();
	app_adc_init();
	app_wifi_init();
	app_time_init();

	app_spiffs_load("repertory", app.repertory, sizeof(app.repertory));
	app_spiffs_load("privkey", app.privkey, sizeof(app.privkey));

	app_event_group = xEventGroupCreate();
	ESP_ERROR_CHECK(app_event_group == NULL);

	config_show();

	xTaskCreate(app_main_task, "app_main_task", 1024 * 18, NULL, 5, NULL);
	xTaskCreate(usb_serial_task, "usb_serial_task", 1024 * 4, NULL, 10, NULL);
}
