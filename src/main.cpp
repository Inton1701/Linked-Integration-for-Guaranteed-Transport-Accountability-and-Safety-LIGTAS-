#include <Arduino.h>
#include <PN532_HSU.h>
#include <PN532.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <HardwareSerial.h>
#include <nvs_flash.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// ========================
// PIN DEFINITIONS
// ========================
#define PN532_RX        16  // ESP32 GPIO16 (RX2) -> PN532 TX
#define PN532_TX        17  // ESP32 GPIO17 (TX2) -> PN532 RX
#define BUZZER_PIN      25  // Buzzer on GPIO25
#define RED_LED_PIN     23  // Red LED on GPIO23
#define GREEN_LED_PIN   22  // Green LED on GPIO22

// ========================
// HARDWARE OBJECTS
// ========================
HardwareSerial PN532Serial(1);  // UART1 for PN532
PN532_HSU pn532hsu(PN532Serial);
PN532 nfc(pn532hsu);
AsyncWebServer server(80);
Preferences prefs;
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

// ========================
// GLOBAL VARIABLES
// ========================
bool nfcAvailable = false;
bool firebaseReady = false;

// Driver Info Cache
String driverName = "";
String driverLicenseNo = "";
String driverPhone = "";
String driverEmail = "";
bool driverInfoLoaded = false;

// WiFi & AP Settings
String wifiSSID = "";
String wifiPassword = "";
String apSSID = "ESP32-RFID";
String apPassword = "12345678";

// Firebase Settings
const String FIREBASE_PROJECT_ID = "studious-bit-420613";
const String FIREBASE_API_KEY = "AIzaSyAge9lFaqLvPGVo_t6hymfis4FP_ESZXFc";
String deviceID = "DVC0001";

// RFID Tracking
String lastRFID = "";
unsigned long lastRFIDTime = 0;
const unsigned long rfidCooldown = 2000; // 2 seconds cooldown between same card reads

// ========================
// FUNCTION DECLARATIONS
// ========================
void initBuzzer();
void initLEDs();
void initWiFi();
void initTime();
void initNFC();
void initWebServer();
void loadSettings();
void saveSettings();
void handleRFID();
void initFirebase();
bool fetchDriverInfo();
bool sendScanWithDriverInfo(String rfid);
bool sendToAPI(String rfid);
String uidToString(uint8_t *uid, uint8_t length);
String getISOTimestamp();
void beep(int duration = 100);
void beepTimes(int times, int duration = 100, int delayBetween = 100);
void setRedLED(bool state);
void setGreenLED(bool state);
void blinkRedLED(int times = 1, int duration = 100);
String getWebPage();

// ========================
// SETUP
// ========================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n========================================");
  Serial.println("L.I.G.T.A.S RFID System Starting...");
  Serial.println("========================================");

  // Initialize NVS (Non-Volatile Storage)
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    Serial.println("NVS partition was truncated, erasing...");
    nvs_flash_erase();
    err = nvs_flash_init();
  }
  if (err == ESP_OK) {
    Serial.println("✓ NVS Initialized");
  } else {
    Serial.println("✗ NVS Initialization Failed!");
  }

  // Initialize components
  initBuzzer();
  initLEDs();
  loadSettings();
  initWiFi();
  initTime();
  initFirebase();
  initNFC();
  initWebServer();

  // Boot sequence: beep twice
  Serial.println("\n--- Boot Sequence ---");
  beepTimes(2, 200, 200);
  Serial.println("✓ Boot Complete");

  Serial.println("\n========================================");
  Serial.println("✓ System Ready!");
  Serial.println("========================================");
  Serial.print("Device ID: ");
  Serial.println(deviceID);
  Serial.print("Firebase Project: ");
  Serial.println(FIREBASE_PROJECT_ID);
  Serial.print("AP SSID: ");
  Serial.println(apSSID);
  Serial.print("AP Password: ");
  Serial.println(apPassword);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("NFC Reader: ");
  Serial.println(nfcAvailable ? "Connected" : "Not Available");
  Serial.print("Firebase: ");
  Serial.println(firebaseReady ? "Connected" : "Not Connected");
  Serial.print("Driver Info: ");
  Serial.println(driverInfoLoaded ? ("Loaded (" + driverName + ")") : "Not Loaded");
  Serial.println("========================================\n");
}

// ========================
// MAIN LOOP
// ========================
void loop() {
  if (nfcAvailable) {
    handleRFID();
  }
  delay(100);
}

// ========================
// INITIALIZATION FUNCTIONS
// ========================
void initBuzzer() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("✓ Buzzer Initialized");
}

