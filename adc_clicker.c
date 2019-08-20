#include <stdio.h>

#include <contiki.h>
#include <contiki-net.h>

#include "dev/leds.h"
#include "letmecreate/core/network.h"
#include "letmecreate/core/common.h"
#include "letmecreate/core/debug.h"
#include <letmecreate/core/spi.h>

#define SERVER_IP_ADDR "fe80::19:f5ff:fe89:1922"

#define SERVER_PORT 3000
#define CLIENT_PORT 3001
#define MENSAJE "IR-DETECT 1"// IR-DETECT Numero puerta
#define BUFFER_SIZE 64

PROCESS(main_process, "Main process");
AUTOSTART_PROCESSES(&main_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(main_process, ev, data)
{
    PROCESS_BEGIN();
    INIT_NETWORK_DEBUG();
    {
        // Due to the way Contiki protothreads work this needs to be static,
        // otherwise the data will be lost when switching to a different thread
        static struct uip_udp_conn * conn;
        static char buffer[BUFFER_SIZE];
        static int i = 0;
        static uint8_t tx_buffer[3], rx_buffer[3];
        static uint16_t datos = 0, datosviejo=0, distancianormal=20;

        spi_init();
        spi_set_mode(MIKROBUS_1, SPI_MODE_3);

        conn = udp_new_connection(CLIENT_PORT, SERVER_PORT, SERVER_IP_ADDR);

        //udp_packet_send(conn, "Starting loop\n", strlen("Starting loop\n"));


        tx_buffer[0] = 0x01;  //   start bit
        tx_buffer[1] = 0x80; // [10[00] 0000]
                                 // CHN 0
        tx_buffer[2] = 0x00; // don't care


        while(1)
        {
            
            //udp_packet_send(conn, "Getting data SPI\n", strlen("Getting data SPI\n"));

           
            
            if (spi_transfer(tx_buffer, rx_buffer, sizeof(tx_buffer)) < 0) {
                fprintf(stderr, "accel: Failed to get measure from device.\n");
                return -1;
            }
            
          
            //    rx_buffer[0]
            //    rx_buffer[1] dos ultimos bits, son los BMS del dato devuelto por el canal
            //    rx_buffer[2] resto de bits del dato                  

            datos=((0x03 & rx_buffer[1])*256)+ rx_buffer[2];
            if(datos<16)datos=16;
            datos= 2076/(datos-11);
            sprintf(buffer,"IR:%u ALIVE",datos);
            udp_packet_send(conn, buffer, strlen(buffer));
           
            PROCESS_WAIT_UDP_SENT();
            if(datos<distancianormal){
              if(datosviejo>datos+5){
                udp_packet_send(conn, MENSAJE, strlen(MENSAJE));
                PROCESS_WAIT_UDP_SENT();
              }
            }
            datosviejo=datos;
            for( i=0;i<=1000;++i) { 
                clock_delay_usec(500);
                leds_on(LED1);
            }
            leds_off(LED1);
        }
        spi_release();
    }

    PROCESS_END();
}

/*---------------------------------------------------------------------------*/







