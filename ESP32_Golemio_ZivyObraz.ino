
/*
Prague PID (Public Transport System) departure informations extract and import to "Zivy Obraz" https://zivyobraz.eu/
Proof of concept. No error handling. Use at your own risk.
Use ArduinoJson.h library V6 and higher.
>> Caution - See the calculation of JSON buffer size. If you overload it, it crashes <<
It can be overloaded by asking data about too many trains into the future.
For more details, see https://arduinojson.org/v6/assistant/#/step1
Have fun!
Karel Kotrba (https://github.com/kkodl/MHD_GW_ESP32)
and chiptron.cz (https://github.com/petus/ESP32_Golemio_ZivyObraz) -> rewriting of current code, added * before 
departure time for air conditioned vehicles and possibility to show waste stations 
and pick up days
Use ESP32 library 2.x.y
*/

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "setup.h"

char json[6000];                  // buffer response from golemio into char array
String payload;                   //store for the golemio direct response

  // Inside the brackets, 6000 is the capacity of the memory pool in bytes.
  // Don't forget to change this value to match your JSON document.
  // Use arduinojson.org/v6/assistant to compute the capacity.

StaticJsonDocument<8192> jsonBuffer;    // place for deserialization and data extraction

WiFiClient wifi;
HTTPClient http;

#ifdef MHD
int train_count;        // how many vehicles into the future we are asking i.e. 4 = next four vehicles leaving
int Stop_Number;        // our own index of stop i.e. if we ask for data from 3 stops, we must somehow label the values in Zivy Obraz
                        // so this is label index for stops

String Number_Of_Trips;   // in http querry it has to be as a string
String Stop_Ids;          // PID stop code storage, see http://data.pid.cz/stops/xml/StopsByName.xml
String Name;              // string to store name of value for Zivy Obraz
#endif

long HttpGetStartMillis;  // timer

void setup() {
  USBSerial.begin(115200);
  delay (10);
  connectWifi();
  HttpGetStartMillis = millis() + 1000 * HTTP_GET_INTERVAL_SECS;    // fake the timer, we ask immediately after boot
}


void loop() {
    USBSerial.println ("Time to get data!");
    HttpGetStartMillis = millis();
    connectWifi();

#ifdef MHD
    Number_Of_Trips = "4";        // Number of next connections
    Stop_Ids = "ids=XXXXXXX";     // code of stop, see readme (https://github.com/kkodl/MHD_GW_ESP32/blob/main/docs/ODJEZDY%20MHD%20V%20PRAZE.pdf)
    Stop_Number = 1;              // index of Stop in Zivy Obraz
    Request_Train();              // ask for data and push it

    /* In case you need more Stops*/
    //Number_Of_Trips = "3";        // Number of next connections
    //Stop_Ids = "ids=_ZDE KOD PRO ZASTAVKU #2_";     // code of stop, see readme
    //Stop_Number = 2;              // index of Stop in Zivy Obraz
    //Request_Train();              // ask for data a push it

    //Number_Of_Trips = "3";        // Number of next connections
    //Stop_Ids = "ids=_ZDE KOD PRO ZASTAVKU #3_";     // code of stop, see readme
    //Stop_Number = 3;              // index of Stop in Zivy Obraz
    //Request_Train();              // ask for data a push it
#endif // MHD

#ifdef WasteStations
    Request_WasteStations();
#endif // WasteStations

    USBSerial.println("--------- Done for the moment ------------------");

  Serial.flush(); 
  esp_sleep_enable_timer_wakeup(HTTP_GET_INTERVAL_SECS * 1000000);
  esp_deep_sleep_start();
}


void Request_Train(){
  train_count = Number_Of_Trips.toInt();
  USBSerial.println("GET request to PID Golemio server");
  String serverName = "https://api.golemio.cz/v2/pid/departureboards?" + Stop_Ids + "&total=" + Number_Of_Trips + "&preferredTimezone=Europe%2FPrague" + "&airCondition=true";  // server address, must include querry
  USBSerial.println(serverName);
  http.begin(serverName.c_str());
  http.addHeader("Content-Type", "application/json; charset=utf-8");
  http.addHeader("X-Access-Token", auth_token);
  //http.addHeader("Accept-Encoding: identity"); // ask for noncompressed data, otherwise it is gzip
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  if (httpResponseCode>0) {
    USBSerial.print("HTTP Response code: ");
    USBSerial.println(httpResponseCode);
    payload = http.getString();
    payload.toCharArray(json, payload.length()+1);
    USBSerial.println(json);
   }
  else {
    USBSerial.print("Error code: ");
    USBSerial.println(httpResponseCode);
   }
  // Free resources
  http.end();


  // Deserialize the JSON document
DeserializationError error = deserializeJson(jsonBuffer, json);
  // Test if parsing succeeds.
  if (error) {
    USBSerial.print(F("deserializeJson() failed: "));
    USBSerial.println(error.f_str());
    return;
  }

  USBSerial.print("Number of next connections: ");
  USBSerial.println(train_count);

  // Get stop_name from "stops" object
  String stop_name = jsonBuffer["stops"][0]["stop_name"];

  // Get values from JSON
  JsonArray departures = jsonBuffer["departures"].as<JsonArray>();
  int train_cnt = 0;

  for (JsonObject departure : departures) 
  {
    // Get departure_timestamp - predicted and find the time of predicted departure
    String departure_predicted = departure["departure_timestamp"]["predicted"];
    String time_predicted = String(departure_predicted).substring(11, 16);

    // Get last_stop - name
    String headsign = departure["trip"]["headsign"];

    // Get route - short_name
    String route_short_name = departure["route"]["short_name"];

    bool is_air_conditioned = departure["trip"]["is_air_conditioned"] | false;
    if (is_air_conditioned)
    {
      time_predicted = String('*') + time_predicted;  // add '*' in case the vehicle is airconditioned
    }

    // Write messages to Serial Monitor
    /*USBSerial.print("Stop name: ");
    USBSerial.println(stop_name);
    USBSerial.print("Departure Time (predicted): ");
    USBSerial.println(time_predicted);
    USBSerial.print("Last Stop Name: ");
    USBSerial.println(headsign);
    USBSerial.print("Route Short Name: ");
    USBSerial.println(route_short_name);*/

    USBSerial.print("Z_" + stop_name + "_Linka_" + route_short_name + "_" + train_cnt);
    USBSerial.println(" " + route_short_name);
    USBSerial.print("Z_" + stop_name + "_Linka_" + route_short_name + "_" + train_cnt + "_čas");
    USBSerial.println(" " + time_predicted);
    USBSerial.print("Z_" + stop_name + "_Linka_" + route_short_name + "_" + train_cnt + "_směr");
    USBSerial.println(" " + headsign);

    Push_Data_To_LiPi(String("Z_" + stop_name + "_Linka_" + route_short_name + "_" + train_cnt), route_short_name);        // and push it to Ž.O.
    Push_Data_To_LiPi(String("Z_" + stop_name + "_Linka_" + route_short_name + "_" + train_cnt + "_čas"), time_predicted); // and push it to Ž.O.
    Push_Data_To_LiPi(String("Z_" + stop_name + "_Linka_" + route_short_name + "_" + train_cnt + "_směr"), headsign);      // and push it to Ž.O.

    train_cnt++;
  }
}