void initLEDs() {
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  
  // Red LED on by default, green LED off
  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, LOW);
  
  Serial.println("✓ LEDs Initialized");
}

void loadSettings() {
  prefs.begin("rfidsystem", false);
  
  wifiSSID = prefs.getString("wifiSSID", "");
  wifiPassword = prefs.getString("wifiPass", "");
  apSSID = prefs.getString("apSSID", "ESP32-RFID");
  apPassword = prefs.getString("apPass", "12345678");
  deviceID = prefs.getString("deviceID", "DVC0001");
  
  prefs.end();
  Serial.println("✓ Settings Loaded");
}

void saveSettings() {
  prefs.begin("rfidsystem", false);
  
  prefs.putString("wifiSSID", wifiSSID);
  prefs.putString("wifiPass", wifiPassword);
  prefs.putString("apSSID", apSSID);
  prefs.putString("apPass", apPassword);
  prefs.putString("deviceID", deviceID);
  
  prefs.end();
  Serial.println("✓ Settings Saved");
}

void initWiFi() {
  Serial.println("\n--- WiFi Initialization ---");
  
  // Disconnect any previous connections
  WiFi.disconnect(true);
  delay(100);
  
  // Configure AP mode
  WiFi.mode(WIFI_AP_STA);
  delay(100);
  
  // Start AP mode
  bool apStarted = WiFi.softAP(apSSID.c_str(), apPassword.c_str(), 1, 0, 4);
  delay(500);
  
  if (apStarted) {
    Serial.println("✓ Access Point Started");
  } else {
    Serial.println("✗ Access Point Failed");
  }
  
  Serial.print("AP SSID: ");
  Serial.println(apSSID);
  Serial.print("AP Password: ");
  Serial.println(apPassword);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // Connect to WiFi if configured
  if (wifiSSID.length() > 0) {
    Serial.print("Connecting to WiFi: ");
    Serial.println(wifiSSID);
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n✓ WiFi Connected");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\n✗ WiFi Connection Failed");
    }
  } else {
    Serial.println("No WiFi credentials configured");
  }
}

void initTime() {
  Serial.println("\n--- Time Initialization ---");
  
  if (!WiFi.isConnected()) {
    Serial.println("✗ Cannot sync time: No WiFi connection");
    return;
  }
  
  Serial.println("Syncing time with NTP server...");
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");  // GMT+8 for Philippines
  
  // Wait for time sync with timeout
  int attempts = 0;
  time_t now = time(nullptr);
  while (now < 1577836800 && attempts < 20) {  // Jan 1, 2020
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    attempts++;
  }
  Serial.println();
  
  if (now > 1577836800) {
    Serial.println("✓ Time synced successfully");
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    Serial.print("Current time: ");
    Serial.println(asctime(&timeinfo));
  } else {
    Serial.println("⚠ Time sync failed - timestamps may be inaccurate");
  }
}

void initFirebase() {
  Serial.println("\n--- Firebase Initialization ---");
  
  if (!WiFi.isConnected()) {
    Serial.println("✗ Cannot initialize Firebase: No WiFi connection");
    firebaseReady = false;
    return;
  }

  firebaseReady = true;
  Serial.println("✓ Firebase Ready (REST API)");
  Serial.print("Project ID: ");
  Serial.println(FIREBASE_PROJECT_ID);
  
  // Fetch driver info immediately after Firebase is ready
  if (fetchDriverInfo()) {
    Serial.println("✓ Driver Info Loaded Successfully");
  } else {
    Serial.println("⚠ Driver Info Not Found - Please register driver for this device");
  }
}

void initNFC() {
  Serial.println("\n--- NFC Initialization ---");
  
  // Initialize Serial for PN532 (GPIO16=RX, GPIO17=TX)
  PN532Serial.begin(115200, SERIAL_8N1, PN532_RX, PN532_TX);
  delay(500);

  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  
  if (!versiondata) {
    Serial.println("✗ PN532 not found!");
    Serial.println("Please check wiring:");
    Serial.println("  PN532 TX -> ESP32 GPIO16");
    Serial.println("  PN532 RX -> ESP32 GPIO17");
    Serial.println("  PN532 VCC -> 3.3V");
    Serial.println("  PN532 GND -> GND");
    nfcAvailable = false;
    return;
  }

  nfcAvailable = true;
  Serial.print("✓ Found PN5");
  Serial.print((versiondata >> 24) & 0xFF, HEX);
  Serial.print(" Firmware v");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print(".");
  Serial.println((versiondata >> 8) & 0xFF, DEC);
  
  nfc.SAMConfig();
  Serial.println("✓ NFC Ready");
}

