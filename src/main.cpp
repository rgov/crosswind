#include <Arduino.h>

// BLE library documentation is here:
// https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ble.html
// https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>

#include <FastLED.h>


#define HEADWIND_BLE_ADDRESS "c9:a5:2c:c7:ba:b7" // lowercase
#define SERVICE_UUID "A026EE0C-0A7D-4AB3-97FA-F1500F9FEB8B"
#define CHARACTERISTIC_UUID "A026E038-0A7D-4AB3-97FA-F1500F9FEB8B"


static BLEAddress *pServerAddress;
static BLEClient *pClient = nullptr;
static BLERemoteCharacteristic *pRemoteCharacteristic = nullptr;
static bool doConnect = false;
static bool connected = false;
static bool didManualSwitch = false;


class MyClientCallbacks : public BLEClientCallbacks
{
    void onConnect(BLEClient *client)
    {
        // This can be used for actions when the device is connected
    }

    void onDisconnect(BLEClient *client)
    {
        Serial.println("Disconnected from the server");
        connected = false;
        // Here you can add code to handle reconnection if needed
    }
};


class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice) override
    {
        Serial.print("Found a device: ");
        Serial.println(advertisedDevice.getAddress().toString().c_str());

        if (advertisedDevice.getAddress().toString() == HEADWIND_BLE_ADDRESS)
        {
            advertisedDevice.getScan()->stop();
            pServerAddress = new BLEAddress(advertisedDevice.getAddress());
            doConnect = true;
        }
    }
};


void connectToServer()
{
    Serial.print("Forming a connection to ");
    Serial.println(pServerAddress->toString().c_str());

    pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallbacks());
    pClient->connect(*pServerAddress, BLE_ADDR_TYPE_RANDOM); // can also just pass the advertisedDevice here

    BLERemoteService *pRemoteService = pClient->getService(SERVICE_UUID);
    if (pRemoteService == nullptr)
    {
        Serial.print("Failed to find our service UUID: ");
        Serial.println(SERVICE_UUID);
        return;
    }

    pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
    if (pRemoteCharacteristic == nullptr)
    {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(CHARACTERISTIC_UUID);
        return;
    }

    if (pRemoteCharacteristic->canNotify())
        pRemoteCharacteristic->registerForNotify([](BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
                                                 {
        Serial.print("Received data: ");
        for (int i = 0; i < length; i++) {
          Serial.print(pData[i]);
          Serial.print(" ");
        }
        Serial.println(); });

    connected = true;
}


void setup()
{
#define LED_GPIO 27
    CRGB leds[1];
    FastLED.addLeds<WS2812, LED_GPIO, GRB>(leds, 1);
    leds[0] = CRGB::Red;
    FastLED.show();

    Serial.begin(115200);
    delay(10000);

    Serial.println("Scanning for BLE devices...");

    BLEDevice::init("");
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->start(30);
}


void loop()
{
    if (doConnect)
    {
        connectToServer();
        doConnect = false;
    }

    if (connected && !didManualSwitch)
    {
        Serial.println("Connected, switching to manual mode");
        std::vector<uint8_t> data = {0x04, 0x04};
        pRemoteCharacteristic->writeValue(data.data(), data.size(), false);
        didManualSwitch = true;
    }
    else if (connected && didManualSwitch)
    {
        float speed = 20 * (sin((2 * M_PI * millis()) / 30000) + 1);
        Serial.print("Speed is: ");
        Serial.println(speed);
        std::vector<uint8_t> speedData = {0x02, static_cast<uint8_t>(speed)};
        pRemoteCharacteristic->writeValue(speedData.data(), speedData.size(), false);
    }

    Serial.println("busy");
    delay(1000);
}
