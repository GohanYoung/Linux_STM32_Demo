#include "app_main.h"
#include "oled.h"
#include "dht11.h"
#include "bsp_sys.h"
#include <stdio.h>

void app_main(void)
{
    BSP_Delay_ms(200);

    Device_OLED_Init();
    Device_OLED_Clear();

    Device_DHT11_Init();

    SensorDHT11_t env_sensor = {0};
    char display_buf[20];

    while (1) {
        Device_OLED_Clear();

        if (Device_DHT11_Read(&env_sensor) == DEV_OK) {
            if (env_sensor.temperature >= 0 && env_sensor.temperature <= 50) {
                int temp_int = (int)env_sensor.temperature;
                int temp_frac = (int)(env_sensor.temperature * 10.0f) % 10;

                snprintf(display_buf, sizeof(display_buf), "TEMP: %d.%d C", temp_int, temp_frac);
                Device_OLED_ShowString(20, 1, display_buf);
            } else {
                Device_OLED_ShowString(20, 1, "TEMP: ERR");
            }

            if (env_sensor.humidity >= 20 && env_sensor.humidity <= 90) {
                int humi_int = (int)env_sensor.humidity;
                int humi_frac = (int)(env_sensor.humidity * 10.0f) % 10;

                snprintf(display_buf, sizeof(display_buf), "HUMI: %d.%d %%", humi_int, humi_frac);
                Device_OLED_ShowString(20, 3, display_buf);
            } else {
                Device_OLED_ShowString(20, 3, "HUMI: ERR");
            }
        } else {
            Device_OLED_Clear();
            Device_OLED_ShowString(15, 1, "DHT11 ERROR!");
            Device_OLED_ShowString(15, 3, "Check wiring!");
        }

        BSP_Delay_ms(1500);
    }
}