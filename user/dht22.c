#include <osapi.h>
#include <user_interface.h>
#include <gpio.h>
#include "dht22.h"

#define DHT_PIN 4
#define MAXTIMINGS 100
#define DHT_MAXCOUNT 1000
#define BREAKTIME 20

int ICACHE_FLASH_ATTR dht22_read() {
  int counter = 0;
  int laststate = 1;
  int i = 0;
  int bits_in = 0;

  int data[100];

  data[0] = data[1] = data[2] = data[3] = data[4] = 0;

  //disable interrupts, start of critical section
  os_intr_lock();

  // Wake up device, 250ms of high
  GPIO_OUTPUT_SET(DHT_PIN, 1);
  os_delay_us(250 * 1000);

  // Hold low for 20ms
  GPIO_OUTPUT_SET(DHT_PIN, 0);
  os_delay_us(30 * 1000);

  // High for 40ms
  GPIO_DIS_OUTPUT(DHT_PIN);
  os_delay_us(40);

  // wait for pin to drop?
  while (GPIO_INPUT_GET(DHT_PIN) == 1 && i < DHT_MAXCOUNT) {
    os_delay_us(1);
    if (i >= DHT_MAXCOUNT) {
      goto fail;
    }
    i++;
  }

  // read data!
  for (i = 0; i < MAXTIMINGS; i++) {
    // Count high time (in approx us)
    counter = 0;
    while (GPIO_INPUT_GET(DHT_PIN) == laststate) {
      counter++;
      os_delay_us(1);
      if (counter == DHT_MAXCOUNT)
        break;
    }
    laststate = GPIO_INPUT_GET(DHT_PIN);

    // store data after 3 reads
    if ((i > 3) && (i % 2 == 0)) {
      // shove each bit into the storage bytes
      data[bits_in / 8] <<= 1;
      if (counter > BREAKTIME) {
        data[bits_in / 8] |= 1;
      }
      bits_in++;
    }
  }

  //Re-enable interrupts, end of critical section
  os_intr_unlock();

  if (bits_in < 40) {
    os_printf("Got too few bits: %d should be at least 40\r\n", bits_in);
    goto fail;
  }

  int checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;

  if (data[4] != checksum) {
    os_printf("DHT: %02x %02x %02x %02x [%02x] CS: %02x\r\n",
              data[0], data[1], data[2], data[3], data[4], checksum);
    os_printf("Checksum was incorrect after %d bits. Expected %d but got %d\r\n",
              bits_in, data[4], checksum);
    goto fail;
  }

  sint16 temperature;
  uint16 humidity;

  humidity = data[0] * 256 + data[1];
  temperature = (data[2] & 0x7f) * 256 + data[3];
  if(data[2] & 0x80) temperature = -temperature;

  os_printf("Temperature: %d C, Humidity: %d %%\r\n", temperature, humidity);

  return 0;
fail:
  //Re-enable interrupts, end of critical section
  os_intr_unlock();

  os_printf("Failed to get reading, dying\r\n");
  return -1;
}

int ICACHE_FLASH_ATTR dht22_init()
{
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
    return 0;
}
