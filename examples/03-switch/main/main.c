#include <stdio.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#include <driver/gpio.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>

#include "iot_button.h"

#define DEVICE_MANUFACTURER     "PuHome"
#define DEVICE_SERIAL           "1234567890"
#define DEVICE_MODEL            "MySWITCH"
#define FW_VERSION              "1.0"

const int led_gpio = 2;
const int button_gpio = 0;

static button_handle_t btn = NULL;

void switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context);
void on_wifi_ready();

homekit_characteristic_t name           = HOMEKIT_CHARACTERISTIC_(NAME, "SWITCH");
homekit_characteristic_t manufacturer   = HOMEKIT_CHARACTERISTIC_(MANUFACTURER,  DEVICE_MANUFACTURER);
homekit_characteristic_t serial         = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, DEVICE_SERIAL);
homekit_characteristic_t model          = HOMEKIT_CHARACTERISTIC_(MODEL,         DEVICE_MODEL);
homekit_characteristic_t revision       = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, FW_VERSION);
homekit_characteristic_t switch_on = HOMEKIT_CHARACTERISTIC_(
    ON, false, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(switch_on_callback)
);

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
            .ssid = "maxinf",
            .password = "maxinf951!",
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void led_write(bool on) {
    gpio_set_level(led_gpio, on ? 1 : 0);
}

void led_init() {
    gpio_set_direction(led_gpio, GPIO_MODE_OUTPUT);
    led_write(false);
}

void btn_tap_cb()
{
	printf("Toggling relay\n");
	switch_on.value.bool_value = !switch_on.value.bool_value;
	led_write(switch_on.value.bool_value);
	homekit_characteristic_notify(&switch_on, switch_on.value);
}

void button_init()
{
    btn = iot_button_create((gpio_num_t)button_gpio, BUTTON_ACTIVE_LOW);
    iot_button_set_evt_cb(btn, BUTTON_CB_TAP, btn_tap_cb, "TAP");
}

void switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    led_write(switch_on.value.bool_value);
}

void switch_identify_task(void *_args) {
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            led_write(true);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            led_write(false);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        vTaskDelay(250 / portTICK_PERIOD_MS);
    }

    led_write(false);

    vTaskDelete(NULL);
}

void switch_identify(homekit_value_t _value) {
    printf("LED identify\n");
    xTaskCreate(switch_identify_task, "LED identify", 512, NULL, 2, NULL);
}

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_lightbulb, .services=(homekit_service_t*[]){
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
            &name,
            &manufacturer,
            &serial,
            &model,
            &revision,
            HOMEKIT_CHARACTERISTIC(IDENTIFY, switch_identify),
            NULL
        }),
        HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Switch"),
            &switch_on,
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
	
    led_init();
	button_init();	
    wifi_init();

}
