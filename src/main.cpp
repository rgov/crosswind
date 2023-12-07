#include <Arduino.h>
#include <ArduinoBLE.h>
#include <FastLED.h>


#define HEADWIND_BLE_ADDRESS "c9:a5:2c:c7:ba:b7" // lowercase
#define SERVICE_UUID "A026EE0C-0A7D-4AB3-97FA-F1500F9FEB8B"
#define CHARACTERISTIC_UUID "A026E038-0A7D-4AB3-97FA-F1500F9FEB8B"


static BLEDevice peripheral;
static BLECharacteristic pRemoteCharacteristic;
static bool doConnect = false;
static bool connected = false;
static bool didManualSwitch = false;


void device_discovered(BLEDevice advertisedDevice)
{
    Serial.print("Found a device: ");
    Serial.println(advertisedDevice.address());

    if (advertisedDevice.address() == HEADWIND_BLE_ADDRESS)
    {
        BLE.stopScan();
        peripheral = advertisedDevice;
        doConnect = true;
    }
}


void connectToServer()
{
    Serial.print("Forming a connection to ");
    Serial.println(peripheral.address());

    if (!peripheral.connect()) {
        Serial.println("Failed to connect!");
        return;
    }

    // Discover peripheral attributes
    Serial.println("Discovering attributes ...");
    if (peripheral.discoverAttributes()) {
        Serial.println("Attributes discovered");
    } else {
        Serial.println("Attribute discovery failed");
        peripheral.disconnect();
        return;
    }

    pRemoteCharacteristic = peripheral.characteristic(CHARACTERISTIC_UUID);
    if (!pRemoteCharacteristic)
    {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(CHARACTERISTIC_UUID);
        peripheral.disconnect();
        return;
    }

    if (pRemoteCharacteristic.canSubscribe()) {
        pRemoteCharacteristic.subscribe();

        pRemoteCharacteristic.setEventHandler(BLEUpdated, [](BLEDevice central, BLECharacteristic characteristic) {
            Serial.print("Received data: ");
            for (int i = 0; i < characteristic.valueLength(); i++) {
                Serial.print(characteristic.value()[i]);
                Serial.print(" ");
            }
            Serial.println();
        });
    } else {
        Serial.println("Failed to subscribe to characteristic");
        peripheral.disconnect();
        return;
    }

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

    if (!BLE.begin())
    {
        Serial.println("Starting BLE failed!");
        while (1) {}
    }

    Serial.println("Scanning for BLE devices...");
    BLE.setEventHandler(BLEDiscovered, device_discovered);
    BLE.scan();
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
        uint8_t data[] = {0x02, 0x04};
        pRemoteCharacteristic.writeValue(data, sizeof(data));
        didManualSwitch = true;
    }
    else if (connected && didManualSwitch)
    {
        float speed = 20 * (sin((2 * M_PI * millis()) / 30000) + 1);
        Serial.print("Speed is: ");
        Serial.println(speed);
        uint8_t speedData[] = {0x02, static_cast<uint8_t>(speed)};
        pRemoteCharacteristic.writeValue(speedData, sizeof(speedData));
    }

    Serial.println("busy");
    BLE.poll(1000);
}
