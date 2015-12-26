#include <osapi.h>
#include <user_interface.h>
#include <mem.h>
#include "user_config.h"
#include "dht22.h"
#include "tcp_client.h"
#include "sensor_logger.h"

#define STATE_READING  0
#define STATE_WIFI     1
#define STATE_SEND     2
#define STATE_SENDING  3

static volatile os_timer_t timer;
static volatile dht22_t *reading;
static volatile int state;
static volatile int send_state;
static volatile int failures;

static void ICACHE_FLASH_ATTR construct_packet(char *buff)
{
    // Fields:
    // 1) packet type
    // 2) temperature
    // 3) humidity
    os_sprintf(buff, "1,%d,%d\r\n", reading->temperature, reading->humidity);
}

static int ICACHE_FLASH_ATTR check_wifi()
{
    if (wifi_station_get_connect_status() != STATION_GOT_IP)
    {
        return -1;
    } else {
        return 0;
    }
}

void ICACHE_FLASH_ATTR send_callback(int state)
{
    send_state = state;
}

static void ICACHE_FLASH_ATTR timer_callback(void *arg)
{
    int result;
    char buff[16];

    os_timer_disarm(&timer);

    if (failures > 30) {
        os_printf("Too many failures - rebooting!\r\n");
        system_restart();
    }

    // 1. take reading
    if (state == STATE_READING) {
        result = dht22_read(reading);
        if (result != 0) {
          failures++;
          os_timer_arm(&timer, 500, 1);
          os_printf("Error taking reading - Retry in 0.5s\r\n");
          return;
        }
        state = STATE_WIFI;
    }

    // 2. Check wifi
    if (state == STATE_WIFI) {
        result = check_wifi();
        if (result != 0) {
          failures++;
          os_timer_arm(&timer, 500, 1);
          os_printf("Not connected to wifi - Retry in 0.5s\r\n");
          return;
        }
        state = STATE_SEND;
    }

    // 3. send to server
    if (state == STATE_SEND) {
        construct_packet((char*) &buff);
        result = tcp_client_send(TARGET_IP, TARGET_PORT, (char*) &buff, send_callback);
        if (result != 0) {
          failures++;
          os_timer_arm(&timer, 500, 1);
          os_printf("Error sending - Retry in 0.5s\r\n");
          return;
        }
        send_state = TCP_CLIENT_SENDING;
        state = STATE_SENDING;
    }

    if (state == STATE_SENDING) {
        switch(send_state) {
            case TCP_CLIENT_SENDING:
                failures++;
                os_timer_arm(&timer, 500, 1);
                os_printf("Send in progress - Retry in 0.5s\r\n");
                return;
            case TCP_CLIENT_ERROR:
                failures++;
                state = STATE_SEND;
                os_timer_arm(&timer, 500, 1);
                os_printf("Error sending - Retry in 0.5s\r\n");
                return;
            case TCP_CLIENT_SENT:
                os_printf("Sent!\r\n");
                break;
        }
    }

    state = STATE_READING;
    failures = 0;

    os_printf("Entering deep sleep for 300s\r\n");
    system_deep_sleep(300 * 1000 * 1000);
    //os_timer_arm(&timer, 10000, 1);
    //os_printf("Sleeping for 10s\r\n");
}

void ICACHE_FLASH_ATTR sensor_logger_init() {
    dht22_init();

    failures = 0;
    state = STATE_READING;
    reading = (dht22_t*) os_malloc(sizeof(dht22_t));
    if (!reading) {
        os_printf("FATAL: Failed to malloc %d bytes for dht22 reading\r\n", sizeof(reading));
        return;
    }

    os_timer_disarm(&timer);
    os_timer_setfn(&timer, (os_timer_func_t *)timer_callback, NULL);
    os_timer_arm(&timer, 7500, 1);
}
