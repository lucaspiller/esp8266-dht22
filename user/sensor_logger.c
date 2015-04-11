#include <osapi.h>
#include <user_interface.h>
#include <mem.h>
#include "user_config.h"
#include "dht22.h"
#include "sensor_logger.h"

static volatile os_timer_t timer;
static volatile dht22_t *reading;

static void ICACHE_FLASH_ATTR construct_packet(char *buff)
{
    // Fields:
    // 1) packet type
    // 2) temperature
    // 3) humidity
    os_sprintf(buff, "1,%d,%d\r\n", reading->temperature, reading->humidity);
}

static void ICACHE_FLASH_ATTR timer_callback(void *arg)
{
    int result;
    char buff[16];

    os_printf("Awake\r\n");
    os_timer_disarm(&timer);

    // 1. take reading
    result = dht22_read(reading);
    if (result != 0) {
      os_timer_arm(&timer, 2500, 1);
      os_printf("Error taking reading - Retry in 2.50s\r\n");
      return;
    }

    // 2. send to server
    construct_packet((char*) &buff);
    result = tcp_client_send(TARGET_IP, TARGET_PORT, (char*) &buff);
    if (result != 0) {
      os_timer_arm(&timer, 2500, 1);
      os_printf("Error sending - Retry in 2.50s\r\n");
      return;
    }

    os_timer_arm(&timer, 30000, 1);
    os_printf("Sleeping for 30s\r\n");
}

void ICACHE_FLASH_ATTR sensor_logger_init() {
    dht22_init();

    reading = (dht22_t*) os_malloc(sizeof(dht22_t));
    if (!reading) {
        os_printf("FATAL: Failed to malloc %d bytes for dht22 reading\r\n", sizeof(reading));
        return;
    }

    os_timer_disarm(&timer);
    os_timer_setfn(&timer, (os_timer_func_t *)timer_callback, NULL);
    os_timer_arm(&timer, 5000, 1);
}
