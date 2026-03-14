// Import WiFi and HTTP libraries
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Add your Wi-Fi credentials
const char* ssid = "the-leg";
const char* password = "12345678";

// Supabase credentials
const char* supabaseUrl = "https://sefoaaasnyorakarqsrm.supabase.co";
const char* supabaseKey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InNlZm9hYWFzbnlvcmFrYXJxc3JtIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NzM0MTE2NDYsImV4cCI6MjA4ODk4NzY0Nn0.mfGpP7xW3b5rHXFtck1ujOGvyHUbiMaWoN8vhdru8Vg";

// How often to poll (ms)
const unsigned long POLL_INTERVAL = 3000;
unsigned long lastPoll = 0;

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
bool submitTime(long rowId, long timeMs) {
  HTTPClient http;
  String url = String(supabaseUrl) + "/rest/v1/a_leg?id=eq." + String(rowId);

  http.begin(url);
  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", String("Bearer ") + supabaseKey);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Prefer", "return=minimal");

  String body = String("{\"time\":") + timeMs + "}";
  int httpCode = http.sendRequest("PATCH", body);

  http.end();
  return (httpCode == 200 || httpCode == 204);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Wi-Fi connected!");
}

void loop() {
  unsigned long now = millis();
  if (now - lastPoll < POLL_INTERVAL) return;
  lastPoll = now;

  long armedId;
  String armedUser;

  if (checkArmedLeg(armedId, armedUser)) {
    Serial.printf("Leg armed! id=%ld user=%s\n", armedId, armedUser.c_str());

    // TODO: replace this with your actual sensor measurement
    long measuredTimeMs = 1234;

    if (submitTime(armedId, measuredTimeMs)) {
      Serial.printf("Time submitted: %ldms\n", measuredTimeMs);
    } else {
      Serial.println("Failed to submit time.");
    }
  } else {
    Serial.println("No armed leg.");
  }
}