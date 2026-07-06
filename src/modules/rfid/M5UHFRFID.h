/**
 * @file M5UHFRFID.h
 * @brief Read EPC Gen2 tags using the M5Stack UHF RFID Unit
 */

#ifndef __M5_UHF_RFID_H__
#define __M5_UHF_RFID_H__

#include "RFIDInterface.h"
#include <HardwareSerial.h>

#if !defined(M5_UHF_RFID_RX_PIN) || !defined(M5_UHF_RFID_TX_PIN)
#define M5_UHF_RFID_RX_PIN bruceConfigPins.uart_bus.rx
#define M5_UHF_RFID_TX_PIN bruceConfigPins.uart_bus.tx
#endif

#define M5_UHF_RFID_BAUD 115200

class M5UHFRFID : public RFIDInterface {
public:
    M5UHFRFID();
    ~M5UHFRFID();

    bool begin() override;

    int read(int cardBaudRate = 0) override;
    int clone() override;
    int erase() override;
    int write(int cardBaudRate = 0) override;
    int write_ndef() override;
    int load() override;
    int save(String filename) override;

private:
    struct InventoryTag {
        uint8_t rssi = 0;
        uint8_t pc[2] = {0};
        uint8_t crc[2] = {0};
        uint8_t epc[32] = {0};
        uint8_t epcLength = 0;
    };

    HardwareSerial *_stream = nullptr;
    bool _begun = false;
    InventoryTag _lastTag;

    void clearInput();
    void sendCommand(const uint8_t *command, size_t commandLength);
    bool readFrame(uint8_t *frame, size_t &frameLength, uint32_t timeoutMs);
    bool validateFrame(const uint8_t *frame, size_t frameLength) const;
    bool readHardwareVersion(String &version);
    bool parseInventoryFrame(const uint8_t *frame, size_t frameLength, InventoryTag &tag) const;
    void formatTag();
    void parseLoadedTag();
    String byteToHex(uint8_t value) const;
    String bytesToHex(const uint8_t *data, size_t dataLength, char separator = ' ') const;
    String rssiToString(uint8_t rssi) const;
};

#endif