void Request_WasteStations(){
  USBSerial.println("GET request to Waste Stations Golemio server");
  String serverName = "https://api.golemio.cz/v2/sortedwastestations?latlng=" + Lat + ',' + Long + "&range=" + Range + "&accessibility=" + Accessibility;  // server address, must include querry
  USBSerial.println(serverName);
  http.begin(serverName.c_str());
  http.addHeader("Content-Type", "application/json; charset=utf-8");
  http.addHeader("X-Access-Token", auth_token);
  //http.addHeader("Accept-Encoding: identity"); // ask for noncompressed data, otherwise it is gzip
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  if (httpResponseCode>0) {
    USBSerial.print("HTTP Response code: ");
    USBSerial.println(httpResponseCode);
    payload = http.getString();
    payload.toCharArray(json, payload.length()+1);
    USBSerial.println(payload);
   }
  else {
    USBSerial.print("Error code: ");
    USBSerial.println(httpResponseCode);
   }
  // Free resources
  http.end();

  delay(100);

    // Deserialize the JSON document
  DeserializationError error = deserializeJson(jsonBuffer, json);
  // Test if parsing succeeds.
  if (error) {
    USBSerial.print(F("deserializeJson() failed: "));
    USBSerial.println(error.f_str());
    return;
  }

  USBSerial.println();
  // Go through of each "Feature"
  JsonArray features = jsonBuffer["features"].as<JsonArray>();
  for (JsonObject feature : features) {
    JsonArray containers = feature["properties"]["containers"].as<JsonArray>();
    for (JsonObject container : containers) {
      // Get "pick_days"
      String pick_days = container["cleaning_frequency"]["pick_days"];
      // Get "description"
      String description = container["trash_type"]["description"];
      // Get "percent_calculated"
      String percent_calculated = container["last_measurement"]["percent_calculated"];

      // Write messages to Serial Monitor
      USBSerial.print("Description: ");
      USBSerial.println(description);
      USBSerial.print("Days: ");
      USBSerial.println(pick_days);
      Serial.print("Percent Calculated: ");
      Serial.println(percent_calculated);

      Push_Data_To_LiPi(description, pick_days);  
	  //Push_Data_To_LiPi(description, percent_calculated);
    }
  }
}


// ----------------------------- Connects to Wifi ---------------------------
// if connected, it does nothing
//
void connectWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  delay(500);
  USBSerial.print("\nConnecting to WiFi ");
  //USBSerial.print(WIFI_SSID);
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.hostname("MHD_GW");
  WiFi.begin(WIFI_SSID,WIFI_PASS);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    USBSerial.print(".");
    i++;
    if(i>50) // timeout 200ms * 50x (10s), otherwise goes to sleep
    {
      Serial.flush(); 
      esp_sleep_enable_timer_wakeup(HTTP_GET_INTERVAL_SECS * 1000000);
      esp_deep_sleep_start();
    }
    }
  USBSerial.println("Connected");
}


// ---------- Pushes the data passed to it towards Živý Obraz server
// 
void Push_Data_To_LiPi(String DataName, String DataValue){
  DataName.replace(" ","%20");           // replace spaces in names, to have clean http request
  DataValue.replace(" ","%20");           // replace spaces in value, to have clean http request
  HTTPClient http;
  String serverPath = Server_Name + Server_Key + "&" + DataName + "=" + DataValue;    // compose URL for Ž.O. server
  USBSerial.print ("Pushing to Ž.O.: ");
  USBSerial.print(serverPath);
  http.begin(wifi, serverPath.c_str());
  int httpResponseCode = http.GET();      // Send HTTP GET request
  if (httpResponseCode>0) {
    USBSerial.print(" >> Response: ");
    USBSerial.print(httpResponseCode);
    String payload = http.getString();
    //USBSerial.println(payload);
    } else {
      USBSerial.print("Err code: ");
      USBSerial.println(httpResponseCode);
     }
  http.end();         // Free resources
  USBSerial.println();   // newline to console
  delay (200);       //most probbably not needed, just not to piss off the server on LiPi side
}
