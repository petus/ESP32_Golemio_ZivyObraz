// MHD or/and Waste Stations - uncomment
#define MHD // for MHD/Public Transport, fill the stop ID in the code, line 59 - 73
#define WasteStations // waste stations

// WiFi credentials
#define WIFI_PASS "PASS"
#define WIFI_SSID "SSID"

#define HTTP_GET_INTERVAL_SECS 900        //interval in secs to load data to Zivy Obraz, do not put less than 60s

#ifdef WasteStations
String Lat = "50";  //latitude
String Long = "14"; //longtitude
String Range = "50";        //range in meters
String Accessibility = "1"; // dostupnost (1) - volně; (2) - obyvatele domu; (3) - neznámá
#endif

// PID Golemio server token here - see https://api.golemio.cz/api-keys/auth/sign-in
String auth_token = "xxxyyyzzz";

// Zivy obraz
String Server_Name = "http://in.zivyobraz.eu";        // server URL for "Zivy Obraz"
String Server_Key = "/?import_key=xxxyyyzzz";  // your import token for "Zivy Obraz" https://zivyobraz.eu/
