/* 
 * 2023, Andrew "Miros" Budd 
 * let's send some messages.
 * try to keep it simple, and avp/lotj common denominator
 */


#include <stdio.h>
#include <mosquitto.h>
#include "mud.h"
#include "mqtt.h"

struct mosquitto *mosq = NULL;

// disconnect from the broker and cleanup
void mqtt_cleanup()
{
    if(!mosq) return;
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    mosq = NULL;
}

// mqtt on message handler
void mqtt_on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
    char* topic = message->topic + strlen(sysdata.exe_file) + 1;
    if(!str_prefix("in/channel/", topic)) {
        topic += strlen("in/channel/");
        if(!str_cmp(topic, "ooc")) {
            // the message will be of the format <speaker>, <message>
            char *speaker = strtok(message->payload, ",");
            char *msg = strtok(NULL, ",");
            char buf[MAX_STRING_LENGTH];
            snprintf(buf, MAX_STRING_LENGTH, "%s: %s", speaker, msg);
            echo_to_all(AT_OOC, buf, ECHOTAR_ALL);
        } else if(!str_cmp(message->topic, "echo")) {
            echo_to_all(AT_IMMORT, message->payload, ECHOTAR_ALL);
        }
        return;
    }

}

void subscribe_to_topics() {
    // subscribe to topics
    char buf[MAX_STRING_LENGTH];
    // all input topics
    snprintf(buf, MAX_STRING_LENGTH, "%s/in/#", sysdata.exe_file);
    mosquitto_subscribe(mosq, NULL, buf, 0);
}

// init library and connect to the broker
bool mqtt_init()
{
    if(mosq) return true; // already connected
    if(NULLSTR(sysdata.mqtt_host)) {
        log_string("MQTT host not set, not connecting.");
        return false;
    }
    int rc;
    // init library
    mosquitto_lib_init();
    // create a new client instance
    mosq = mosquitto_new(sysdata.exe_file, true, NULL);
    if(!mosq){
        fprintf(stderr, "Error: Out of memory.\n");
        exit(1);
    }
    // set callbacks
    mosquitto_message_callback_set(mosq, mqtt_on_message);
    // connect to broker
    rc = mosquitto_connect(mosq, sysdata.mqtt_host, sysdata.mqtt_port, 60);
    if(rc){
        char buf[MAX_STRING_LENGTH];
        snprintf(buf, MAX_STRING_LENGTH, "Unable to connect to MQTT broker: %s\n", mosquitto_strerror(rc));
        log_string(buf);
        mqtt_cleanup();
        return false;
    }
    // subscribe to topics
    subscribe_to_topics();
}

void mqtt_update() {
    if(!mosq) return;
    int rc = mosquitto_loop(mosq, 0, 10);
    if(rc){
        char buf[MAX_STRING_LENGTH];
        snprintf(buf, MAX_STRING_LENGTH, "MQTT error: %s\n", mosquitto_strerror(rc));
        log_string(buf);
        mqtt_cleanup();
    }
}

