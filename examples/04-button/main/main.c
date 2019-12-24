#include <stdio.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/gpio.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>

#include "iot_button.h"

#define DEVICE_MANUFACTURER     "PuHome"
#define DEVICE_SERIAL           "1234567890"
#define DEVICE_MODEL            "MyBUTTON"
#define FW_VERSION              "1.0"

const int button_gpio = 0;

static button_handle_t btn = NULL;

void on_wifi_ready();

homekit_characteristic_t name           = HOMEKIT_CHARACTERISTIC_(NAME, "PuHome");
homekit_characteristic_t manufacturer   = HOMEKIT_CHARACTERISTIC_(MANUFACTURER,  DEVICE_MANUFACTURER);
homekit_characteristic_t serial         = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, DEVICE_SERIAL);
homekit_characteristic_t model          = HOMEKIT_CHARACTERISTIC_(MODEL,         DEVICE_MODEL);
homekit_characteristic_t revision       = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, FW_VERSION);
homekit_characteristic_t button_event   = HOMEKIT_CHARACTERISTIC_(PROGRAMMABLE_SWITCH_EVENT, 0);

static char *device_name_get(void)
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    int name_len = snprintf(NULL, 0, "PuHome-%02X%02X%02X",
                            mac[3], mac[4], mac[5]);
    char *name_value = malloc(name_len+1);
    snprintf(name_value, name_len+1, "PuHome-%02X%02X%02X",
             mac[3], mac[4], mac[5]);

    return name_value;
}

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
            printf("STA start\n");
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            printf("WiFI ready\n");
            on_wifi_ready();
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            printf("STA disconnected\n");
            esp_wifi_connect();
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void wifi_init() {
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "max-inf",
            .password = "maxinf951!",
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void btn_tap_cb()
{
	printf("single press\n");
	homekit_characteristic_notify(&button_event, HOMEKIT_UINT8(0));
}

void btn_long_cb()
{
	printf("long press\n");
	homekit_characteristic_notify(&button_event, HOMEKIT_UINT8(2));
}

void button_init()
{
    btn = iot_button_create((gpio_num_t)button_gpio, BUTTON_ACTIVE_LOW);
    iot_button_set_evt_cb(btn, BUTTON_CB_TAP, btn_tap_cb, "TAP");
    iot_button_add_custom_cb(btn, 5, btn_long_cb, "LONG");
}


void button_identify(homekit_value_t _value) {
    printf("BUTTON identify\n");
    
}

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(
        .id=1,
        .category=homekit_accessory_category_programmable_switch, 
        .services=(homekit_service_t*[]){
            HOMEKIT_SERVICE(
                ACCESSORY_INFORMATION, 
                .characteristics=(homekit_characteristic_t*[]){
                &name,
                &manufacturer,
                &serial,
                &model,
                &revision,
                HOMEKIT_CHARACTERISTIC(IDENTIFY, button_identify),
                NULL
            }),
            HOMEKIT_SERVICE(
                STATELESS_PROGRAMMABLE_SWITCH, 
                .primary=true, 
                .characteristics=(homekit_characteristic_t*[]){
                HOMEKIT_CHARACTERISTIC(NAME, "Button"),
                &button_event,
                NULL
            }),
        NULL
    }),
    NULL
};

homekit_server_config_t config = {
    .accessories = accessories,
    .password = "111-11-111",
    .setupId="1SP0",
};

void on_wifi_ready() {
    homekit_server_init(&config);
}

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
	
	name.value = HOMEKIT_STRING(device_name_get());

	button_init();	
    wifi_init();

}
