#include <BLEDevice.h>

// UUID of the service we want from iTag
static const BLEUUID serviceUUID("0000ffe0-0000-1000-8000-00805f9b34fb");
// UUID of the service characteristic (iTag button)
static const BLEUUID characteristicUUID("0000ffe1-0000-1000-8000-00805f9b34fb");

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify address ");
    Serial.println(pBLERemoteCharacteristic->getRemoteService()->getClient()->getPeerAddress().toString().c_str());
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    char Byte_In_Hex[length];
    sprintf(Byte_In_Hex,"%x", pData);
    Serial.print(Byte_In_Hex);
    //Serial.write(pData, length);
    Serial.println();
}

// Class responsible for controlling each iTag
class ITag {
  public: int pinNumber; // pin number linked to this iTag
  int pinStatus = HIGH; // pin status (HIGH or LOW)
  BLEClient * client; // iTag connection client
  std::string address; // address of iTag to which you will connect

  ITag(std::string addrs, int pNum) {
    // We link the values ​​of the address, pin number, put it as an output
    address = addrs;
    pinNumber = pNum;
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, pinStatus);
  }

  void connect() {
    // Check if there was a customer before and disconnect if so
    if (client != NULL) {
      client -> disconnect();
      delete client;
    }

    // We create the client with a new id and connect to iTag
    client = BLEDevice::createClient();
    BLEAddress bleAddress(address);
    boolean connected = client -> connect(bleAddress);

    // If the connection was successful
    if (connected) {
      // We obtain the service and feature of the iTag button and link the onData function
      // to answer the button press
      BLERemoteService * remoteService = client -> getService(serviceUUID);
      BLERemoteCharacteristic * remoteCharacteristic = remoteService -> getCharacteristic(characteristicUUID);
      remoteCharacteristic -> registerForNotify(notifyCallback);
    }
  }
};

// Interval between each scan
#define SCAN_INTERVAL 3000
// Quantity of iTag we have (change according to the amount of iTags you have)
#define ITAG_COUNT 2

// Definition of the iTags that we will use
// For each iTag we link his address (obtained through a scan)
// and the pin whose status will be changed when the iTag button is pressed
ITag iTags[ITAG_COUNT] = {
  ITag("ff:ff:10:5f:bc:55", 25),
  ITag("ff:22:04:13:18:ec", 26)
};

// Variable that will save the scan
BLEScan * pBLEScan;

// When the last scan occurred
uint32_t lastScanTime = 0;

void setup() {
  Serial.begin(115200);

  // We started BLE
  BLEDevice::init("");

  // We keep the object responsible for the scan
  pBLEScan = BLEDevice::getScan();
  pBLEScan -> setActiveScan(true);
}

void loop() {
  // Time in milliseconds since boot
  uint32_t now = millis();

  // If it's time to scan
  if (now - lastScanTime > SCAN_INTERVAL) {
    // Marks when the last scan occurred and starts the scan
    lastScanTime = now;
    scan();
  }
}

void scan() {
  // Performs the scan for 2 seconds
  BLEScanResults results = pBLEScan -> start(2);
  pBLEScan -> stop();

  // For each device found by the scan
  for (int i = 0; i < results.getCount(); i++) {
    // We keep the reference for the device and show it on the serial monitor
    BLEAdvertisedDevice advertisedDevice = results.getDevice(i);
    Serial.println("advertisedDevice:" + String(advertisedDevice.toString().c_str()));

    // For every iTag we have
    for (int j = 0; j < ITAG_COUNT; j++) {
      // If the scanned device is one of our iTags
      if (advertisedDevice.getAddress().toString() == iTags[j].address) {
        // We order to connect
        iTags[j].connect();
      }
    }
  }
}