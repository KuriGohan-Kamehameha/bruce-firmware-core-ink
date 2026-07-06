/**
 * @file M5UHFRFID.cpp
 * @brief Read EPC Gen2 tags using the M5Stack UHF RFID Unit
 */

#include "M5UHFRFID.h"
#include "core/sd_functions.h"

namespace {
constexpr uint8_t UHF_FRAME_START = 0xBB;
constexpr uint8_t UHF_FRAME_END = 0x7E;
constexpr uint8_t UHF_CMD_HARDWARE_VERSION = 0x03;
constexpr uint8_t UHF_CMD_POLLING_ONCE = 0x22;
constexpr uint8_t UHF_CMD_ERROR = 0xFF;
constexpr size_t UHF_MAX_FRAME_SIZE = 128;
constexpr uint32_t UHF_READ_TIMEOUT_MS = 650;

const uint8_t HARDWARE_VERSION_CMD[] = {0xBB, 0x00, 0x03, 0x00, 0x01, 0x00, 0x04, 0x7E};
const uint8_t POLLING_ONCE_CMD[] = {0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E};
} // namespace

M5UHFRFID::M5UHFRFID() {}

M5UHFRFID::~M5UHFRFID() {
    if (_stream) {
        _stream->end();
        delete _stream;
    }
}

bool M5UHFRFID::begin() {
    if (M5_UHF_RFID_RX_PIN == GPIO_NUM_NC || M5_UHF_RFID_TX_PIN == GPIO_NUM_NC) return false;

    if (!_stream) _stream = new HardwareSerial(1);
    _stream->begin(M5_UHF_RFID_BAUD, SERIAL_8N1, M5_UHF_RFID_RX_PIN, M5_UHF_RFID_TX_PIN);
    delay(50);
    clearInput();

    String version;
    _begun = readHardwareVersion(version);
    printableUID.picc_type = "UHF RFID";
    return _begun;
}

int M5UHFRFID::read(int cardBaudRate) {
    (void)cardBaudRate;

    pageReadStatus = TAG_NOT_PRESENT;
    pageReadSuccess = false;

    if (!_begun && !begin()) return FAILURE;

    clearInput();
    sendCommand(POLLING_ONCE_CMD, sizeof(POLLING_ONCE_CMD));

    uint8_t frame[UHF_MAX_FRAME_SIZE];
    size_t frameLength = 0;
    uint32_t startTime = millis();

    while (millis() - startTime < UHF_READ_TIMEOUT_MS) {
        if (!readFrame(frame, frameLength, 120)) continue;

        if (parseInventoryFrame(frame, frameLength, _lastTag)) {
            formatTag();
            pageReadStatus = SUCCESS;
            pageReadSuccess = true;
            return SUCCESS;
        }

        if (frameLength > 2 && frame[2] == UHF_CMD_ERROR) return TAG_NOT_PRESENT;
    }

    return TAG_NOT_PRESENT;
}

int M5UHFRFID::clone() { return NOT_IMPLEMENTED; }

int M5UHFRFID::erase() { return NOT_IMPLEMENTED; }

int M5UHFRFID::write(int cardBaudRate) {
    (void)cardBaudRate;
    return NOT_IMPLEMENTED;
}

int M5UHFRFID::write_ndef() { return NOT_IMPLEMENTED; }

int M5UHFRFID::load() {
    FS *fs;
    if (!getFsStorage(fs)) return FAILURE;

    String filepath = loopSD(*fs, true, "RFID|NFC", "/BruceRFID");
    File file = fs->open(filepath, FILE_READ);
    if (!file) return FAILURE;

    strAllPages = "";
    printableUID = PrintableUID();

    while (file.available()) {
        String line = file.readStringUntil('\n');
        String value = line.substring(line.indexOf(":") + 1);
        value.trim();

        if (line.startsWith("Device type:")) printableUID.picc_type = value;
        if (line.startsWith("UID:")) printableUID.uid = value;
        if (line.startsWith("EPC:")) printableUID.uid = value;
        if (line.startsWith("PC:")) printableUID.sak = value;
        if (line.startsWith("RSSI:")) printableUID.atqa = value;
        if (line.startsWith("CRC:")) printableUID.bcc = value;
        if (line.startsWith("Pages total:")) dataPages = value.toInt();
    }

    file.close();
    if (printableUID.uid.isEmpty()) return FAILURE;

    parseLoadedTag();
    pageReadSuccess = true;
    pageReadStatus = SUCCESS;
    return SUCCESS;
}

