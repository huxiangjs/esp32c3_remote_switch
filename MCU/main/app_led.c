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
 * refs: esp-idf/examples/peripherals/ledc/ledc_basic/main/ledc_basic_example_main.c
 *
 */

#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "hal/gpio_types.h"

#define LED_RED_IO          	GPIO_NUM_4
#define LED_GREEN_IO          	GPIO_NUM_5

void app_led_red_on(void)
{
	/* Set duty to 50% */
	// ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 4096));
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0x1f00));
	/* Update duty to apply the new value */
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));

	/* Set duty to 50% */
	// ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 4096));
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0x1fff));
	/* Update duty to apply the new value */
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1));
}

void app_led_green_on(void)
{
	/* Set duty to 50% */
	// ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 4096));
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0x1fff));
	/* Update duty to apply the new value */
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));

	/* Set duty to 50% */
	// ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 4096));
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0x1f00));
	/* Update duty to apply the new value */
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1));
}

void app_led_init(void)
{
	/* Prepare and then apply the LEDC PWM timer configuration */
	ledc_timer_config_t ledc_timer = {
		.speed_mode       = LEDC_LOW_SPEED_MODE,
		.timer_num        = LEDC_TIMER_0,
		.duty_resolution  = LEDC_TIMER_13_BIT,	/* Set duty resolution to 13 bits */
		.freq_hz          = 5000,		/* Frequency in Hertz. Set frequency at 5 kHz */
		.clk_cfg          = LEDC_AUTO_CLK
	};
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	/* Prepare and then apply the LEDC PWM channel configuration */
	ledc_channel_config_t ledc_channel = {
		.speed_mode     = LEDC_LOW_SPEED_MODE,
		.channel        = LEDC_CHANNEL_0,
		.timer_sel      = LEDC_TIMER_0,
		.intr_type      = LEDC_INTR_DISABLE,
		.gpio_num       = LED_RED_IO,
		.duty           = 0, 			/* Set duty to 0% */
		.hpoint         = 0
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
	ledc_channel.gpio_num = LED_GREEN_IO;
	ledc_channel.channel = LEDC_CHANNEL_1;
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

	app_led_red_on();
}