void initWebServer() {
  Serial.println("\n--- Web Server Initialization ---");
  
  // Serve main configuration page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", getWebPage());
  });

  // Get settings
  server.on("/api/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(1024);
    doc["deviceID"] = deviceID;
    doc["apSSID"] = apSSID;
    doc["wifiSSID"] = wifiSSID;
    doc["wifiConnected"] = WiFi.isConnected();
    doc["wifiIP"] = WiFi.localIP().toString();
    doc["apIP"] = WiFi.softAPIP().toString();
    doc["firebaseProject"] = FIREBASE_PROJECT_ID;
    doc["firebaseReady"] = firebaseReady;
    doc["driverInfoLoaded"] = driverInfoLoaded;
    doc["driverName"] = driverName;
    doc["driverLicenseNo"] = driverLicenseNo;
    doc["driverPhone"] = driverPhone;
    doc["driverEmail"] = driverEmail;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // Update settings
  server.on("/api/settings", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      DynamicJsonDocument doc(512);
      DeserializationError error = deserializeJson(doc, data);
      
      if (error) {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
      }
      
      bool needRestart = false;
      
      if (doc.containsKey("deviceID")) {
        deviceID = doc["deviceID"].as<String>();
      }
      
      if (doc.containsKey("wifiSSID")) {
        wifiSSID = doc["wifiSSID"].as<String>();
        needRestart = true;
      }
      
      if (doc.containsKey("wifiPassword")) {
        wifiPassword = doc["wifiPassword"].as<String>();
        needRestart = true;
      }
      
      if (doc.containsKey("apSSID")) {
        apSSID = doc["apSSID"].as<String>();
        needRestart = true;
      }
      
      if (doc.containsKey("apPassword")) {
        apPassword = doc["apPassword"].as<String>();
        needRestart = true;
      }
      
      saveSettings();
      
      String response = "{\"success\":true,\"needRestart\":" + String(needRestart ? "true" : "false") + "}";
      request->send(200, "application/json", response);
      
      Serial.println("Settings updated via web interface");
    });

  // Test buzzer
  server.on("/api/beep", HTTP_POST, [](AsyncWebServerRequest *request) {
    beep(200);
    request->send(200, "application/json", "{\"success\":true}");
  });

  // Refresh driver info
  server.on("/api/refresh-driver", HTTP_POST, [](AsyncWebServerRequest *request) {
    bool success = fetchDriverInfo();
    String response = "{\"success\":" + String(success ? "true" : "false") + ",";
    response += "\"driverInfoLoaded\":" + String(driverInfoLoaded ? "true" : "false") + ",";
    response += "\"driverName\":\"" + driverName + "\"}";
    request->send(200, "application/json", response);
  });

  // Get current time and date
  server.on("/api/time", HTTP_GET, [](AsyncWebServerRequest *request) {
    time_t now = time(nullptr);
    bool timeSynced = (now > 1577836800); // Check if time is synced (after Jan 1, 2020)
    
    DynamicJsonDocument doc(512);
    doc["timeSynced"] = timeSynced;
    doc["timestamp"] = (unsigned long)now;
    
    if (timeSynced) {
      struct tm timeinfo;
      localtime_r(&now, &timeinfo);
      
      char dateStr[20];
      char timeStr[20];
      strftime(dateStr, sizeof(dateStr), "%B %d, %Y", &timeinfo);
      strftime(timeStr, sizeof(timeStr), "%I:%M:%S %p", &timeinfo);
      
      doc["date"] = String(dateStr);
      doc["time"] = String(timeStr);
    } else {
      doc["date"] = "Time not synced";
      doc["time"] = "--:--:--";
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // Sync time endpoint
  server.on("/api/sync-time", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!WiFi.isConnected()) {
      request->send(400, "application/json", "{\"success\":false,\"error\":\"No WiFi connection\"}");
      return;
    }
    
    Serial.println("Manual time sync requested...");
    configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    delay(2000);
    
    time_t now = time(nullptr);
    bool success = (now > 1577836800);
    
    String response = "{\"success\":" + String(success ? "true" : "false") + "}";
    request->send(200, "application/json", response);
  });

  // Restart ESP32
  server.on("/api/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"success\":true}");
    delay(500);
    ESP.restart();
  });

  server.begin();
  Serial.println("✓ Web Server Started");
  Serial.print("Configuration URL: http://");
  Serial.println(WiFi.softAPIP());
}