int M5UHFRFID::save(String filename) {
    FS *fs;
    if (!getFsStorage(fs)) return FAILURE;

    File file = createNewFile(fs, "/BruceRFID", filename + ".rfid");
    if (!file) return FAILURE;

    file.println("Filetype: Bruce RFID File");
    file.println("Version 1");
    file.println("Device type: " + printableUID.picc_type);
    file.println("# EPCglobal Class 1 Gen 2 / ISO18000-6C inventory data");
    file.println("UID: " + printableUID.uid);
    file.println("EPC: " + printableUID.uid);
    file.println("PC: " + printableUID.sak);
    file.println("RSSI: " + printableUID.atqa);
    file.println("CRC: " + printableUID.bcc);
    file.println("# Inventory dump");
    file.println("Pages total: " + String(dataPages));
    file.print(strAllPages);

    file.close();
    delay(100);
    return SUCCESS;
}

void M5UHFRFID::clearInput() {
    if (!_stream) return;
    while (_stream->available()) _stream->read();
}

void M5UHFRFID::sendCommand(const uint8_t *command, size_t commandLength) {
    if (!_stream) return;
    _stream->write(command, commandLength);
    _stream->flush();
}

bool M5UHFRFID::readFrame(uint8_t *frame, size_t &frameLength, uint32_t timeoutMs) {
    if (!_stream) return false;

    bool receivingFrame = false;
    frameLength = 0;
    uint32_t startTime = millis();

    while (millis() - startTime < timeoutMs) {
        while (_stream->available()) {
            uint8_t byteValue = _stream->read();

            if (!receivingFrame) {
                if (byteValue != UHF_FRAME_START) continue;
                receivingFrame = true;
                frameLength = 0;
            }

            if (frameLength >= UHF_MAX_FRAME_SIZE) {
                receivingFrame = false;
                frameLength = 0;
                continue;
            }

            frame[frameLength++] = byteValue;
            if (byteValue == UHF_FRAME_END && validateFrame(frame, frameLength)) return true;
            if (byteValue == UHF_FRAME_END) {
                receivingFrame = false;
                frameLength = 0;
            }
        }

        delay(2);
    }

    return false;
}

bool M5UHFRFID::validateFrame(const uint8_t *frame, size_t frameLength) const {
    if (frameLength < 7) return false;
    if (frame[0] != UHF_FRAME_START || frame[frameLength - 1] != UHF_FRAME_END) return false;

    uint16_t payloadLength = (static_cast<uint16_t>(frame[3]) << 8) | frame[4];
    if (frameLength != static_cast<size_t>(payloadLength) + 7) return false;

    uint8_t checksum = 0;
    for (size_t checksumIndex = 1; checksumIndex < frameLength - 2; checksumIndex++) {
        checksum += frame[checksumIndex];
    }

    return checksum == frame[frameLength - 2];
}

bool M5UHFRFID::readHardwareVersion(String &version) {
    sendCommand(HARDWARE_VERSION_CMD, sizeof(HARDWARE_VERSION_CMD));

    uint8_t frame[UHF_MAX_FRAME_SIZE];
    size_t frameLength = 0;
    if (!readFrame(frame, frameLength, 500)) return false;
    if (frameLength < 7 || frame[2] != UHF_CMD_HARDWARE_VERSION) return false;

    uint16_t payloadLength = (static_cast<uint16_t>(frame[3]) << 8) | frame[4];
    version = "";
    for (size_t payloadIndex = 0; payloadIndex < payloadLength; payloadIndex++) {
        char charValue = static_cast<char>(frame[5 + payloadIndex]);
        if (charValue >= 32 && charValue <= 126) version += charValue;
    }

    return true;
}

