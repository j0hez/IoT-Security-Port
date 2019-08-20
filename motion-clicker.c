
#include <stdio.h>

#include <contiki.h>
#include <contiki-net.h>

#include "dev/leds.h"
#include "letmecreate/core/network.h"
#include "letmecreate/core/common.h"
#include "letmecreate/core/debug.h"
#include "letmecreate/click/motion.h"
#include <pic32_gpio.h>

#define SERVER_IP_ADDR "fe80::19:f5ff:fe89:1922"
//#define SERVER_IP_ADDR "fe80::19:f5ff:fe89:1e7a"
#define SERVER_PORT 3000
#define CLIENT_PORT 3001
#define n_puerta 1
#define BUFFER_SIZE 64

static struct uip_udp_conn * conn;
static char buffer[BUFFER_SIZE];
static int i = 0;

void motion_callback(uint8_t event)
{
    
  if(GPIO_VALUE(D,0))
  {
        leds_on(LED1);


        sprintf(buffer,"MOTION-DETECT %d", n_puerta);
        udp_packet_send(conn, buffer, strlen(buffer));
        
        /* Delay for 500ms */
        for( i=0;i<=500;++i) {
            clock_delay_usec(1000);
        }
        leds_off(LED1);
  }
}

PROCESS(main_process, "Main process");
AUTOSTART_PROCESSES(&main_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(main_process, ev, data)
{
    PROCESS_BEGIN();
    INIT_NETWORK_DEBUG();
    {
        conn = udp_new_connection(CLIENT_PORT, SERVER_PORT, SERVER_IP_ADDR);
        
        leds_on(LED2);

        sprintf(buffer,"MOTION-ON %d", n_puerta);
        udp_packet_send(conn, buffer, strlen(buffer));
        PROCESS_WAIT_UDP_SENT();

        motion_click_enable(MIKROBUS_1);
        motion_click_attach_callback(MIKROBUS_1, motion_callback);
        /* DEBUG
        while(1)
        {
            
            sprintf(buffer,"MOTION-ALIVE 1");
            udp_packet_send(conn, buffer, strlen(buffer));
            PROCESS_WAIT_UDP_SENT();

            for( i=0;i<=1000;++i) {
                clock_delay_usec(1000);
            }
        
        }*/

    }

    PROCESS_END();
}

/*---------------------------------------------------------------------------*/