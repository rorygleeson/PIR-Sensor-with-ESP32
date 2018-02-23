/**
 * espPIR.ino
 *
 *  Created on: 14.11.2017
 *
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#define USE_SERIAL Serial
#include <esp_system.h>


const char* ssid = "xxxxxxxxxxxx";
const char* password =  "xxxxxxxxxxx";
 

int attemptsCount = 0;
int wake;

void setup() {
    
    USE_SERIAL.begin(115200);
    delay(1000); /* Take some time to open up the Serial Monitor */
    USE_SERIAL.print("In Setup..........\n");
    wake  = wakeup_reason1();
    Serial.println("WAKE IS..\n");                      /* The wake reason allows us to know what woke the esp32 up, ext int or other */
    Serial.println(wake);


    USE_SERIAL.printf("Wait one minute for PIR to settle");
    for(uint8_t t = 60; t > 0; t--) {
        delay(1000);
    }


    
    USE_SERIAL.print("First DISCONNECT from  WIFI..then connect to WIFI....\n");       
    WiFi.disconnect(true);                                                              // forum recommendation to fix  bug of not connecting to wifi...maybe not required.
    WiFi.begin(ssid, password);
 
    while (WiFi.waitForConnectResult() != WL_CONNECTED) 
    {
            Serial.println("Waiting to connect to WiFi..\n");
            ++attemptsCount;
            USE_SERIAL.println("Attempts...\n");
            USE_SERIAL.println(attemptsCount);
            delay(2000);                              // wait 2 secs
            
              if(attemptsCount == 5)                  // we have failed to connect to WIFI after 5 attempts
              {   
                   USE_SERIAL.print("have tried max number of times to connect to wifi..");
             
                   if(wake == 1)
                   { USE_SERIAL.print("ext interrupt occured, but failed to connect to WIFI..turn off wifi and do a RESTART in 2 seconds");                   
                     delay(2000); 
                     esp_restart();    // after issuing this reset, the wake cause will be 0, so need to avoid running in a forver loop
                    }
                    else
                    {
                      USE_SERIAL.print("This is a wake = 0 ..this means it is not from an external interrupt so must be from a previous reset followed by wifi failure, do not reset again...\n");
                      delay(2000);
                      USE_SERIAL.print("Failed to connect after a wake 0...dont restart...Now set up GPIO33 for external interrupt...\n");
                      esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1); //1 = High, 0 = Low
                      USE_SERIAL.print("All done, Failed to connect to WIFI twice, sleep now ...\n");
                      Serial.println("Going to sleep now until external interrupt occurs");
                      esp_deep_sleep_start();
                    }
                   
              }
    }
    
    Serial.println("Continue, Connected to the WiFi network");
    
}

void loop() {

    USE_SERIAL.print("In LOOP...\n");
    Serial.println("WAKE IS..");
    Serial.println(wake);


    
    String clientMac = "";
    unsigned char mac[6];
    WiFi.macAddress(mac);
    clientMac += macToStr(mac);
    Serial.println("My Mac ID is ..");
    Serial.println(clientMac);


    
    if((WiFi.status() == WL_CONNECTED)) 
    {
        Serial.println("Connected to Wifi, setup sleep mode ...");
        Serial.println("Send event to SkyGrid");
        HTTPClient http;
        USE_SERIAL.print("[HTTP] begin...\n");
        String url = "http://192.168.0.100/myHub.php?btnID=";
        url += clientMac;
        url += "&resetCause=";
        url += wake;

        
        USE_SERIAL.print("URL is ...\n");
        USE_SERIAL.print(url);
        
        http.begin(url); //HTTP

        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

              // httpCode will be negative on error
             if(httpCode > 0) 
             {
                  // HTTP header has been send and Server response header has been handled
                  USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

                  // file found at server
                  if(httpCode == HTTP_CODE_OK) 
                  {
                      String payload = http.getString();
                      USE_SERIAL.println(payload);
                  }
             } 
             else // connected to wifi but http get failed
             {
                USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
                USE_SERIAL.print("WIFI  connected, but update  failed.");
                http.end();
                WiFi.disconnect(true);
                delay(2000); 
                    if(wake ==1)
                    { 
                     USE_SERIAL.print("ext interrupt occured, connected to wifi but failed http get..turn off wifi and do a RESTART in 2 seconds");                   
                     delay(2000); 
                     esp_restart();    // after issuing this reset, the wake cause will be 0, so need to avoid running in a forver loop
                    
                    }
                    else
                    {
                      USE_SERIAL.print("Http get failed: This is a wake = 0 .. do not reset again...\n");
                      delay(2000);
                      USE_SERIAL.print("connected to WIFI but failed to do http get after a wake 0...dont restart...Now set up GPIO33 for external interrupt...\n");
                      esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1); //1 = High, 0 = Low
                      USE_SERIAL.print("All done, Failed to connect to WIFI twice, sleep now ...\n");
                      Serial.println("Going to sleep now until external interrupt occurs");
                      esp_deep_sleep_start();
                    }
             }
   
       http.end();

    }


    
      USE_SERIAL.print("delay 3 secs...");
      delay(3000);
      USE_SERIAL.print("Now set up GPIO33 for external interrupt...");
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1); //1 = High, 0 = Low
      USE_SERIAL.print("WIFI turn OFF  ...\n");
      WiFi.disconnect(true);
      USE_SERIAL.print("All done, sleep now ...\n");
      Serial.println("Going to sleep now until external interrupt occurs");
      esp_deep_sleep_start();
}



int wakeup_reason1() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  return wakeup_reason;
}







String macToStr(const uint8_t* mac)
{
String result;
for (int i = 0; i < 6; ++i) {
result += String(mac[i], 16);
if (i < 5)
result += ':';
}
return result;
}



