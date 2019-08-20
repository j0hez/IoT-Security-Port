#include <stdio.h>
#include <stdlib.h>
#include <letmecreate/letmecreate.h>
#include "mosquitto.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <ctype.h>



#define BUFFER_SIZE 64
#define SERVER_PORT 3000


// Server connection parameters
#define MQTT_HOSTNAME "test.mosquitto.org"
#define MQTT_PORT 1883
#define MQTT_TOPIC "rpiiotalert"

int main() {
    
//#########################   UDP CONF
    int sockfd;
    struct sockaddr_in6 my_addr;     /* informacion de mi direccion */
    struct sockaddr_in6 their_addr;  /* informacion de la direccion del cliente */
    char buffer_udp[BUFFER_SIZE];          /* buffer de recepcion */
    int numbytes;                   /* numero de bytes recibidos o enviados */
    size_t sin_size;

 /* crea el socket */
    if ((sockfd = socket (AF_INET6, SOCK_DGRAM, 0)) == -1)
    {
            perror ("socket");
            exit (1);
    }

    my_addr.sin6_family = AF_INET6;  /* Familia: ordenacion de bytes de la maquina */
    my_addr.sin6_port = htons (SERVER_PORT);  /* Puerto: ordenacion de bytes de la red */
    my_addr.sin6_addr = in6addr_any;     /* IP: ordenacion de bytes de la red */
    //memset (&(my_addr.sin6_zero), '\0', 8);  /* Pone a cero el resto de la estructura */

    /* asigna el socket a un puerto local */
    if (bind (sockfd, (void*) &my_addr, sizeof (struct sockaddr_in6)) == -1)
    {
            perror ("bind");
            exit (1);
    }

//#########################     MOSQUITTO CONF
    int ret=0;
    struct mosquitto *mosq;
    char buffer_mosq[BUFFER_SIZE];

    //Debe llamarse antes de usar cualquier otra funcionalidad
    mosquitto_lib_init();
    //Creamos el objeto mosquitto
    mosq = mosquitto_new(NULL,true,NULL);

    if (!mosq){
        fprintf(stderr,"Can ' t init Mosquitto library\n");
        exit(1);
    }
    
    //Nos conectamos al broker
    ret = mosquitto_connect(mosq, MQTT_HOSTNAME, MQTT_PORT,0);
    
    if (ret) {
        fprintf(stderr,"Can ' t connect\n");
        exit(1);
    }

//##############################    LOGICA 
    // cerramos la puerta por defecto
    relay2_click_disable_relay(MIKROBUS_2, RELAY2_CLICK_RELAY_1);

    int n_puerta = 0;

    printf ("(servidor) esperando mensajes [puerto local %d]\n", ntohs(my_addr.sin6_port));
    while(1){
        

        /* recibe un mensaje de un cliente */
        memset (buffer_udp, '\0', BUFFER_SIZE); /* Pone a cero el buffer inicialmente */
        sin_size = sizeof(struct sockaddr_in6);
        if ((numbytes = recvfrom (sockfd, buffer_udp, BUFFER_SIZE, 0,
                                  (void*)&their_addr, &sin_size))== -1)
        {
                perror ("recvfrom");
                continue;
        }
       
        //Detectamos que puerta es.
        strtok(buffer_udp, " ");
        sscanf(strtok(NULL, " "),"%d",&n_puerta);

        if(strcmp(buffer_udp,"MOTION-DETECT")==0){
            //abrimos puerta
            printf("ABRIENDO PUERTA %d\n", n_puerta);

            //Simulacion de puerta N
            relay2_click_enable_relay(MIKROBUS_2, RELAY2_CLICK_RELAY_1);
            sleep(4);
            printf("CERRANDO PUERTA %d\n", n_puerta);
            relay2_click_disable_relay(MIKROBUS_2, RELAY2_CLICK_RELAY_1);
        }
        else if(strcmp(buffer_udp,"IR-DETECT")==0){
            //publicamos
            printf("Dectado intruso por IR-DETECT puerta %d\n", n_puerta);
           
            sprintf(buffer_mosq,"%d",n_puerta);
            ret = mosquitto_publish(mosq,NULL,MQTT_TOPIC,strlen(buffer_mosq),&buffer_mosq,0,false);
            if (ret) {
                fprintf(stderr,"Cant publish\n");
                exit (-1);
            }
        }
        else{
            //DEBUG
            printf("DEBUG: %s\n",buffer_udp);
        }
       
    }

    //Eliminamos el objeto mosquitto
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
return 0;
}