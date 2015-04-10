#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"
#include "os_type.h"
#include "net_sender.h"
#include "user_config.h"

struct pokerface {
    struct espconn esp_conn;
    esp_tcp esptcp;
    char databuf[128];
    int datalen;
    /*volatile*/ os_timer_t conn_checker;
};

static void conn_checker_handler(void *arg)
{
    struct pokerface *p = arg;
    os_free(p);
}

static void connected(void *arg)
{
    struct pokerface *p = arg;
    espconn_sent(&p->esp_conn, (uint8*)p->databuf, p->datalen);
}

static void disconnected(void *arg)
{
    os_printf("Sent ok\n");
    struct pokerface *p = arg;
    os_timer_arm(&p->conn_checker, 50, 0);
}

static void reconnect(void *arg, sint8 err)
{
    os_printf("Error sending %d\n", err);
    struct pokerface *p = arg;
    espconn_disconnect(&p->esp_conn);
}

static void datasent(void *arg)
{
    struct pokerface *p = arg;
    os_timer_disarm(&p->conn_checker);
    os_timer_setfn(&p->conn_checker, (os_timer_func_t *)conn_checker_handler, p);
    espconn_disconnect(&p->esp_conn);
}


static int do_send(char* target, int port, char* data)
{
    struct pokerface *p = (struct pokerface*) os_zalloc(sizeof(struct pokerface));
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

    //os_printf("Sending %d bytes to %s:%d\n", p->datalen, target, port);
    espconn_connect(&p->esp_conn);

    return 0;
}

void ICACHE_FLASH_ATTR net_sender_init() {
    os_printf("network init\n");
    do_send(TARGET_IP, TARGET_PORT, "test\n");
}