// ========================
// RFID HANDLING
// ========================
void handleRFID() {
  uint8_t uid[10];
  uint8_t uidLength;

  // Check for RFID card
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100)) {
    String rfid = uidToString(uid, uidLength);
    unsigned long currentTime = millis();
    
    // Check if this is the same card within cooldown period
    if (rfid == lastRFID && (currentTime - lastRFIDTime) < rfidCooldown) {
      return; // Ignore duplicate reads
    }
    
    // New card detected or cooldown expired
    lastRFID = rfid;
    lastRFIDTime = currentTime;
    
    Serial.println("\n--- RFID Detected ---");
    Serial.print("RFID: ");
    Serial.println(rfid);
    
    // Beep once
    beep(100);
    
    // Blink red LED to indicate sending
    blinkRedLED(3, 150);
    
    // Send to API with driver info
    bool success = sendScanWithDriverInfo(rfid);
    
    if (success) {
      // Success: switch to green LED
      setRedLED(false);
      setGreenLED(true);
      delay(1000); // Green LED on for 1 second
      setGreenLED(false);
      setRedLED(true); // Red LED back to default
    } else {
      // Failure: blink red LED 2x fast and beep 2x
      blinkRedLED(2, 100);
      beepTimes(2, 100, 100);
      setRedLED(true); // Ensure red LED is back on
    }
    
    Serial.println("--------------------\n");
  }
}

String uidToString(uint8_t *uid, uint8_t length) {
  String result = "";
  for (uint8_t i = 0; i < length; i++) {
    if (uid[i] < 0x10) result += "0";
    result += String(uid[i], HEX);
  }
  result.toUpperCase();
  return result;
}

String getISOTimestamp() {
  time_t now = time(nullptr);
  
  // Check if time is synced (epoch should be > 2020)
  if (now < 1577836800) {  // Jan 1, 2020
    Serial.println("⚠ Time not synced, attempting to sync...");
    configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    delay(2000);
    now = time(nullptr);
  }
  
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  return String(buffer);
}

// ========================
// FIREBASE FUNCTIONS
// ========================
bool fetchDriverInfo() {
  if (!firebaseReady) {
    Serial.println("✗ Firebase not ready");
    return false;
  }

  Serial.println("--- Fetching Driver Info ---");
  
  Serial.print("Querying for deviceId: ");
  Serial.println(deviceID);
  
  // Firestore REST API endpoint for querying documents
  String url = "https://firestore.googleapis.com/v1/projects/" + FIREBASE_PROJECT_ID + 
               "/databases/(default)/documents:runQuery?key=" + FIREBASE_API_KEY;
  
  // Build query to find document where deviceId field matches
  String queryPayload = "{\"structuredQuery\":{\"from\":[{\"collectionId\":\"devices\"}],";
  queryPayload += "\"where\":{\"fieldFilter\":{\"field\":{\"fieldPath\":\"deviceId\"},";
  queryPayload += "\"op\":\"EQUAL\",\"value\":{\"stringValue\":\"" + deviceID + "\"}}},";
  queryPayload += "\"limit\":1}}";
  
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setConnectTimeout(10000);
  http.setTimeout(10000);
  
  int httpCode = http.POST(queryPayload);
  Serial.print("HTTP Code: ");
  Serial.println(httpCode);
  
  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println("Query successful");
    
    // Parse JSON response
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      // Check if we got results
      if (doc[0]["document"]) {
        JsonObject fields = doc[0]["document"]["fields"];
        
        // Extract driver info from Firestore document structure
        if (fields["driverName"]["stringValue"]) {
          driverName = fields["driverName"]["stringValue"].as<String>();
          Serial.print("Name: ");
          Serial.println(driverName);
        }
        
        if (fields["driverLicenseNo"]["stringValue"]) {
          driverLicenseNo = fields["driverLicenseNo"]["stringValue"].as<String>();
          Serial.print("License No: ");
          Serial.println(driverLicenseNo);
        }
        
        if (fields["driverPhone"]["stringValue"]) {
          driverPhone = fields["driverPhone"]["stringValue"].as<String>();
          Serial.print("Phone: ");
          Serial.println(driverPhone);
        }
        
        if (fields["driverEmail"]["stringValue"]) {
          driverEmail = fields["driverEmail"]["stringValue"].as<String>();
          Serial.print("Email: ");
          Serial.println(driverEmail);
        }
        
        // Check if we got any driver info
        if (driverName.length() > 0) {
          driverInfoLoaded = true;
          Serial.println("✓ Driver info fetched successfully");
          Serial.println("---------------------------");
          http.end();
          return true;
        }
      } else {
        Serial.println("✗ No matching document found");
      }
    } else {
      Serial.print("JSON Parse Error: ");
      Serial.println(error.c_str());
    }
  } else if (httpCode == 404) {
    Serial.println("✗ Document not found");
  } else {
    Serial.print("✗ HTTP Error: ");
    Serial.println(http.errorToString(httpCode));
    String response = http.getString();
    if (response.length() > 0) {
      Serial.print("Response: ");
      Serial.println(response);
    }
  }
  
  http.end();
  Serial.println("✗ No driver registered for this device ID");
  Serial.println("Please register driver info in Firestore");
  Serial.println("Make sure deviceId field matches: " + deviceID);
  Serial.println("Expected fields: driverName, driverLicenseNo, driverPhone, driverEmail");
  Serial.println("---------------------------");
  driverInfoLoaded = false;
  return false;
}

