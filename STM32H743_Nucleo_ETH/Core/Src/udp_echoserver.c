#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>

#define UDP_SERVER_PORT 7
#define UDP_CLIENT_PORT 7

static struct udp_pcb *upcb;
static int menu_sent = 0;

void udp_echoserver_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
void send_menu(struct udp_pcb *pcb, const ip_addr_t *addr, u16_t port);
void control_leds(int selection);

void udp_echoserver_init(void)
{
    struct udp_pcb *upcb;
    err_t err;


    upcb = udp_new();

    if (upcb)
    {

        err = udp_bind(upcb, IP_ADDR_ANY, UDP_SERVER_PORT);

        if(err == ERR_OK)
        {

            udp_recv(upcb, udp_echoserver_receive_callback, NULL);
        }
        else
        {
            udp_remove(upcb);
        }
    }
}

void send_menu(struct udp_pcb *pcb, const ip_addr_t *addr, u16_t port)
{
    const char menu[] =
        "*********************************\n"
        "Which LED do you want to light?\n"
        "1 - Red LED\n"
        "2 - Blue LED\n"
        "3 - Both LEDs\n"
        "*********************************\n";

    struct pbuf *menu_buf = pbuf_alloc(PBUF_TRANSPORT, strlen(menu), PBUF_RAM);
    if (menu_buf != NULL) {
        memcpy(menu_buf->payload, menu, strlen(menu));
        udp_sendto(pcb, menu_buf, addr, port);
        pbuf_free(menu_buf);
    }
}

void control_leds(int selection)
{
    switch (selection) {
        case 1:
            HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_RESET);
            break;
        case 2:
            HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_SET);
            break;
        case 3:
            HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_SET);
            break;
        default:
            HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_RESET);
            break;
    }
}

void udp_echoserver_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{

    udp_connect(pcb, addr, UDP_CLIENT_PORT);


    if (!menu_sent) {
        send_menu(pcb, addr, port);
        menu_sent = 1;
    }
    else if (p != NULL && p->len > 0) {

        char response = ((char*)p->payload)[0];
        int selection = response - '0';
        control_leds(selection);
    }


    pbuf_free(p);


    udp_disconnect(pcb);
}
