#ifndef MAIN_H__
#define MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#define APP_VERSION         "2.0.0.11"
#define HW_VERSION          "2.0"
#define CONFIG_FILE         "/config.json"
#define CONFIG_TMP_FILE     "/config.tmp.json"
#define WIFI_CONF_FILE      "/wifi.json"
#define WIFI_LOCK_FILE      "/wifi.lock"

#define OK_MSG      		F("<meta http-equiv=refresh content=10;url=app.htm>Saved. The page is redirecting.")

#define debug_println(x)            LOG_SERIAL.println(x)
#define debug_printf(x, ...)        LOG_SERIAL.printf((x), __VA_ARGS__)
#define debug_print(x)              LOG_SERIAL.print(x)

struct settings_t {
    byte connType;
    char host[32];
    int port;
    char httpUrl[48];
    char mqttTopic[32];
    char mqttName[32];
    char mqttPass[32];
    byte mqttSsl;
    uint8_t reqInt;
    int8_t minRssi;
    uint8_t dataFormat;
    uint8_t advFilter;
};

enum CONN_TYPE {
    CONN_TYPE_WS_SERVER = 0,
    CONN_TYPE_WS_CLIENT,
    CONN_TYPE_HTTP_CLIENT,
    CONN_TYPE_MQTT_CLIENT,
    CONN_TYPE_MAX
};

enum APP_STATE {
    APP_IDLE = 0,
    APP_UPLOADING,
    APP_SMARTCONFIG,
    APP_FACTORY_RESET,
    APP_SEEK_CONFIG,
    APP_STATE_MAX
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MAIN_H__ */
