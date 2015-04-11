#ifndef DHT22_H_
#define DHT22_H_

typedef struct
{
    int16_t  temperature;
    uint16_t humidity;
} dht22_t;

extern int dht22_init();
extern int dht22_read();

#endif
