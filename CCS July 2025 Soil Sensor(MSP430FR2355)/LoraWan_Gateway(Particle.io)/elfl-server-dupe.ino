//Current IDE
// This #include statement was automatically added by the Particle IDE.
#include <ArduinoJson.h>

// This #include statement was automatically added by the Particle IDE.
#include <RH_RF95.h>

// This #include statement was automatically added by the Particle IDE.
#include <RF9X-RK.h>

// This #include statement was automatically added by the Particle IDE.
#include <RH_RF95.h>

// This #include statement was automatically added by the Particle IDE./
#include <RF9X-RK.h>

// This #include statement was automatically added by the Particle IDE.
#define ARDUINOJSON_ENABLE_PROGMEM 0
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#include <ArduinoJson.h>

//CURRENT USABLE VERSION

// This #include statement was automatically added by the Particle IDE.
//#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
//#define ARDUINOJSON_ENABLE_PROGMEM 0

#define SERIAL_DEBUG

int SEESAW_NUM = 1;

// This #include statement was automatically added by the Particle IDE.
#include <RHGenericDriver.h>

//for publishing to google
#include "Particle.h"

SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler;

// How often to publish a value
const std::chrono::milliseconds publishPeriod = 30s;

// The event name to publish with
const char *eventName = "data_file";

// rf95_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing server
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95  if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf95_client

#include <SPI.h>

#include "defines.h"
#include <string>


// 0 => BW = 125, Cr = 4/5, Sf = 128, CRC On, Medium Range, Default
// 1 => BW = 500, Cr = 4/5, Sf = 128, CRC On, Fast + Short Range
// 2 => BW = 32.25, Cr = 4/8, Sf = 512, CRC On, Slow + Long Range
// 3 => BW = 125, Cr = 4/8, Sf = 4096, CRC On, Low Data rate, CRC On, Slow + Long Range
// 4 => BW = 125, Cr = 4/5 Sf = 2048, CRC On, Slow + Long Range
// #define MODEM_CONFIG 2

// // +2 to +20, 13 default
#define TX_POWER 23
#define SF       10
#define BW       250000
#define CR       5

int counter = 0;

RH_RF95 rf95(A5,D2);
StaticJsonDocument<100> doc; // allocate on stack - we need about 100 bytes to have this working
StaticJsonDocument<200> doc_1; // allocate on stack - we need about 100 bytes to have this working
StaticJsonDocument<300> doc_2;

static int32_t twosComplement(int32_t val, uint8_t bits) {
  if (val & ((uint32_t)1 << (bits - 1))) {
    val -= (uint32_t)1 << bits;
  }
  return val;
}

void setup()
{

  
  
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  //while (!Serial) ; // Wait for serial port to be available
  if (!rf95.init())
    //Serial.println("init failed");
    Particle.publish("init failed");
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  //rf95.setModemConfig(MODEM_CONFIG); // set new modem configuration
  rf95.setFrequency(915);
  rf95.setTxPower(TX_POWER,false); // set new transmit power configuration
  rf95.setSpreadingFactor(SF); // set new spreading factor 2^10
  rf95.setSignalBandwidth(BW); // set new bandwidth
  rf95.setCodingRate4(CR); // set new coding rate
  rf95.setPayloadCRC(true);
   

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
//    driver.setTxPower(23, false);
}


