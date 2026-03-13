// Import WiFi and ESPSupabase Library
#include <WiFi.h>
#include <ESPSupabase.h>

// Add you Wi-Fi credentials
const char* ssid = "YOUR SSID";
const char* password = "YOUR PASSWORD";

// Supabase credentials
const char* supabaseUrl = "https://sefoaaasnyorakarqsrm.supabase.co/rest/v1/a_leg";
const char* supabaseKey = "SUPABASE_ANON_KEY";

Supabase supabase;

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Wi-Fi connected!");

  // Init Supabase
  supabase.begin(supabaseUrl, supabaseKey);

  // Add the table name here
  String tableName = "a_leg";
  // change the correct columns names you create in your table
  String jsonData = "{\"time\": \"7\"}";

  // sending data to supabase
  int response = supabase.insert(tableName, jsonData, false);
  if (response == 200) {
    Serial.println("Data inserted successfully!");
  } else {
    Serial.print("Failed to insert data. HTTP response: ");
    Serial.println(response);
  }
}

void loop() {
}
