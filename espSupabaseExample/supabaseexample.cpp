// Import WiFi and HTTP libraries
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Add your Wi-Fi credentials
const char* ssid = "YOUR SSID";
const char* password = "YOUR PASSWORD";

// Supabase credentials
const char* supabaseUrl = "https://sefoaaasnyorakarqsrm.supabase.co";
const char* supabaseKey = "SUPABASE_ANON_KEY";

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
  int httpCode = http.GET();

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

// Submit the measured time via RPC — updates whichever row currently has time=NULL.
// Robust against re-arming: if the leg was re-armed between poll and submit,
// the new armed row gets the time instead of failing silently.
bool submitTime(long timeMs) {
  HTTPClient http;
  String url = String(supabaseUrl) + "/rest/v1/rpc/submit_leg_time";

  http.begin(url);
  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", String("Bearer ") + supabaseKey);
  http.addHeader("Content-Type", "application/json");

  String body = String("{\"p_time\":") + timeMs + "}";
  int httpCode = http.POST(body);

  String payload = http.getString();
  http.end();

  // RPC returns true if a row was updated, false if no armed leg existed anymore
  return (httpCode == 200 && payload == "true");
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

    if (submitTime(measuredTimeMs)) {
      Serial.printf("Time submitted: %ldms\n", measuredTimeMs);
    } else {
      Serial.println("Failed to submit time.");
    }
  } else {
    Serial.println("No armed leg.");
  }
}
