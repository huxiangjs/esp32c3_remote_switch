#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "hello_world";

void app_main(void)
{
    int size = esp_get_free_heap_size();

    ESP_LOGI(TAG, "hello world! free heap size: %dbyte", size);
}
