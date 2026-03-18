// Import WiFi and HTTP libraries
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Pins
const int flowPin = 13;
const int water_level_sensor_PIN = 34;

// Constants
const int LEVEL_DROP = 80;
const int NO_FLOW_TIMEOUT = 5000;

// Variables
volatile int pulseCount = 0;
float liquid_max;
bool isMeasuring = false;

// Statistics
float maxFlow = 0.0;
float sumFlow = 0.0;
int flowReadingsCount = 0;
unsigned long lastFlowTime = 0;

// Timer
unsigned long timer_start = 0;
unsigned long timer_stop = 0;

void IRAM_ATTR pulseCounter() { // pulse counter 
  pulseCount++;
}

// Add your Wi-Fi credentials
const char* ssid = "the-leg";
const char* password = "12345678";

// Supabase credentials
const char* supabaseUrl = "https://sefoaaasnyorakarqsrm.supabase.co";
const char* supabaseKey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InNlZm9hYWFzbnlvcmFrYXJxc3JtIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NzM0MTE2NDYsImV4cCI6MjA4ODk4NzY0Nn0.mfGpP7xW3b5rHXFtck1ujOGvyHUbiMaWoN8vhdru8Vg";

// How often to poll (ms)
const unsigned long POLL_INTERVAL = 3000;
unsigned long lastPoll = 0;
long currentRowId = -1;

//initialised within implementation in loop using checkArmedLeg: 
long armedId;
String armedUser;

// Check if a leg is armed (time IS NULL in a_leg table).
// Returns true and fills out_id/out_username if an armed leg is found.
bool checkArmedLeg(long& out_id, String& out_username) {
  HTTPClient http;
  String url = String(supabaseUrl) + "/rest/v1/rpc/is_leg_armed";

  http.begin(url);
  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", String("Bearer ") + supabaseKey);
  http.addHeader("Content-Type", "application/json");

  // RPC with no arguments still needs an empty JSON body on POST
  int httpCode = http.POST("{}");

  if (httpCode != 200) {
    Serial.printf("Poll failed, HTTP %d\n", httpCode);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  // Response is an array: [] if no armed leg, [{"id":5,"username":"alice"}] if armed
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err || !doc.is<JsonArray>() || doc.as<JsonArray>().size() == 0) {
    return false;
  }

  JsonObject row = doc[0];
  out_id = row["id"].as<long>();
  out_username = row["username"].as<String>();
  return true;
}

// Insert the measured time for the armed leg row (finishes the run)
bool submitTime(long rowId, long timeMs, long finalAvg) {
  HTTPClient http;
  String url = String(supabaseUrl) + "/rest/v1/a_leg?id=eq." + String(rowId);

  http.begin(url);
  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", String("Bearer ") + supabaseKey);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Prefer", "return=minimal");

  String body = String("{\"time\":") + timeMs + String(",\"avg_flow_rate\":") + finalAvg + "}"; // time in ms and flow rate in L/min 
  int httpCode = http.sendRequest("PATCH", body);

  http.end();
  return (httpCode == 200 || httpCode == 204);
}


void setup() {
  pinMode(water_level_sensor_PIN, INPUT);
  pinMode(flowPin, INPUT_PULLUP);   // Use INPUT_PULLUP for cleaner signal
  Serial.begin(9600);

  attachInterrupt(digitalPinToInterrupt(flowPin), pulseCounter, RISING);

  delay(1000); // wait a bit 
  WiFi.mode(WIFI_STA); //Optional
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(100);
  }

  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
}

int i = 0;

void resetStats(){
  maxFlow = 0;
  sumFlow = 0;
  flowReadingsCount = 0;
  lastFlowTime = millis();
}

void loop() {


  unsigned long now = millis();
  if (i == 0) {
    liquid_max = analogRead(water_level_sensor_PIN);
    i++;
  }

  int currentLevel = analogRead(water_level_sensor_PIN);

  // Level Drop Trigger: check every loop  
  if (!isMeasuring && currentRowId > 0 && (liquid_max - currentLevel) >= LEVEL_DROP) {
    Serial.println("System Triggered: Level Drop Detected.");
    isMeasuring = true;
    timer_start = millis();
    resetStats();
    return;
  }

  
  if (!isMeasuring) {

  if (now - lastPoll >= POLL_INTERVAL){
  lastPoll = now;


  if (checkArmedLeg(armedId, armedUser)) {
    Serial.printf("Leg armed! id=%ld user=%s\n", armedId, armedUser.c_str());
    currentRowId = armedId;  // wait for level drop to start measuring
  } else {
    Serial.println("No armed leg.");
    currentRowId = -1;
  }
  }
  return; // if over interval
}


if (isMeasuring) {
    static unsigned long last10ms = 0;
    static unsigned long last3s = 0;
    static float runningSum10ms = 0;
    static int sampleCount10ms = 0;

    // --- STEP 1: High-Frequency Sampling (Every 10ms) ---
    if (millis() - last10ms >= 1000) {
      // Safely snapshot and reset pulse count
      noInterrupts();
      int pulses = pulseCount;
      pulseCount = 0;
      interrupts();

      // YF-S201: 7.5 pulses per second = 1 L/min
      // pulses here are over 10ms, so scale to per-second first
      float flowRate = (pulses / 7.5) * (1000.0 / 10.0);  // L/min

      if (flowRate > 0) {
        lastFlowTime = millis();
      }
      Serial.print("Instant flow (L/min): ");
      Serial.println(flowRate);

      runningSum10ms += flowRate;
      sampleCount10ms++;
      last10ms = millis();
    }

       


        // // --- STEP 2: Mean Calculation & Reporting (Every 3 seconds) ---
        if (millis() - last3s >= 3000) {
            // This is the "Mean of the means"
            float meanFlow3s = (sampleCount10ms > 0) ? (runningSum10ms / sampleCount10ms) : 0;

            if (meanFlow3s > 0) {
                lastFlowTime = millis(); // Reset the "Still flowing" timer
                
                // Global stats for final JSON
                sumFlow += meanFlow3s;
                flowReadingsCount++;
                if (meanFlow3s > maxFlow) maxFlow = meanFlow3s;

                Serial.print("3s Mean Flow: "); Serial.println(meanFlow3s);
            }

            // Reset 3s window variables
            runningSum10ms = 0;
            sampleCount10ms = 0;
            last3s = millis();
        }

        // --- STEP 3: Stop Condition ---
        if (millis() - lastFlowTime > NO_FLOW_TIMEOUT && flowReadingsCount > 0) {
            timer_stop=millis();
            Serial.println("STOPPED");
            stopAndSendData();
        }
    }

}
void stopAndSendData(){
  isMeasuring = false;
  long finalAvg = (flowReadingsCount > 0) ? (sumFlow / flowReadingsCount) : 0; // need to send this next
  long durationMs = timer_stop - timer_start;

  Serial.printf("Submitting flow avg: %ldms\n", finalAvg);

  Serial.printf("Submitting time: %ldms\n", durationMs);

  if (currentRowId > 0 && submitTime(armedId, durationMs, finalAvg)) {
        Serial.printf("Time submitted: %ldms\n", durationMs);
      } else {
        Serial.println("Failed to submit time.");
      }
  currentRowId = -1;  // reset for next run
  }