bool M5UHFRFID::parseInventoryFrame(const uint8_t *frame, size_t frameLength, InventoryTag &tag) const {
    if (frameLength < 12 || frame[2] != UHF_CMD_POLLING_ONCE) return false;

    uint16_t payloadLength = (static_cast<uint16_t>(frame[3]) << 8) | frame[4];
    if (payloadLength < 6) return false;

    uint8_t epcLength = payloadLength - 5;
    if (epcLength == 0 || epcLength > sizeof(tag.epc)) return false;

    tag.rssi = frame[5];
    tag.pc[0] = frame[6];
    tag.pc[1] = frame[7];
    tag.epcLength = epcLength;

    for (size_t epcIndex = 0; epcIndex < epcLength; epcIndex++) {
        tag.epc[epcIndex] = frame[8 + epcIndex];
    }

    tag.crc[0] = frame[8 + epcLength];
    tag.crc[1] = frame[9 + epcLength];
    return true;
}

void M5UHFRFID::formatTag() {
    printableUID.picc_type = "UHF RFID";
    printableUID.uid = bytesToHex(_lastTag.epc, _lastTag.epcLength);
    printableUID.sak = bytesToHex(_lastTag.pc, sizeof(_lastTag.pc));
    printableUID.bcc = bytesToHex(_lastTag.crc, sizeof(_lastTag.crc));
    printableUID.atqa = rssiToString(_lastTag.rssi);

    uid.size = _lastTag.epcLength > sizeof(uid.uidByte) ? sizeof(uid.uidByte) : _lastTag.epcLength;
    for (size_t uidIndex = 0; uidIndex < uid.size; uidIndex++) { uid.uidByte[uidIndex] = _lastTag.epc[uidIndex]; }
    uid.sak = _lastTag.pc[1];
    uid.atqaByte[0] = _lastTag.pc[0];
    uid.atqaByte[1] = _lastTag.pc[1];

    strAllPages = "EPC: " + printableUID.uid + "\n";
    strAllPages += "PC: " + printableUID.sak + "\n";
    strAllPages += "RSSI: " + printableUID.atqa + "\n";
    strAllPages += "CRC: " + printableUID.bcc + "\n";
    totalPages = 4;
    dataPages = 4;
}

void M5UHFRFID::parseLoadedTag() {
    printableUID.picc_type = printableUID.picc_type.isEmpty() ? "UHF RFID" : printableUID.picc_type;

    String compactEpc = printableUID.uid;
    compactEpc.replace(" ", "");
    _lastTag.epcLength = compactEpc.length() / 2;
    if (_lastTag.epcLength > sizeof(_lastTag.epc)) _lastTag.epcLength = sizeof(_lastTag.epc);

    for (size_t epcIndex = 0; epcIndex < _lastTag.epcLength; epcIndex++) {
        _lastTag.epc[epcIndex] = strtoul(compactEpc.substring(epcIndex * 2, epcIndex * 2 + 2).c_str(), NULL, 16);
    }

    uid.size = _lastTag.epcLength > sizeof(uid.uidByte) ? sizeof(uid.uidByte) : _lastTag.epcLength;
    for (size_t uidIndex = 0; uidIndex < uid.size; uidIndex++) { uid.uidByte[uidIndex] = _lastTag.epc[uidIndex]; }

    strAllPages = "EPC: " + printableUID.uid + "\n";
    if (!printableUID.sak.isEmpty()) strAllPages += "PC: " + printableUID.sak + "\n";
    if (!printableUID.atqa.isEmpty()) strAllPages += "RSSI: " + printableUID.atqa + "\n";
    if (!printableUID.bcc.isEmpty()) strAllPages += "CRC: " + printableUID.bcc + "\n";
    totalPages = 4;
    dataPages = 4;
}

String M5UHFRFID::byteToHex(uint8_t value) const {
    String result = value < 0x10 ? "0" : "";
    result += String(value, HEX);
    result.toUpperCase();
    return result;
}

String M5UHFRFID::bytesToHex(const uint8_t *data, size_t dataLength, char separator) const {
    String result = "";
    for (size_t byteIndex = 0; byteIndex < dataLength; byteIndex++) {
        if (byteIndex > 0 && separator) result += separator;
        result += byteToHex(data[byteIndex]);
    }
    return result;
}

String M5UHFRFID::rssiToString(uint8_t rssi) const {
    String result = String(static_cast<int>(static_cast<int8_t>(rssi))) + " dBm";
    result += " (0x" + byteToHex(rssi) + ")";
    return result;
}