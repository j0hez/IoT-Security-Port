
//Este archivo debe reemplazar a motion_local para poder compilarlo. 
#include <unistd.h>
#include <letmecreate/letmecreate.h>
#include <letmecreate/core.h>
#include <stdio.h>
#include <stdlib.h>
#include "mosquitto.h"
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <mosquitto.h>

#define mqtt_host "test.mosquitto.org"
#define mqtt_port 1883
bool alarma = false;
static int run = 1;
int n_puerta =0;

static void desactivar_alarma(void)
{
    alarma = false;
    led_set(ALL_LEDS,0);
}

void handle_signal(int s)
{
    run = 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
    printf("connect callback, rc=%d\n", result);
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{

    printf("Mensaje recibido '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);

    sscanf(message->payload,"%d",&n_puerta);
   
    //ENCENDEMOS LOS LED EN BINARIO SEGUN EL NUMERO DE PUERTA
    led_set(ALL_LEDS,n_puerta);

    printf("Alarma activada de la puerta %d. Necesita apagado manual\n", n_puerta);
    printf("\n");
    alarma=true;
    while(alarma){};

    printf("Alarma desactivada!\n");

}

int main()
{
    char clientid[24];
    struct mosquitto *mosq;
    int rc = 0;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    mosquitto_lib_init();

    memset(clientid, 0, 24);
    snprintf(clientid, 23, "log_%d", getpid());
    mosq = mosquitto_new(clientid, true, 0);

    led_init();
    switch_init();
    switch_add_callback(SWITCH_1_PRESSED, desactivar_alarma);


    if(mosq){
        mosquitto_connect_callback_set(mosq, connect_callback);
        mosquitto_message_callback_set(mosq, message_callback);

        rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);


        mosquitto_subscribe(mosq, NULL, "rpiiot_puerta/#", 0);

        while(run){
            rc = mosquitto_loop(mosq, -1, 1);
            if(run && rc){
                printf("connection error!\n");
                sleep(10);
                mosquitto_reconnect(mosq);
            }
        }
        mosquitto_destroy(mosq);
    }

    mosquitto_lib_cleanup();
    led_release();
    switch_release();
    return rc;
}
