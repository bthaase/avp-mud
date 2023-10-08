/* 
 * 2023, Andrew "Miros" Budd 
 * let's send some messages.
 * try to keep it simple, and avp/lotj common denominator
 */


#include <stdio.h>
#include <mosquitto.h>
#include "mud.h"
#include "mqtt.h"
#include <sys/select.h>
#include <sys/time.h>

struct mosquitto *mosq = NULL;

void subscribe_to_topics() {
    // subscribe to topics
    char buf[MAX_STRING_LENGTH];
    // all input topics
    snprintf(buf, MAX_STRING_LENGTH, "%s/in/#", sysdata.exe_file);
    mosquitto_subscribe(mosq, NULL, buf, 0);
}

// disconnect from the broker and cleanup
void mqtt_cleanup()
{
    if(!mosq) return;
    log_string("MQTT disconnecting...");
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    mosq = NULL;
}

// on connect callback
void mqtt_on_connect(struct mosquitto *mosq, void *userdata, int result) {
    log_string("MQTT connected.");
    if(result) {
        char buf[MAX_STRING_LENGTH];
        snprintf(buf, MAX_STRING_LENGTH, "MQTT connection failed: %s\n", mosquitto_strerror(result));
        log_string(buf);
        mqtt_cleanup();
        return;
    }
    // subscribe to topics
    subscribe_to_topics();
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

// init library and connect to the broker
bool mqtt_init()
{
    if(mosq) return true; // already connected
    if(!sysdata.mqtt_enabled) return false; // not enabled
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
    // on connect
    mosquitto_connect_callback_set(mosq, mqtt_on_connect);
    // connect to broker
    rc = mosquitto_connect_async(mosq, sysdata.mqtt_host, sysdata.mqtt_port, 60);
    if(rc){
        char buf[MAX_STRING_LENGTH];
        snprintf(buf, MAX_STRING_LENGTH, "Unable to connect to MQTT broker: %s\n", mosquitto_strerror(rc));
        log_string(buf);
        mqtt_cleanup();
        return false;
    }
    return true;
}


void mqtt_update(int time_limit) {
    static int retry_connection=0;
    if(!mosq && sysdata.mqtt_enabled) {
        if(--retry_connection < 0 ) {
            log_string("(Re)connecting to MQTT...");
            retry_connection = 60 * PULSE_PER_SECOND;
            mqtt_init();
        }
        return;
    } else {
        retry_connection = 0;
        if(!sysdata.mqtt_enabled) {
            mqtt_cleanup();
            return;
        }
    }

    // Get the current time
    struct timeval start, current;
    gettimeofday(&start, NULL);

    bool want_read = false;

    do {
        int rc = mosquitto_loop(mosq, 0, 1);
        if(rc){
            char buf[MAX_STRING_LENGTH];
            snprintf(buf, MAX_STRING_LENGTH, "MQTT error: %s\n", mosquitto_strerror(rc));
            log_string(buf);
            mqtt_cleanup();
            break;
        }
         // Check elapsed time
        gettimeofday(&current, NULL);
        long elapsed_time = (current.tv_sec - start.tv_sec) * 1000L + 
                            (current.tv_usec - start.tv_usec) / 1000L;  // Convert to milliseconds

        if ( elapsed_time > time_limit ) {
            break;
        }
        int socket_fd = mosquitto_socket(mosq);
        if (socket_fd == -1) {
            // Handle error
            return;
        }

        fd_set read_fds;
        struct timeval timeout;
        FD_ZERO(&read_fds);
        FD_SET(socket_fd, &read_fds);

        timeout.tv_sec = 0;
        timeout.tv_usec = 1000;  // Set timeout for select as 1 millisecond for fine granularity

        int activity = select(socket_fd + 1, &read_fds, NULL, NULL, &timeout);

        if (activity < 0) {
            // Handle error
            break;
        } else if (activity > 0 && FD_ISSET(socket_fd, &read_fds)) {
            want_read = true;
        }

    } while( mosquitto_want_write(mosq) || want_read );
}

void mqtt_publish(const char *topic, const char *message) {
    if(!mosq) return;
    char publish_topic[1024];
    snprintf(publish_topic, 1024, "%s/%s", sysdata.exe_file, topic);
    mosquitto_publish(mosq, NULL, strlower(publish_topic), strlen(message), message, 0, false);
}
