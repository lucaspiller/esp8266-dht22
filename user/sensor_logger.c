#include <osapi.h>
#include <user_interface.h>
#include "dht22.h"
#include "sensor_logger.h"

static volatile os_timer_t timer;

static void ICACHE_FLASH_ATTR timer_callback(void *arg)
{
    os_printf("Awake\r\n");
    os_timer_disarm(&timer);
    bool result = dht22_read();

    if (result == 0) {
      os_timer_arm(&timer, 30000, 1);
      os_printf("Sleeping for 30s\r\n");
    } else {
      os_timer_arm(&timer, 2500, 1);
      os_printf("Sleeping for 2.50s\r\n");
    }
}

void ICACHE_FLASH_ATTR sensor_logger_init() {
    dht22_init();

    os_timer_disarm(&timer);
    os_timer_setfn(&timer, (os_timer_func_t *)timer_callback, NULL);
    os_timer_arm(&timer, 5000, 1);
}
