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
 * refs: esp-idf/examples/protocols/sntp/main/sntp_example_main.c
 *
 */

#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "esp_sntp.h"

static const char *TAG = "app-time";

static char sync_history[24] = {0};

void app_time_init(void)
{
	ESP_LOGI(TAG, "Initializing SNTP");

	/* Set time-zone */
	setenv("TZ", "CST-8", 1);
	tzset();

	esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
	esp_sntp_setservername(0, "pool.ntp.org");
	sntp_set_sync_interval(60000);
	esp_sntp_init();
}

void app_time_wait_sync(void)
{
	/* Wait for time to be set */
	time_t now = 0;
	struct tm timeinfo = { 0 };
	int retry = 0;

	while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && retry <= 120) {
		ESP_LOGI(TAG, "Waiting for system time to be set... (%d/120)", retry);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
		retry++;
	}

	time(&now);
	localtime_r(&now, &timeinfo);

	sprintf(sync_history, "%04d/%02d/%02d %02d:%02d:%02d", 1900 + timeinfo.tm_year, timeinfo.tm_mon,
		timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

	ESP_LOGI(TAG, "Now: %s", sync_history);
}

const char *app_time_get_sync_history(void)
{
	return sync_history;
}