bool sendScanWithDriverInfo(String rfid) {
  if (!firebaseReady) {
    Serial.println("✗ Cannot send: Firebase not ready");
    return false;
  }

  Serial.println("--- Sending Scan with Driver Info ---");
  
  // Refresh driver info if not loaded
  if (!driverInfoLoaded) {
    Serial.println("⚠ Driver info not loaded, attempting to fetch...");
    fetchDriverInfo();
  }
  
  // Use standard Firestore document creation with timestamp
  String url = "https://firestore.googleapis.com/v1/projects/" + FIREBASE_PROJECT_ID + 
               "/databases/(default)/documents/scans?key=" + FIREBASE_API_KEY;
  
  // Get current timestamp
  String timestamp = getISOTimestamp();
  Serial.print("Timestamp: ");
  Serial.println(timestamp);
  
  // Build Firestore document JSON
  String payload = "{\"fields\":{";
  payload += "\"deviceId\":{\"stringValue\":\"" + deviceID + "\"},";
  payload += "\"rfid\":{\"stringValue\":\"" + rfid + "\"},";
  payload += "\"status\":{\"stringValue\":\"pending\"},";
  payload += "\"timestamp\":{\"timestampValue\":\"" + timestamp + "\"}";
  
  // Add driver info if available
  if (driverInfoLoaded) {
    payload += ",\"driverName\":{\"stringValue\":\"" + driverName + "\"}";
    payload += ",\"driverLicenseNo\":{\"stringValue\":\"" + driverLicenseNo + "\"}";
    payload += ",\"driverPhone\":{\"stringValue\":\"" + driverPhone + "\"}";
    payload += ",\"driverEmail\":{\"stringValue\":\"" + driverEmail + "\"}";
    Serial.println("✓ Including driver info in scan");
  } else {
    payload += ",\"driverName\":{\"stringValue\":\"Unknown\"}";
    payload += ",\"driverLicenseNo\":{\"stringValue\":\"Not Registered\"}";
    Serial.println("⚠ Sending without driver info (not registered)");
  }
  
  payload += "}}";
  
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setConnectTimeout(10000);
  http.setTimeout(10000);
  
  int httpCode = http.POST(payload);
  Serial.print("HTTP Code: ");
  Serial.println(httpCode);
  
  bool success = false;
  if (httpCode == 200 || httpCode == 201) {
    Serial.println("✓ Scan sent to Firestore successfully");
    String response = http.getString();
    Serial.print("Response: ");
    Serial.println(response);
    success = true;
  } else {
    Serial.println("✗ Failed to send scan");
    Serial.print("Error: ");
    Serial.println(http.errorToString(httpCode));
    String response = http.getString();
    if (response.length() > 0) {
      Serial.print("Response: ");
      Serial.println(response);
    }
  }
  
  http.end();
  Serial.println("---------------------------");
  return success;
}

