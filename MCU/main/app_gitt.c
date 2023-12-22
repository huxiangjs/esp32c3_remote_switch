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
#include <string.h>
#include <time.h>
#include <gitt_type.h>
#include "app_gitt.h"

static int app_gitt_get_date_impl(char *buf, uint8_t size)
{
	time_t cur_time;

	cur_time = time(NULL);
	if (cur_time == -1)
		return cur_time;

	sprintf(buf, "%ld", cur_time);

	return 0;
}

static int app_gitt_get_zone_impl(char *buf, uint8_t size)
{
	sprintf(buf, "+0800");

	return 0;
}

static void app_gitt_replace(char src, char tag, char *buf, int buf_size)
{
	int i;

	for (i = 0; i < buf_size; i++)
		buf[i] = buf[i] == src ? tag : buf[i];
}

static void app_gitt_remote_event_callback(struct gitt *g, struct gitt_device *device,
					   char *date, char *zone, char *event)
{
	struct app_gitt *app = gitt_containerof(g, struct app_gitt, g);
	int len = strlen(event);
	// char buf[128];

	// sprintf(buf, "%s <%s> %s %s: ", device->name, device->id, date, zone);
	// printf("%-64s", buf);

	app_gitt_replace('\n', ' ', event, len);
	// printf("%s\n", event);

	if (app->callback)
		app->callback(event);
}

int app_gitt_init(struct app_gitt *app, app_gitt_recv call)
{
	int ret = 0;

	app->callback = call;

	/* Initialize */
	app->g.privkey = app->privkey;
	app->g.url = app->repository;
	app->g.buf = app->buffer;
	app->g.buf_len = sizeof(app->buffer);
	app->g.remote_event = app_gitt_remote_event_callback;

	/*
	 * These two functions are optional,
	 * you can choose not to implement them.
	 */
	app->g.get_date = app_gitt_get_date_impl,
	app->g.get_zone = app_gitt_get_zone_impl,

	printf("Initialize...\n");
	ret = gitt_init(&app->g);
	printf("Initialize result: %s\n", GITT_ERRNO_STR(ret));
	if (ret)
		return ret;

	printf("HEAD: %s\n", app->g.repository.head);
	printf("Refs: %s\n", app->g.repository.refs);

	/* Device info */
	printf("Device name: %s\n", app->g.device.name);
	printf("Device id:   %s\n", app->g.device.id);

	return 0;
}
