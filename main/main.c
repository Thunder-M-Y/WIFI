#include <stdio.h>
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"

static const char *TAG = "WIFI";
static int s_retry_num = 0;     // 重连次数
#define ESP_MAXIMUM_RETRY 5     // WiFi 连接重试次数
#define WIFI_CONNECTED_BIT BIT0 // 连接成功
#define WIFI_FAIL_BIT BIT1      // 连接失败
/**
 * @brief 扫描wifi
 */
// static void wifi_scan()
// {
//     wifi_country_t country_cfg = {
//         .cc = "CN",
//         .schan = 1,
//         .nchan = 13,
//         .policy = WIFI_COUNTRY_POLICY_AUTO,
//     };

//     // 配置国家信息
//     esp_wifi_set_country(&country_cfg);

//     ESP_LOGI(TAG, "开始扫描网络");
//     wifi_scan_config_t scan_cfg = {
//         .show_hidden = true, // 扫描隐藏网络
//     };

//     esp_wifi_scan_start(&scan_cfg, true);

//     // 获取扫描结果
//     uint16_t ap_num;                                    // 扫描到 AP 的数量
//     ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_num)); // 获取 扫描到的 热点数量
//     ESP_LOGI(TAG, "扫描到热点数量： %d", ap_num);

//     uint16_t max_aps = 20;
//     wifi_ap_record_t ap_records[max_aps];
//     memset(ap_records, 0, sizeof(wifi_ap_record_t)); // 归零
//     uint16_t aps_count = max_aps;
//     ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&aps_count, ap_records)); // 只取前 20 条热点的信息
//     printf("%20s   %s   %s   %s\n", "SSID", "频道", "强度", "MAC");
//     // 输出 热点信息
//     for (int i = 0; i < aps_count; i++)
//     {
//         printf("%30s    %3d    %3d    %02X-%02X-%02X-%02X-%02X-%02X\n", ap_records[i].ssid, ap_records[i].primary, ap_records[i].rssi, ap_records[i].bssid[0], ap_records[i].bssid[1], ap_records[i].bssid[2], ap_records[i].bssid[3], ap_records[i].bssid[4], ap_records[i].bssid[5]);
//     }
//     ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));
// }

/**
 * @brief
 */
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "WiFi 启动成功，准备连接到 AP");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        ESP_LOGI(TAG, "WiFi 成功连接到 AP");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "重新连接到 AP");
        }
        else
        {

            ESP_LOGI(TAG, "连接 AP 失败");
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "获取IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
    }
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
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    // 注册事件环事件

    // 注册相关事件
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
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Wi-Fi 启动成功...");
    // 3 连接WIFI
    wifi_config_t wifi_cfg = {
        .sta = {
            .ssid = "3203",
            .password = "32033203",
        }};
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_LOGI(TAG, "设置 Wi-Fi 为 Station 模式");

    // 扫描
    // wifi_scan();
    vTaskDelete(NULL);
}
