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
 *
 * refs: esp-idf/examples/wifi/getting_started/station/main/station_example_main.c
 *
 */

#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"

static const char *TAG = "app-wifi";

static EventGroupHandle_t wifi_event_group;

#define WIFI_CONNECTED_BIT	BIT0
#define WIFI_FAIL_BIT		BIT1

static volatile int retry_count = -1;

static volatile bool app_wifi_state = false;

bool app_wifi_available(void)
{
	return app_wifi_state;
}

static void event_handler(void* arg, esp_event_base_t event_base,
			  int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		if (retry_count < 5) {
			esp_wifi_connect();
			if (retry_count >= 0) {
				retry_count++;
				ESP_LOGI(TAG, "Retry to connect to the AP");
			}
		} else {
			xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
			retry_count = -1;
			ESP_LOGI(TAG,"Connect to the AP fail");
		}
		app_wifi_state = false;
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
		ESP_LOGI(TAG, "Got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		retry_count = -1;
		xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
		app_wifi_state = true;
	}
}

void app_wifi_init(void)
{
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

	wifi_event_group = xEventGroupCreate();
	ESP_ERROR_CHECK(wifi_event_group == NULL);
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
							    ESP_EVENT_ANY_ID,
							    &event_handler,
							    NULL,
							    &instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
							    IP_EVENT_STA_GOT_IP,
							    &event_handler,
							    NULL,
							    &instance_got_ip));

	/* Automatically connect to the last wifi */
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "Wifi init finished");

	// ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
	// ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
	// vEventGroupDelete(wifi_event_group);
}

int app_wifi_connect(const char *ssid, const char *password)
{
	int retval = -1;

	wifi_config_t wifi_config = {
		.sta = {
			.sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
		},
	};

	esp_wifi_stop();

	retry_count = 0;

	xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
	strcpy((char *)wifi_config.sta.ssid, ssid);
	strcpy((char *)wifi_config.sta.password, password);
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());
	ESP_LOGI(TAG, "WiFi Connecting...");

	EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
					       WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
					       pdFALSE,
					       pdFALSE,
					       portMAX_DELAY);

	if (bits & WIFI_CONNECTED_BIT) {
		ESP_LOGI(TAG, "Connected to ap SSID:%s password:%s",
			 ssid, password);
		retval = 0;
	} else if (bits & WIFI_FAIL_BIT) {
		ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
			 ssid, password);
	} else {
		ESP_LOGE(TAG, "Unexpected event");
	}

	return retval;
}
