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

#ifndef __APP_GITT_H_
#define __APP_GITT_H_

#include <gitt.h>
#include <gitt_errno.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void (*app_gitt_recv)(char *data);

struct app_gitt {
	struct gitt g;
	char privkey[1024];
	char repository[128];
	char dev_name[GITT_DEVICE_NAME_SIZE];
	char dev_id[GITT_DEVICE_ID_SIZE];
	char wifi_ssid[32];
	char wifi_password[32];
	uint8_t buffer[4096];
	uint8_t interval;
	app_gitt_recv callback;
};

int app_gitt_init(struct app_gitt *app, app_gitt_recv call);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __APP_GITT_H_ */
