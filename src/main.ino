#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#define DEBUG_ESP_PORT      Serial1

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiMulti.h>
#include <Hash.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#include <ESP8266mDNS.h>
#include <Ticker.h>

#include <MQTT.h>
#include <PubSubClient.h>

#include "main.h"

#define LOG_SERIAL  		Serial1
#define BLE_SERIAL  		Serial

#define TMP_BUFFER_SIZE     64


enum DATA_STATE {
    DATA_STATE_IDLE = 0,
    DATA_STATE_LEN,
    DATA_STATE_BUFF,
    DATA_STATE_END
};

const char *WIFI_SSID           = "YOUR-SSID";
const char *WIFI_PASS           = "YOUR-PASS";

const char *DEFAULT_AP_NAME     = "ab.gateway";
const char *DEFAULT_AP_PASS     = "12345678";

const uint8_t DEFAULT_PASS_LEN  = 8;
const uint8_t MAX_FRAME_LEN     = 42;

uint32_t mMaxBufferSize = 15000;

WiFiEventHandler onGotIp;
WiFiEventHandler onDisconnected;

byte mBuf[8000];
byte mFrameBuf[TMP_BUFFER_SIZE];

uint8_t mConnected              = 0;
uint8_t mAppState               = APP_IDLE;
uint8_t mPrevAppState           = APP_IDLE;
uint8_t mDataState              = DATA_STATE_IDLE;

int mFrameCnt                   = 0;
int mFrameWrongCnt              = 0;

unsigned long mCurrentMillis    = 0;
unsigned long mPrevMillis       = 0;
unsigned long mMessageId        = 1;

uint8_t mFrameLen               = 0;
int mTotal                      = 0;
char mClientId[32];

void setupBle() {
    debug_println("=== Start setup BLE ===");
    debug_println("AT+RSSI=-127");
    debug_println("AT+SCAN=1\r\n");
    BLE_SERIAL.printf("AT+RSSI=-127\r\n");
    BLE_SERIAL.println("AT+SCAN=1\r\n");
}

void setup() {
    BLE_SERIAL.begin(115200);
    LOG_SERIAL.begin(115200);
    //LOG_SERIAL.setDebugOutput(true);

    WiFi.begin(WIFI_SSID, WIFI_PASS);

    onDisconnected = WiFi.onStationModeDisconnected([&](const WiFiEventStationModeDisconnected& evt){
        mConnected = 0;
    });

    onGotIp = WiFi.onStationModeGotIP([&](const WiFiEventStationModeGotIP& evt){
        mAppState = APP_IDLE;
        mConnected = 1;

	debug_printf("WiFi connected - %s\r\n", WiFi.SSID().c_str());
	debug_print("IP address: ");
	debug_println(WiFi.localIP());

        mTotal = 0;
    });

    BLE_SERIAL.swap();
    BLE_SERIAL.setTimeout(500);

    setupBle();
}

void loop() {

    readUartData();
    mCurrentMillis = millis();

    uint8_t ret;
    int interval = 1000;
    if (canUpload()) {
        debug_printf("mId: %d Frame: %d Free heap: %d Bytes %d\r\n", 
            mMessageId, 
            mFrameCnt,
            ESP.getFreeHeap(), 
            mTotal
        );

        if (mTotal > 0) {
            ret = uploadData();
            mCurrentMillis = mPrevMillis = millis();
            mAppState = ret > 0 ? APP_UPLOADING : APP_IDLE;
        } 

        mFrameCnt = mFrameWrongCnt = 0;
        mTotal = 0;
    }

}
