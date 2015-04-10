#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "net_sender.h"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1

os_event_t user_procTaskQueue[user_procTaskQueueLen];

static void user_procTask(os_event_t *events);

static void ICACHE_FLASH_ATTR user_procTask(os_event_t *events)
{
}

// Timer function
static volatile os_timer_t timer;
static int i = 0;

static void ICACHE_FLASH_ATTR timer_callback(void *arg)
{
    os_printf(".");
    if (i++ == 3) {
      i = 0;
      os_printf("\r\n");
      net_sender_init();
    }
}

void ICACHE_FLASH_ATTR user_init()
{
    char ssid[32] = SSID;
    char password[64] = SSID_PASSWORD;
    struct station_config stationConf;

    //Set station mode
    wifi_set_opmode( 0x1 );

    //Set ap settings
    os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 64);
    wifi_station_set_config(&stationConf);

    // Initialize UART0
    uart_div_modify(0, UART_CLK_FREQ / 115200);

    os_timer_disarm(&timer);
    os_timer_setfn(&timer, (os_timer_func_t *)timer_callback, NULL);
    os_timer_arm(&timer, 1000, 1);

    os_printf("\r\n** Started **\r\n");
    system_os_task(user_procTask, user_procTaskPrio, user_procTaskQueue, user_procTaskQueueLen);
}
