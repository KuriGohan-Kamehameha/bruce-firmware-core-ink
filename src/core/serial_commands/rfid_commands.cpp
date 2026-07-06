#include "rfid_commands.h"
#include "modules/rfid/M5UHFRFID.h"
#include <globals.h>

uint32_t rfidReadCallback(cmd *c) {
    Command cmd(c);
    String timeoutValue = cmd.getArgument("timeout").getValue();
    String rxValue = cmd.getArgument("rx").getValue();
    String txValue = cmd.getArgument("tx").getValue();
    timeoutValue.trim();
    rxValue.trim();
    txValue.trim();

    int timeoutSeconds = timeoutValue.toInt();
    if (timeoutSeconds <= 0) timeoutSeconds = 10;

    if (bruceConfigPins.rfidModule != M5_UHF_RFID_MODULE) {
        serialDevice->println("RFID module is not M5 UHF RFID. Run: settings rfidModule 6");
        return false;
    }

    bool overridePins = rxValue.length() > 0 || txValue.length() > 0;
    if (overridePins && (rxValue.length() == 0 || txValue.length() == 0)) {
        serialDevice->println("Both RX and TX pins are required when overriding UART pins");
        return false;
    }

    gpio_num_t savedRx = bruceConfigPins.uart_bus.rx;
    gpio_num_t savedTx = bruceConfigPins.uart_bus.tx;
    if (overridePins) {
        bruceConfigPins.uart_bus.rx = static_cast<gpio_num_t>(rxValue.toInt());
        bruceConfigPins.uart_bus.tx = static_cast<gpio_num_t>(txValue.toInt());
    }

    auto restorePins = [&]() {
        bruceConfigPins.uart_bus.rx = savedRx;
        bruceConfigPins.uart_bus.tx = savedTx;
    };

    M5UHFRFID rfid;
    serialDevice->println(
        "M5 UHF RFID UART RX=" + String(static_cast<int>(bruceConfigPins.uart_bus.rx)) +
        " TX=" + String(static_cast<int>(bruceConfigPins.uart_bus.tx)) + " baud=" + String(M5_UHF_RFID_BAUD)
    );

    if (!rfid.begin()) {
        serialDevice->println("UHF RFID module not found");
        restorePins();
        return false;
    }

    serialDevice->println("UHF RFID module ready");

    uint32_t startTime = millis();
    uint32_t timeoutMs = static_cast<uint32_t>(timeoutSeconds) * 1000UL;

    while (millis() - startTime < timeoutMs) {
        int readStatus = rfid.read();

        if (readStatus == RFIDInterface::SUCCESS) {
            serialDevice->println("UHF tag detected");
            serialDevice->println("Type: " + rfid.printableUID.picc_type);
            serialDevice->println("EPC: " + rfid.printableUID.uid);
            serialDevice->println("PC: " + rfid.printableUID.sak);
            serialDevice->println("RSSI: " + rfid.printableUID.atqa);
            serialDevice->println("CRC: " + rfid.printableUID.bcc);
            restorePins();
            return true;
        }

        if (readStatus == RFIDInterface::FAILURE) {
            serialDevice->println("UHF RFID read failed");
            restorePins();
            return false;
        }

        delay(100);
    }

    serialDevice->println("No UHF tag found");
    restorePins();
    return true;
}

void createRfidCommands(SimpleCLI *cli) {
    Command rfidCmd = cli->addCompositeCmd("rfid");
    Command readCmd = rfidCmd.addCommand("read", rfidReadCallback);
    readCmd.addPosArg("timeout", "10");
    readCmd.addPosArg("rx", "");
    readCmd.addPosArg("tx", "");
}