bool sendToAPI(String rfid) {
  if (!WiFi.isConnected()) {
    Serial.println("✗ Cannot send to Firebase: No WiFi connection");
    return false;
  }

  Serial.println("--- Sending to Firebase ---");
  
  // Firebase Firestore REST API endpoint
  String firebaseURL = "https://firestore.googleapis.com/v1/projects/" + FIREBASE_PROJECT_ID + 
                       "/databases/(default)/documents/scans?key=" + FIREBASE_API_KEY;
  
  Serial.print("Firebase URL: ");
  Serial.println(firebaseURL);
  
  HTTPClient http;
  http.begin(firebaseURL);
  http.addHeader("Content-Type", "application/json");
  http.setConnectTimeout(10000);
  http.setTimeout(10000);
  
  // Create Firestore document structure
  // Note: timestamp will be added by Firestore server
  String payload = "{\"fields\": {";
  payload += "\"deviceId\": {\"stringValue\": \"" + deviceID + "\"},";
  payload += "\"rfid\": {\"stringValue\": \"" + rfid + "\"},";
  payload += "\"status\": {\"stringValue\": \"pending\"}";
  payload += "}}";
  
  Serial.print("Payload: ");
  Serial.println(payload);
  
  int httpCode = http.POST(payload);
  
  Serial.print("HTTP Response Code: ");
  Serial.println(httpCode);
  
  bool success = false;
  
  if (httpCode > 0) {
    String response = http.getString();
    Serial.print("Response: ");
    Serial.println(response);
    
    if (httpCode == 200 || httpCode == 201) {
      Serial.println("✓ Data sent to Firebase successfully");
      success = true;
    } else {
      Serial.println("✗ Firebase returned error");
    }
  } else {
    Serial.print("✗ HTTP Error: ");
    Serial.println(http.errorToString(httpCode));
  }
  
  http.end();
  Serial.println("---------------------------");
  return success;
}

// ========================
// UTILITY FUNCTIONS
// ========================
void beep(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}

void beepTimes(int times, int duration, int delayBetween) {
  for (int i = 0; i < times; i++) {
    beep(duration);
    if (i < times - 1) {
      delay(delayBetween);
    }
  }
}

void setRedLED(bool state) {
  digitalWrite(RED_LED_PIN, state ? HIGH : LOW);
}

void setGreenLED(bool state) {
  digitalWrite(GREEN_LED_PIN, state ? HIGH : LOW);
}

void blinkRedLED(int times, int duration) {
  for (int i = 0; i < times; i++) {
    setRedLED(false);
    delay(duration);
    setRedLED(true);
    if (i < times - 1) {
      delay(duration);
    }
  }
}

