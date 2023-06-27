#include <stdio.h>
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"

static const char *TAG = "WIFI";

/**
 * @brief 扫描wifi
 */
static void wifi_scan()
{
    wifi_country_t country_cfg = {
        .cc = "CN",
        .schan = 1,
        .nchan = 13,
        .policy = WIFI_COUNTRY_POLICY_AUTO,
    };

    // 配置国家信息
    esp_wifi_set_country(&country_cfg);

ESP_LOGI(TAG,"开始扫描网络");
    wifi_scan_config_t scan_cfg = {
        .show_hidden = true, // 扫描隐藏网络
    };

    esp_wifi_scan_start(&scan_cfg, true);

    // 获取扫描结果
    uint16_t ap_num;                                    // 扫描到 AP 的数量
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_num)); // 获取 扫描到的 热点数量
    ESP_LOGI(TAG, "扫描到热点数量： %d", ap_num);

    uint16_t max_aps = 20;
    wifi_ap_record_t ap_records[max_aps];
    memset(ap_records, 0, sizeof(wifi_ap_record_t)); // 归零
    uint16_t aps_count = max_aps;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&aps_count, ap_records)); // 只取前 20 条热点的信息
    printf("%20s   %s   %s   %s\n", "SSID", "频道", "强度", "MAC");
    // 输出 热点信息
    for (int i = 0; i < aps_count; i++)
    {
        printf("%30s    %3d    %3d    %02X-%02X-%02X-%02X-%02X-%02X\n", ap_records[i].ssid, ap_records[i].primary, ap_records[i].rssi, ap_records[i].bssid[0], ap_records[i].bssid[1], ap_records[i].bssid[2], ap_records[i].bssid[3], ap_records[i].bssid[4], ap_records[i].bssid[5]);
    }
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));
}

void app_main(void)
{
    // 0 初始化 NVS
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_LOGI(TAG, "NVS初始化成功");

    // 1.1 创建LwIP
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_LOGI(TAG, "LwIP初始化成功");

    // 1.2 创建事件循环
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_LOGI(TAG, "事件环初始化成功");

    // 1.3.1 初始化网络接口
    esp_netif_t *netif_sta = esp_netif_create_default_wifi_sta();
    ESP_LOGI(TAG, "网络接口初始化成功");

    // 1.3.2 初始化 wi-fi 驱动
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    ESP_LOGI(TAG, "wi-fi 初始化成功");

    // 2.1 配置WIFI
    esp_wifi_set_mode(WIFI_MODE_STA);

    // 3 连接WIFI



    // 扫描
    wifi_scan();
    vTaskDelete(NULL);
}
