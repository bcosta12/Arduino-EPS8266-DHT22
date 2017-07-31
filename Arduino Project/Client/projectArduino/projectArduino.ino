#include "ESP8266.h" //wifi esp8266 lib
#include <DHT.h> //dht lib

//------------------ wifi config ------------------

#define SSID        "ssid" //your wifi network name
#define PASSWORD    "password" //yout password
#define HOST_NAME   "10.0.0.102" //ip of the server, must be the same of the python udp server (or other that you prefer)
#define HOST_PORT   5005 //port of the server, must be the same of the python udp servr (or other that you prefer)

//------------------ dht22 config ------------------

#define DHTPIN 2  // what pin are connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define fan 4


//------------------ serial wifi mod to pin 6 7 ------------------

SoftwareSerial mySerial(6, 7); /* RX:D3, TX:D2 */
ESP8266 wifi(mySerial);

//------------------ sensors globals ------------------

int maxHum = 60;
int maxTemp = 40;

DHT dht(DHTPIN, DHTTYPE);

void setup(void){
  
    Serial.begin(9600);                               //START setup, baudrate = 9600bps
    Serial.print("setup begin\r\n");

    
    pinMode(fan, OUTPUT);
    dht.begin();
    
    Serial.print("FW Version:");
    Serial.println(wifi.getVersion().c_str());
      
    if (wifi.setOprToStationSoftAP()) {
        Serial.print("to station + softap ok\r\n");
    } else {
        Serial.print("to station + softap err\r\n");
    }
 
    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.print("Join AP success\r\n");
        Serial.print("IP: ");
        Serial.println(wifi.getLocalIP().c_str());       
    } else {
        Serial.print("Join AP failure\r\n");
    }
    
    if (wifi.enableMUX()) {
        Serial.print("multiple ok\r\n");
    } else {
        Serial.print("multiple err\r\n");
    }
    
    Serial.print("setup end\r\n");
}
 
void loop(void){
    
    // Wait a few seconds between measurements.
    delay(2000);
    uint8_t buffer[128] = {0};
    static uint8_t mux_id = 0;

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
  
    if(h > maxHum || t > maxTemp) {
      digitalWrite(fan, HIGH);
    }
    else {
      digitalWrite(fan, LOW); 
    }
  
    Serial.print("Humidity: "); 
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: "); 
    Serial.print(t);
    Serial.println(" *C ");
  
    if (wifi.registerUDP(mux_id, HOST_NAME, HOST_PORT)) {
        Serial.print("register udp ");
        Serial.print(mux_id);
        Serial.println(" ok");
    }
    else {
        Serial.print("register udp ");
        Serial.print(mux_id);
        Serial.println(" err");
    }

    //prepare string to send
    String temp = String(t);
    String humi = String(h);
    String data =  temp+ "," + humi;
    Serial.print(data);
    int str_len = data.length() + 1;
    char sendData[str_len];
    data.toCharArray(sendData, str_len);
    Serial.print(sendData);
    
    wifi.send(mux_id, (const uint8_t*)sendData, strlen(sendData));
    
    
    uint32_t len = wifi.recv(mux_id, buffer, sizeof(buffer), 10000);
    if (len > 0) {
        Serial.print("Received:[");
        for(uint32_t i = 0; i < len; i++) {
            Serial.print((char)buffer[i]);
        }
        Serial.print("]\r\n");
    }
    
    if (wifi.unregisterUDP(mux_id)) {
        Serial.print("unregister udp ");
        Serial.print(mux_id);
        Serial.println(" ok");
    }
    else {
        Serial.print("unregister udp ");
        Serial.print(mux_id);
        Serial.println(" err");
    }
    delay(5000);
    mux_id++;
    if (mux_id >= 5) {
        mux_id = 0;
    }
}
