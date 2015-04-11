#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"
#include "os_type.h"
#include "tcp_client.h"

struct pconn {
    struct espconn esp_conn;
    esp_tcp esptcp;
    char databuf[512];
    int datalen;
    /*volatile*/ os_timer_t conn_checker;
};

static void ICACHE_FLASH_ATTR conn_checker_handler(void *arg)
{
    struct pconn *p = arg;
    os_free(p);
}

static void ICACHE_FLASH_ATTR connected(void *arg)
{
    struct pconn *p = arg;
    espconn_sent(&p->esp_conn, (uint8*)p->databuf, p->datalen);
}

static void ICACHE_FLASH_ATTR disconnected(void *arg)
{
    os_printf("Sent ok\n");
    struct pconn *p = arg;
    os_timer_arm(&p->conn_checker, 50, 0);
}

static void ICACHE_FLASH_ATTR reconnect(void *arg, sint8 err)
{
    os_printf("Error sending %d\n", err);
    struct pconn *p = arg;
    espconn_disconnect(&p->esp_conn);
}

static void ICACHE_FLASH_ATTR datasent(void *arg)
{
    struct pconn *p = arg;
    os_timer_disarm(&p->conn_checker);
    os_timer_setfn(&p->conn_checker, (os_timer_func_t *)conn_checker_handler, p);
    espconn_disconnect(&p->esp_conn);
}

int ICACHE_FLASH_ATTR tcp_client_send(char* target, int port, char* data)
{
    struct pconn *p = (struct pconn*) os_zalloc(sizeof(struct pconn));
    if (!p) {
        os_printf("Can't malloc enough to send\n");
        return -1;
    }

    p->esp_conn.type = ESPCONN_TCP;
    p->esp_conn.state = ESPCONN_NONE;
    p->esp_conn.proto.tcp = &p->esptcp;
    p->esp_conn.proto.tcp->local_port = espconn_port();
    p->esp_conn.proto.tcp->remote_port = port;

    p->databuf[0] = 0;
    if (strlen(p->databuf) + strlen(data + 1) >= sizeof(p->databuf)) {
        os_printf("Total length of data exceeds buffer (max: %d characters)\n", sizeof(p->databuf)-1);
        os_free(p);
        return -1;
    }
    strcat(p->databuf, data);
    p->datalen = strlen(p->databuf);

    uint32_t target_addr = ipaddr_addr(target);
    os_memcpy(p->esp_conn.proto.tcp->remote_ip, &target_addr, 4);
    espconn_regist_connectcb(&p->esp_conn, connected);
    espconn_regist_reconcb(&p->esp_conn, reconnect);
    espconn_regist_disconcb(&p->esp_conn, disconnected);
    espconn_regist_sentcb(&p->esp_conn, datasent);

    os_printf("Sending %d bytes to %s:%d\n", p->datalen, target, port);
    espconn_connect(&p->esp_conn);

    return 0;
}
