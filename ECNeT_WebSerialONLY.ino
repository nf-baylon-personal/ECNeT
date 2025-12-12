// Version 0.9.5 WBSL+Diag (Build 1) ALPHA
// NO SERIAL INPUT HERE. ALL INPUTS MUST COME FROM WEBSERIAL INTERFACE
// Compiled 09/22/2024 | NF Baylon/Group 1 EICT-A

#define HELTEC_POWER_BUTTON

#include <heltec_unofficial.h>

#include <WiFi.h>

#include <AsyncTCP.h>

#include <ESPAsyncWebServer.h>

#include <WebSerialLite.h> // Webserial Lite Interface


// LoRa PHY Set-Up

#define PAUSE               600

#define FREQUENCY           866.3       // for Europe

#define BANDWIDTH           125.0

#define SPREADING_FACTOR    12

#define TRANSMIT_POWER      20          



String message;
String reply;
String ID = "[N1]"; // Node ID
String SOS = "SOS ALERT [Node 1]"; // SOS Message
String RandString;
volatile bool rxFlag = false;


AsyncWebServer server(80);

const char* ssid = "ECNeT_Node1";
const char* password = "gwapocnorbert";


// WebSerial Server Callback
// Receives inputs from WebSerial Interface
void recvMsg(uint8_t *data, size_t len){
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  String message = ID + d; // Webserial input + Node ID STR
  WebSerial.printf("[NX]%s\n", d.c_str());
  both.printf("[NX]%s\n", d.c_str());
  radio.clearDio1Action();
  RADIOLIB(radio.transmit(String(message).c_str())); // Transmits STR through LoRa
  radio.setDio1Action(rx);
  RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
}



void setup() {
  heltec_setup();
  Serial.begin(115200);
  setCpuFrequencyMhz(80); // Sets CPU Freq (limits clock due to heat issues)
  both.println("Initializing ECNeT");
  // LoRa Initialize
  radio.setDio1Action(rx);
  RADIOLIB_OR_HALT(radio.begin());
  both.printf("Frequency: %.2f MHz\n", FREQUENCY);
  RADIOLIB_OR_HALT(radio.setFrequency(FREQUENCY));
  RADIOLIB_OR_HALT(radio.setBandwidth(BANDWIDTH));
  RADIOLIB_OR_HALT(radio.setSpreadingFactor(SPREADING_FACTOR));
  RADIOLIB_OR_HALT(radio.setOutputPower(TRANSMIT_POWER));
  // Start receiving Lora
  RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
  both.printf("Battery: %d \n", heltec_battery_percent());
  both.println("ECNeT Ready");
  // Wifi
  WiFi.softAP(ssid, password);
  both.println("Node 1 Online");
  both.print("IP Address: ");
  both.println(WiFi.softAPIP());
  // Webserial Interface 
  WebSerial.begin(&server);
  WebSerial.onMessage(recvMsg);
  server.begin();
}


void loop() {
  heltec_loop();


  // Manual Check status 
  if (button.isSingleClick()) {
    both.printf("Battery: %d \n", heltec_battery_percent());
    both.printf("SNR: %.2f dB\n", radio.getSNR());
    both.printf("RSSI: %.2f dB\n", radio.getRSSI());
  }


  // Manual Testing
  if (button.isDoubleClick()) {
    int rxpacket = 0;
    while (rxpacket < 100) {
      byte randomValue = random(0, 37);
      char letter = randomValue + 'a';
      if (randomValue > 26){
        letter = (randomValue - 26) + '0';
        radio.clearDio1Action();
        RADIOLIB(radio.transmit(String(letter).c_str()));
        radio.setDio1Action(rx);
        rxpacket++;
        both.printf("Packet sent \n");
        WebSerial.printf("Packet Sent \n");
        both.printf("SNR: %.2f dB\n", radio.getSNR());
        RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
      }
    }
  }


  //Receive data from LoRa
  if (rxFlag) {
    rxFlag = false;
    radio.readData(reply);
    both.printf("SNR: %.2f dB\n", radio.getSNR());
    both.printf("RSSI: %.2f dB\n", radio.getRSSI());
    if (_radiolib_status == RADIOLIB_ERR_NONE) {
      both.printf("%s\n", reply.c_str());
      WebSerial.printf("%s\n", reply.c_str());
      //If an SOS signal is detected
      if (reply == "SOS ALERT [Node 2]") {
        both.printf("SNR: %.2f dB\n", radio.getSNR());
        both.printf("RSSI: %.2f dB\n", radio.getRSSI());
      }
    }
    RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
  }
}


//Callback Function for Receive
//note that I still don't fucking understand how this works
//some boolean flag argument that (somehow) prevents looping in the code.
void rx() {
  rxFlag = true;
}