// ========================
// WEB PAGE
// ========================
String getWebPage() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>L.I.G.T.A.S RFID Configuration</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      padding: 20px;
      display: flex;
      justify-content: center;
      align-items: center;
    }
    .container {
      background: white;
      border-radius: 15px;
      box-shadow: 0 20px 60px rgba(0,0,0,0.3);
      max-width: 600px;
      width: 100%;
      padding: 30px;
    }
    h1 {
      color: #667eea;
      text-align: center;
      margin-bottom: 10px;
      font-size: 28px;
    }
    .subtitle {
      text-align: center;
      color: #666;
      margin-bottom: 30px;
      font-size: 14px;
    }
    .status {
      background: #f0f0f0;
      padding: 15px;
      border-radius: 8px;
      margin-bottom: 25px;
    }
    .status-row {
      display: flex;
      justify-content: space-between;
      margin-bottom: 8px;
      font-size: 14px;
    }
    .status-label {
      color: #666;
      font-weight: 600;
    }
    .status-value {
      color: #333;
      font-family: monospace;
    }
    .connected { color: #22c55e; font-weight: bold; }
    .disconnected { color: #ef4444; font-weight: bold; }
    .form-group {
      margin-bottom: 20px;
    }
    label {
      display: block;
      margin-bottom: 8px;
      color: #333;
      font-weight: 600;
      font-size: 14px;
    }
    input {
      width: 100%;
      padding: 12px;
      border: 2px solid #e0e0e0;
      border-radius: 8px;
      font-size: 14px;
      transition: border-color 0.3s;
    }
    input:focus {
      outline: none;
      border-color: #667eea;
    }
    .btn {
      width: 100%;
      padding: 14px;
      border: none;
      border-radius: 8px;
      font-size: 16px;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.3s;
      margin-bottom: 10px;
    }
    .btn-primary {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
    }
    .btn-primary:hover {
      transform: translateY(-2px);
      box-shadow: 0 10px 20px rgba(102, 126, 234, 0.3);
    }
    .btn-secondary {
      background: #6c757d;
      color: white;
    }
    .btn-secondary:hover {
      background: #5a6268;
    }
    .btn-danger {
      background: #ef4444;
      color: white;
    }
    .btn-danger:hover {
      background: #dc2626;
    }
    .message {
      padding: 12px;
      border-radius: 8px;
      margin-bottom: 20px;
      display: none;
      font-size: 14px;
    }
    .message.success {
      background: #d1fae5;
      color: #065f46;
      border: 1px solid #6ee7b7;
    }
    .message.error {
      background: #fee2e2;
      color: #991b1b;
      border: 1px solid #fca5a5;
    }
    .section-title {
      color: #667eea;
      font-size: 18px;
      margin: 25px 0 15px 0;
      padding-bottom: 8px;
      border-bottom: 2px solid #e0e0e0;
    }
    .help-text {
      font-size: 12px;
      color: #666;
      margin-top: 5px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🔐 L.I.G.T.A.S</h1>
    <div class="subtitle">RFID System Configuration</div>
    
    <div id="message" class="message"></div>
    
    <div class="status">
      <div class="status-row">
        <span class="status-label">Date:</span>
        <span class="status-value" id="currentDate">Loading...</span>
      </div>
      <div class="status-row">
        <span class="status-label">Time:</span>
        <span class="status-value" id="currentTime">Loading...</span>
      </div>
      <div class="status-row">
        <span class="status-label">Device ID:</span>
        <span class="status-value" id="currentDeviceID">Loading...</span>
      </div>
      <div class="status-row">
        <span class="status-label">Firebase Project:</span>
        <span class="status-value" id="firebaseProject">Loading...</span>
      </div>
      <div class="status-row">
        <span class="status-label">Firebase Status:</span>
        <span class="status-value" id="firebaseStatus">Loading...</span>
      </div>
      <div class="status-row">
        <span class="status-label">Driver Info:</span>
        <span class="status-value" id="driverInfo">Loading...</span>
      </div>
      <div class="status-row">
        <span class="status-label">WiFi Status:</span>
        <span class="status-value" id="wifiStatus">Loading...</span>
      </div>
      <div class="status-row">
        <span class="status-label">WiFi IP:</span>
        <span class="status-value" id="wifiIP">-</span>
      </div>
      <div class="status-row">
        <span class="status-label">AP IP:</span>
        <span class="status-value" id="apIP">Loading...</span>
      </div>
    </div>
    
    <form id="configForm">
      <div class="section-title">Device Settings</div>
      
      <div class="form-group">
        <label for="deviceID">Device ID</label>
        <input type="text" id="deviceID" name="deviceID" placeholder="DVC0001" required>
        <div class="help-text">Unique identifier for this device</div>
      </div>
      
      <div class="section-title">WiFi Settings</div>
      
      <div class="form-group">
        <label for="wifiSSID">WiFi SSID</label>
        <input type="text" id="wifiSSID" name="wifiSSID" placeholder="Your WiFi Name">
        <div class="help-text">Required for sending data to Firebase</div>
      </div>
      
      <div class="form-group">
        <label for="wifiPassword">WiFi Password</label>
        <input type="password" id="wifiPassword" name="wifiPassword" placeholder="Your WiFi Password">
      </div>
      
      <div class="section-title">Driver Information</div>
      
      <div class="form-group">
        <label>Driver Name</label>
        <input type="text" id="driverNameDisplay" disabled>
      </div>
      
      <div class="form-group">
        <label>License Number</label>
        <input type="text" id="driverLicenseDisplay" disabled>
      </div>
      
      <div class="form-group">
        <label>Phone</label>
        <input type="text" id="driverPhoneDisplay" disabled>
      </div>
      
      <div class="form-group">
        <label>Email</label>
        <input type="text" id="driverEmailDisplay" disabled>
      </div>
      
      <div class="help-text" style="margin-bottom: 15px;">⚠️ Driver info must be registered in Firebase at /drivers/{deviceID}</div>
      
      <button type="submit" class="btn btn-primary">💾 Save Settings</button>
      <button type="button" class="btn btn-secondary" onclick="refreshDriverInfo()">🔄 Refresh Driver Info</button>
      <button type="button" class="btn btn-secondary" onclick="syncTime()">🕐 Sync Time</button>
      <button type="button" class="btn btn-secondary" onclick="testBuzzer()">🔔 Test Buzzer</button>
      <button type="button" class="btn btn-danger" onclick="restartDevice()">🔄 Restart Device</button>
    </form>
  </div>

  <script>
    // Update time display
    async function updateTime() {
      try {
        const response = await fetch('/api/time');
        const data = await response.json();
        
        document.getElementById('currentDate').textContent = data.date;
        document.getElementById('currentTime').textContent = data.time;
        
        if (data.timeSynced) {
          document.getElementById('currentTime').className = 'status-value connected';
        } else {
          document.getElementById('currentTime').className = 'status-value disconnected';
        }
      } catch (error) {
        document.getElementById('currentDate').textContent = 'Error loading';
        document.getElementById('currentTime').textContent = '--:--:--';
        document.getElementById('currentTime').className = 'status-value disconnected';
      }
    }
    
    // Load current settings
    async function loadSettings() {
      try {
        const response = await fetch('/api/settings');
        const data = await response.json();
        
        document.getElementById('deviceID').value = data.deviceID;
        document.getElementById('wifiSSID').value = data.wifiSSID;
        
        document.getElementById('currentDeviceID').textContent = data.deviceID;
        document.getElementById('firebaseProject').textContent = data.firebaseProject || 'studious-bit-420613';
        document.getElementById('firebaseStatus').textContent = data.firebaseReady ? 'Connected' : 'Not Connected';
        document.getElementById('firebaseStatus').className = 'status-value ' + (data.firebaseReady ? 'connected' : 'disconnected');
        
        const driverStatus = data.driverInfoLoaded ? data.driverName : 'Not Registered';
        document.getElementById('driverInfo').textContent = driverStatus;
        document.getElementById('driverInfo').className = 'status-value ' + (data.driverInfoLoaded ? 'connected' : 'disconnected');
        
        document.getElementById('wifiStatus').textContent = data.wifiConnected ? 'Connected' : 'Disconnected';
        document.getElementById('wifiStatus').className = 'status-value ' + (data.wifiConnected ? 'connected' : 'disconnected');
        document.getElementById('wifiIP').textContent = data.wifiConnected ? data.wifiIP : '-';
        document.getElementById('apIP').textContent = data.apIP;
        
        // Display driver info
        document.getElementById('driverNameDisplay').value = data.driverName || 'Not registered';
        document.getElementById('driverLicenseDisplay').value = data.driverLicenseNo || 'Not registered';
        document.getElementById('driverPhoneDisplay').value = data.driverPhone || 'Not registered';
        document.getElementById('driverEmailDisplay').value = data.driverEmail || 'Not registered';
      } catch (error) {
        showMessage('Failed to load settings', 'error');
      }
    }

    // Save settings
    document.getElementById('configForm').addEventListener('submit', async (e) => {
      e.preventDefault();
      
      const formData = {
        deviceID: document.getElementById('deviceID').value,
        wifiSSID: document.getElementById('wifiSSID').value,
        wifiPassword: document.getElementById('wifiPassword').value
      };
      
      try {
        const response = await fetch('/api/settings', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(formData)
        });
        
        const result = await response.json();
        
        if (result.success) {
          showMessage('Settings saved successfully!' + (result.needRestart ? ' Please restart the device.' : ''), 'success');
          setTimeout(loadSettings, 1000);
        } else {
          showMessage('Failed to save settings', 'error');
        }
      } catch (error) {
        showMessage('Error saving settings', 'error');
      }
    });

    // Sync time
    async function syncTime() {
      try {
        showMessage('Syncing time...', 'success');
        const response = await fetch('/api/sync-time', { method: 'POST' });
        const result = await response.json();
        
        if (result.success) {
          showMessage('Time synced successfully!', 'success');
          setTimeout(updateTime, 500);
        } else {
          showMessage(result.error || 'Failed to sync time. Check WiFi connection.', 'error');
        }
      } catch (error) {
        showMessage('Failed to sync time', 'error');
      }
    }
    
    // Refresh driver info
    async function refreshDriverInfo() {
      try {
        const response = await fetch('/api/refresh-driver', { method: 'POST' });
        const result = await response.json();
        
        if (result.success) {
          showMessage('Driver info refreshed: ' + result.driverName, 'success');
          setTimeout(loadSettings, 500);
        } else {
          showMessage('Driver info not found. Please register in Firebase.', 'error');
        }
      } catch (error) {
        showMessage('Failed to refresh driver info', 'error');
      }
    }

    // Test buzzer
    async function testBuzzer() {
      try {
        await fetch('/api/beep', { method: 'POST' });
        showMessage('Buzzer test sent!', 'success');
      } catch (error) {
        showMessage('Failed to test buzzer', 'error');
      }
    }

    // Restart device
    async function restartDevice() {
      if (!confirm('Are you sure you want to restart the device?')) return;
      
      try {
        await fetch('/api/restart', { method: 'POST' });
        showMessage('Device is restarting... Please wait 10 seconds.', 'success');
      } catch (error) {
        // Expected to fail as device restarts
        showMessage('Device is restarting... Please wait 10 seconds.', 'success');
      }
    }

    // Show message
    function showMessage(text, type) {
      const msgEl = document.getElementById('message');
      msgEl.textContent = text;
      msgEl.className = 'message ' + type;
      msgEl.style.display = 'block';
      
      setTimeout(() => {
        msgEl.style.display = 'none';
      }, 5000);
    }

    // Load settings and time on page load
    loadSettings();
    updateTime();
    
    // Update time every second
    setInterval(updateTime, 1000);
  </script>
</body>
</html>
  )rawliteral";
}