void loop()
{
    

  if (rf95.available())
  {
    ////Particle.publish("Message Ready");
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
    if (rf95.recv(buf, &len))
    {
        //Print buf array
       /* for (int b = 24; b<32; b++){
            doc["buf"][b] = buf[b];
        }
        */
        /* Print buf length
        doc["length of buf"] = len;
        */
        
        ////Particle.publish(String(len));
      if (len == 14){
        //Oxygen calculation
        int res = 12;                                    //ADC resolution
        uint8_t oxygen[SEESAW_NUM] = {0,0,0,0,0,0,0,0,0,0,0,0}; //12 bits
        int oxygen_value = 0;                       
        float v_ref = 3.300;
        float v_input = 0.0;
        float o2 = 0.0;
        
        for (int i=res-1; i >= 0; i--){          //In case buf[] itself gets changed
            oxygen[i] = (((uint16_t)buf[11-i]));     //Last value in the oxygen[] will be  LSB(buf[0]) of ADC result. First value is MSB(buf[11])
        }
        for (int j=0; j<res; j++){
            //oxygen_value |= (oxygen[j]<<j);            //MSB first
            oxygen_value += oxygen[j]*((int)(pow(2,(res-1)-j)));
        }
        
        v_input = ((float)oxygen_value)/(((float)(pow(2,res)))-1);        //Converts digital value to analog input for display
        v_input *= v_ref * 1000;  
        o2 = 0.209 * (v_input/45.5);
        //oxygen_value &= 0xFFF; 
         //float_conversion.u8_value = co2_raw;
       // float Oxygen = float_conversion.float8;
       
       //json creation
       doc_2["id"] = buf[12];
       doc_2["lid_status"] = buf[13];
       doc_2["ADC_result"] = oxygen_value;
       doc_2["oxygen_mV"] = v_input;
       doc_2["o2%"]       = o2;
 
      
      
        std::string minif_json3;
        serializeJson(doc_2, minif_json3);

        Particle.publish("data_file", minif_json3.c_str(), PRIVATE);

      
        
      }
      else if (len == 26) {                  //For seesaw_buf
        // soil temperature calculation
        float soil_temp[SEESAW_NUM] = {0.0, 0.0, 0.0, 0.0};
        for (int i = 0; i < SEESAW_NUM; i++) {
          int32_t int_soil_temp = ((uint32_t)buf[4*i] << 24) | ((uint32_t)buf[1 + 4*i] << 16) |
                    ((uint32_t)buf[2 + 4*i] << 8) | (uint32_t)buf[3 + 4*i];
          soil_temp[i] = (1.0 / (1UL << 16)) * int_soil_temp;
        }

        // moisture calculation
        uint16_t moisture[SEESAW_NUM] = {0, 0, 0, 0};
        for (int i = 0; i < SEESAW_NUM; i++) {
          moisture[i] = ((uint16_t)buf[16 + (2*i)]) << 8 | (uint16_t)buf[17 + (2*i)];
        }
       
        
        doc["id"] = buf[24];
        doc["lid_status"] = buf[25];
        doc["moisture"] = moisture[0];
        doc["soil_temp"] = soil_temp[0];


        // Corrected code
        std::string minif_json;
        serializeJson(doc, minif_json);

        Particle.publish("data_file", minif_json.c_str(), PRIVATE);
        ////Log.info("published: %s", minif_json.c_str());

        // commented out error code for reference!!!!! 

        // String minif_json;
        // serializeJson(doc, minif_json); // serialize object to String
        // //Particle.publish(minif_json);
        // Particle.publish("data_file", minif_json, PRIVATE);
        // Log.info("published: %s", minif_json);
        
        
      }
      //else 
      else if(len == 35){
        // Pressure coefficient calculation
            int32_t c00 = ((uint32_t)buf[0] << 12) | ((uint32_t)buf[1] << 4) |
                  (((uint32_t)buf[2] >> 4) & 0x0F);
            c00 = twosComplement(c00, 20);
          
            int32_t c10 = (((uint32_t)buf[2] & 0x0F) << 16) | ((uint32_t)buf[3] << 8) |
                  (uint32_t)buf[4];
            c10 = twosComplement(c10, 20);
          
            int16_t c01 = twosComplement(((uint16_t)buf[5] << 8) | (uint16_t)buf[6], 16);
            int16_t c11 = twosComplement(((uint16_t)buf[7] << 8) | (uint16_t)buf[8], 16);
            int16_t c20 = twosComplement(((uint16_t)buf[9] << 8) | (uint16_t)buf[10], 16);
            int16_t c21 = twosComplement(((uint16_t)buf[11] << 8) | (uint16_t)buf[12], 16);
            int16_t c30 = twosComplement(((uint16_t)buf[13] << 8) | (uint16_t)buf[14], 16);
      
            // pressure calculation
            int32_t raw_psr = twosComplement(((uint32_t)buf[15] << 16) | ((uint32_t)buf[16] << 8) | ((uint32_t)buf[17]), 24);
            int32_t raw_tmp = twosComplement(((uint32_t)buf[18] << 16) | ((uint32_t)buf[19] << 8) | ((uint32_t)buf[20]), 24);
      
            float scaled_rawtemp = (float)raw_tmp / 524288;
            float pressure = (float)raw_psr / 1572864;
      
            pressure =
                (int32_t)c00 +
                pressure * ((int32_t)c10 +
                             pressure * ((int32_t)c20 + pressure * (int32_t)c30)) +
                scaled_rawtemp *
                    ((int32_t)c01 +
                     pressure * ((int32_t)c11 + pressure * (int32_t)c21));
      
            pressure = pressure / 100;
      
            // CO2, temp, humidity calculation
            union {
              uint32_t u32_value;
              float float32;
            } float_conversion;
      
            uint32_t co2_raw = (((uint32_t)buf[21]) << 24) + (((uint32_t)buf[22]) << 16) + (((uint32_t)buf[23]) << 8) + ((uint32_t)buf[24]);
            float_conversion.u32_value = co2_raw;
            float co2_conc = float_conversion.float32;
      
            uint32_t temp_raw = (((uint32_t)buf[25]) << 24) + (((uint32_t)buf[26]) << 16) + (((uint32_t)buf[27]) << 8) + ((uint32_t)buf[28]);
            float_conversion.u32_value = temp_raw;
            float temp = float_conversion.float32;
      
            uint32_t hum_raw = (((uint32_t)buf[29]) << 24) + (((uint32_t)buf[30]) << 16) + (((uint32_t)buf[31]) << 8) + ((uint32_t)buf[32]);
            float_conversion.u32_value = hum_raw;
            float hum_conc = float_conversion.float32;
            
            float soil_temp[SEESAW_NUM] = {0.0, 0.0, 0.0, 0.0};
            for (int i = 0; i < SEESAW_NUM; i++) {
              int32_t int_soil_temp = ((uint32_t)buf[35] << 24) | ((uint32_t)buf[36] << 16) |
                        ((uint32_t)buf[37] << 8) | (uint32_t)buf[38];
              soil_temp[i] = (1.0 / (1UL << 16)) * int_soil_temp;
            }
    
            // moisture calculation
            uint16_t moisture[SEESAW_NUM] = {0, 0, 0, 0};
            for (int i = 0; i < SEESAW_NUM; i++) {
              moisture[i] = ((uint16_t)buf[39]) << 8 | (uint16_t)buf[40];
            }
            

           
            // JSON creation
            doc_1["id"] = buf[33];
            doc_1["lid_status"] = buf[34];
            doc_1["pressure"] = pressure;
            doc_1["co2_concentration"] = co2_conc;
            doc_2["Carbon Dioxide"] = co2_conc;
            doc_1["air_temp"] = temp;
            doc_1["humidity"] = hum_conc;
          // doc_1["moisture_0"] = moisture[0];
            //doc_1["soil_temp_0"] = soil_temp[0];
            
            
            // Corrected code
            std::string minif_json2;
            serializeJson(doc_1, minif_json2);

            Particle.publish("data_file", minif_json2.c_str(), PRIVATE);
            ////Log.info("published: %s", minif_json2.c_str());

            int16_t rssi = rf95.lastRssi();
            counter = counter + 1;
            Particle.publish("Count Value :", (String)counter);
            

            ////Particle.publish("RSSI", String(rssi));
            //Log.info("published RSSI: %d", rssi);
            
            // commented out error code for reference!!!! 
            
            // String minif_json2;
            // serializeJson(doc_1, minif_json2); // serialize object to String
            // //Particle.publish(minif_json2);
            // Particle.publish("data_file", minif_json2, PRIVATE);
            // Log.info("published: %s", minif_json2);
            
            // //int16_t rssi = rf95.lastRssi();
            // Particle.publish("RSSI: ", String(rf95.lastRssi()));
            
            
    }
        
    }
    else
    {
      Serial.println("recv failed");
   }
   
 delay(500); //5000
}

}
 