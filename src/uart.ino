#include <Arduino.h>

const uint8_t OFFSET_LEN            = 1;
const uint8_t OFFSET_MAC            = 3;
const uint8_t OFFSET_ADV            = 10;

const uint8_t FRAME_START           = 0xfe;
const uint8_t FRAME_END             = 0x0d;

void readUartData() {
    while (BLE_SERIAL.available() > 0)  {
        uint8_t c = BLE_SERIAL.read();
        switch (mDataState) {
            case DATA_STATE_BUFF:
            {
                mFrameBuf[mFrameLen++] = c;
                uint8_t frameLen = mFrameBuf[OFFSET_LEN] + 1;
                if (mFrameLen == frameLen) {
                    mDataState = DATA_STATE_END;
                }
            }
            break;

            case DATA_STATE_LEN:
            {
                // read data length
                if (c > TMP_BUFFER_SIZE) {
                    mDataState = DATA_STATE_IDLE;
                    continue;
                }

                mFrameBuf[mFrameLen++] = c;
                mDataState = DATA_STATE_BUFF;
            }
            break;

            case DATA_STATE_END:
            {
                // add frame to buffer
                if (c == FRAME_END) {
                    mFrameBuf[mFrameLen++] = c;
                    mFrameBuf[mFrameLen++] = 0x0a;
                    if (mTotal + mFrameLen < mMaxBufferSize) {
                        memcpy(mBuf + mTotal, mFrameBuf, mFrameLen);
                        mTotal+= mFrameLen;
                        mFrameCnt++;
                    } 
                } else {
                    mFrameWrongCnt++;
                }
                mDataState = DATA_STATE_IDLE;
            }
            break;

            case DATA_STATE_IDLE:
            default:
            {
                if (c != FRAME_START) {
                    continue;
                }
                mFrameLen = 0;
                mFrameBuf[mFrameLen++] = c;
                mDataState = DATA_STATE_LEN;
            }
        }
    }
}

