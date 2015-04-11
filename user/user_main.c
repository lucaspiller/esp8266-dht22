#include <osapi.h>
#include <user_interface.h>
#include "user_config.h"
#include "sensor_logger.h"

void ICACHE_FLASH_ATTR user_init()
{
    char ssid[32] = SSID;
    char password[64] = SSID_PASSWORD;
    struct station_config stationConf;

    // Set station mode
    wifi_set_opmode( 0x1 );

    // Set ap settings
    os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 64);
    wifi_station_set_config(&stationConf);

    // Initialize UART0
    uart_div_modify(0, UART_CLK_FREQ / 115200);

    sensor_logger_init();

    os_printf("\r\n** Started **\r\n");
}
