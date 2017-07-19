#include "ESP8266.h" //wifi esp8266 lib
#include <DHT.h> //dht lib

//------------------ wifi config ------------------

#define SSID        "wozniak" //your wifi network name
#define PASSWORD    "2orlando5123" //yout password
#define HOST_NAME   "10.0.0.102" //ip of the server, must be the same of the python udp server (or other that you prefer)
#define HOST_PORT   (5005) //port of the server, must be the same of the python udp servr (or other that you prefer)

//------------------ dht22 config ------------------

#define DHTPIN 2  // what pin are connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define fan 4
//------------------ MQ6 config ------------------

// We have to calibrate this sensor, so you have to follow this:
/*========Hardware Related Macros=====================*/
#define         MQ_PIN                       (0)     //define which analog input channel you are going to use
#define         RL_VALUE                     (20)    //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR          (10)    //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
                                                     //which is derived from the chart in datasheet
 
/*============Software Related Macros ===============*/
#define         CALIBARAION_SAMPLE_TIMES     (50)    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (500)   //define the time interal(in milisecond) between each samples in the
                                                     //cablibration phase
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interal(in milisecond) between each samples in 
                                                     //normal operation
 
/*=============Application Related Macros============*/
#define         GAS_LPG                      (0)
#define         GAS_CH4                      (1)
 
/*===============Globals=============================*/
float           LPGCurve[3]  =  {3,   0,  -0.4};    //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent"
                                                    //to the original curve. 
                                                    //data format:{ x, y, slope}; point1: (lg1000, lg1), point2: (lg10000, lg0.4) 
float           CH4Curve[3]  =  {3.3, 0,  -0.38};   //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent" 
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg2000, lg1), point2: (lg5000,  lg0.7) 
float           Ro           =  10;                 //Ro is initialized to 10 kilo ohms
float glp = 0;
float but = 0;

//------------------ serial wifi mod to pin 6 7 ------------------

SoftwareSerial mySerial(6, 7); /* RX:D3, TX:D2 */
ESP8266 wifi(mySerial);

//------------------ sensors globals ------------------

const int Buzzer = 13;
const int led = 12;


int maxHum = 60;
int maxTemp = 40;

DHT dht(DHTPIN, DHTTYPE);

void setup(void){
  
    Serial.begin(9600);                               //START setup, baudrate = 9600bps
    Serial.print("setup begin\r\n");

    pinMode(Buzzer, OUTPUT);
    Serial.print("Calibrating...\n");                
    Ro = MQCalibration(MQ_PIN);                      //Calibrating the sensor. Please make sure the sensor is in clean air 
                                                    //when you perform the calibration                    
    Serial.print("Calibration is done...\n"); 
    Serial.print("Ro=");
    Serial.print(Ro);
    Serial.print("kohm");
    Serial.print("\n");
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
    
    Serial.print("LPG:"); 
    Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG) );
    glp = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG) ;
    Serial.print( "ppm" );
    Serial.print("        ");   
    Serial.print("CH4::"); 
    but = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CH4);
    Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CH4) );
    Serial.print( "ppm" );
    Serial.print("\n");
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
  } else {
     digitalWrite(fan, LOW); 
  }
  
  Serial.print("Umidade: "); 
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperatura: "); 
  Serial.print(t);
  Serial.println(" *C ");
  
  if((glp > 0) ||(but > 0)){
      digitalWrite(led, HIGH);
      digitalWrite(Buzzer, HIGH);}
   else{digitalWrite(Buzzer, LOW); digitalWrite(led, LOW);
   }
  
    if (wifi.registerUDP(mux_id, HOST_NAME, HOST_PORT)) {
        Serial.print("register udp ");
        Serial.print(mux_id);
        Serial.println(" ok");
    } else {
        Serial.print("register udp ");
        Serial.print(mux_id);
        Serial.println(" err");
    }
    
    char *te = "";
    char *hu = "";
    dtostrf(t, 4, 2, te);
    dtostrf(h, 4, 2, hu);
    wifi.send(mux_id, (const uint8_t*)te, strlen(te));
    //delay(2000);
    //wifi.send(',', 1);
    //delay(2000);
    // wifi.send(mux_id, (const uint8_t*)hu, strlen(hu));
    //delay(2000);
    
    
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
    } else {
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

/*=============== MQResistanceCalculation ==================
Input:   raw_adc - raw value read from adc, which represents the voltage
Output:  the calculated sensor resistance
Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
         across the load resistor and its resistance, the resistance of the sensor
         could be derived.
============================================================*/ 
float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
}
 
/*===================== MQCalibration ======================
Input:   mq_pin - analog channel
Output:  Ro of the sensor
Remarks: This function assumes that the sensor is in clean air. It use  
         MQResistanceCalculation to calculates the sensor resistance in clean air 
         and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about 
         10, which differs slightly between different sensors.
============================================================*/ 
float MQCalibration(int mq_pin)
{
  int i;
  float val=0;
 
  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {            //take multiple samples
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBARAION_SAMPLE_TIMES;                   //calculate the average value
 
  val = val/RO_CLEAN_AIR_FACTOR;                        //divided by RO_CLEAN_AIR_FACTOR yields the Ro 
                                                        //according to the chart in the datasheet 
 
  return val; 
}
/*==================== MQRead ===============================
Input:   mq_pin - analog channel
Output:  Rs of the sensor
Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
         The Rs changes as the sensor is in the different consentration of the target
         gas. The sample times and the time interval between samples could be configured
         by changing the definition of the macros.
============================================================*/ 
float MQRead(int mq_pin)
{
  int i;
  float rs=0;
 
  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }
 
  rs = rs/READ_SAMPLE_TIMES;
 
  return rs;  
}
 
/*=======================  MQGetGasPercentage ==================
Input:   rs_ro_ratio - Rs divided by Ro
         gas_id      - target gas type
Output:  ppm of the target gas
Remarks: This function passes different curves to the MQGetPercentage function which 
         calculates the ppm (parts per million) of the target gas.
===============================================================*/ 
int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
  if ( gas_id == GAS_LPG ) {
     return MQGetPercentage(rs_ro_ratio,LPGCurve);
  } else if ( gas_id == GAS_CH4 ) {
      return MQGetPercentage(rs_ro_ratio,CH4Curve);
  }    
 
  return 0;
}
 
/*========================  MQGetPercentage ===================
Input:   rs_ro_ratio - Rs divided by Ro
         pcurve      - pointer to the curve of the target gas
Output:  ppm of the target gas
Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm) 
         of the line could be derived if y(rs_ro_ratio) is provided. As it is a 
         logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic 
         value.
===============================================================*/ 
int  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10, (((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}            
