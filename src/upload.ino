#include <Arduino.h>

#include <ESP8266WiFi.h>
#include "main.h"

#define MQTT_CONN_TIMEOUT   5

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

const char host[]               = "192.168.31.26";
const int port                  = 1883;
const char mqttTopic[]          = "beacons";

const int MAX_MQTT_PACKET_LEN   = 2500;

static int _sendData(byte *data, int len) {
    if (!mqtt.connected()) {
        mqttConnect();
    }

    if (!mqtt.loop()) {
        return 0;
    }

    MQTT::Publish pub(mqttTopic, data, len);
    if (!mqtt.publish(pub)) {
        return 0;
    }

    return 1;
}

void setupMqttClient() {
    mqtt.set_timeout(MQTT_CONN_TIMEOUT);
    mqtt.set_server(String(host), port);
}

bool mqttConnect() {
    debug_print("Attempting MQTT connection...");

    setupMqttClient();

    // Attempt to connect
    String id = "ab-";
    id += String(ESP.getChipId(), DEC);
    strcpy(mClientId, id.c_str());
    mqtt.connect(mClientId);

    if (mqtt.connected()) {
        debug_println("MQTT connected");
    } else {
        debug_println("MQTT connect fail");
        delay(100);
    }
}


uint8_t canUpload() {
    int heap = ESP.getFreeHeap();
    if (heap < 7500) {
        return 0;
    }

    // check line end
    if ((mTotal > 0) && (mBuf[mTotal - 2] != 0x0D) && (mBuf[mTotal - 1] != 0x0A)) {
        return 0;
    }

    int interval = 1000;
    if (mTotal + TMP_BUFFER_SIZE > mMaxBufferSize) {
        return 1;
    }

    return (mCurrentMillis - mPrevMillis >= interval);
}

int uploadData() {
    if (!mConnected) {
        return 0;
    }

    int offset = 0,
        frameLen = 0,
        maxLen = 2300,
        len = 0;

    uint8_t data[maxLen];
    uint8_t more, i = 0, ret = 0,
        uploadedCnt = 0,
        uploadRound = mTotal / maxLen + (mTotal % maxLen != 0);

    String mac = WiFi.macAddress();
    mac.replace(":", "");

    for (i = 0; i < uploadRound; i++) {
        more = (i == uploadRound - 1) ? 0 : 1;
		char meta[128];
		sprintf(meta, "{\"v\":\"%s\",\"mid\":%d,\"id\":\"%d\",\"mac\":\"%s\",\"rssi\":%d,\"time\":%d,\"ip\":\"%s\",\"more\":%d}\r\n\r\n", 
			APP_VERSION,
			mMessageId,
			ESP.getChipId(),
			mac.c_str(),
			WiFi.RSSI(),
			mCurrentMillis / 1000,
			WiFi.localIP().toString().c_str(),
			more
		);
		memcpy(data, meta, strlen(meta));
		len = strlen(meta);

		while (offset < mTotal) {
			if (mBuf[offset] == FRAME_START) {
				frameLen = mBuf[offset + OFFSET_LEN] + 3; // len frame_start + frame_end
				memcpy(data + len, mBuf + offset, frameLen);
				offset += frameLen;
				len += frameLen;
				if (len + TMP_BUFFER_SIZE > maxLen) {
					break;
				}
			} else {
				offset++;
			}
		}

        ret = _sendData(data, len);
        if (ret < 1) {
            return ret;
        }
        mMessageId++;
    }

    return 1;
}

