extern void mqtt_cleanup();
extern bool mqtt_init();
extern void mqtt_update(int time_limit);
extern void mqtt_publish(const char *topic, const char *message);