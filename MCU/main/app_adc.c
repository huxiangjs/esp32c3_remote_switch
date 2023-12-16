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
 * refs: esp-idf/examples/peripherals/adc/single_read/single_read/main/single_read.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

static const char *TAG = "app-adc";

#define ADC_DETECT_VOLTAGE		2000	/* 2000mv */

static esp_adc_cal_characteristics_t adc1_chars;
static bool cali_enable = false;

void app_adc_init(void)
{
	esp_err_t ret;

	ret = esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP);
	if (ret == ESP_ERR_NOT_SUPPORTED) {
		ESP_LOGW(TAG, "Calibration scheme not supported, skip software calibration");
	} else if (ret == ESP_ERR_INVALID_VERSION) {
		ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
	} else if (ret == ESP_OK) {
		cali_enable = true;
		esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
	} else {
		ESP_LOGE(TAG, "Invalid arg");
	}

	ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
	ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_12));
}

int app_adc_get_voltage(void)
{
	int adc_raw;
	uint32_t voltage = 0;

	adc_raw = adc1_get_raw(ADC1_CHANNEL_1);
	// ESP_LOGI(TAG, "raw  data: %d", adc_raw);
	if (cali_enable) {
		voltage = esp_adc_cal_raw_to_voltage(adc_raw, &adc1_chars);
		// ESP_LOGI(TAG, "cali data: %d mV", voltage);
	}

	/*
	 * Restore true voltage by scaling.
	 * Pull-Up: 12K
	 * Pull-Down: 2K
	 */
	voltage = voltage * (12 + 2) / 2;

	return voltage;
}

bool app_adc_detect(void)
{
	return (bool)(app_adc_get_voltage() > ADC_DETECT_VOLTAGE);
}
