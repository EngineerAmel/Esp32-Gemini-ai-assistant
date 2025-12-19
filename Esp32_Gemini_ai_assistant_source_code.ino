/*
 * ESP32_AI_Connect
 * High Accuracy + High Efficiency Gemini Chat
 */

#include <WiFi.h>
#include <ESP32_AI_Connect.h>

// -----------------------------------------------------------------------------
// WiFi Credentials
// -----------------------------------------------------------------------------
const char* ssid     = "your_SSID";
const char* password = "your_PASSWORD";

// -----------------------------------------------------------------------------
// Gemini Configuration
// -----------------------------------------------------------------------------
const char* apiKey   = "your_API_Key";
const char* model    = "gemini-2.5-flash";
const char* platform = "gemini";

// Token limit (NEW – accuracy & cost control)
const char* Gemini_Max_Tokens = "100";

ESP32_AI_Connect aiClient(platform, apiKey, model);


// -----------------------------------------------------------------------------
// High-Accuracy & High-Efficiency Output Filter
// -----------------------------------------------------------------------------
String filterResponse(const String &input) {
  if (input.isEmpty()) return "No valid response received.";

  String out;
  out.reserve(input.length());

  bool space = false;
  bool newline = false;

  for (char c : input) {

    // Remove non-printable
    if (c == '\r' || !isPrintable(c)) continue;

    // Space normalization
    if (c == ' ' || c == '\t') {
      if (!space) {
        out += ' ';
        space = true;
      }
      continue;
    }
    space = false;

    // Newline normalization
    if (c == '\n') {
      if (!newline) {
        out += '\n';
        newline = true;
      }
      continue;
    }
    newline = false;

    // Allowed characters (accuracy-safe)
    if (isalnum(c) ||
        c == '.' || c == ',' ||
        c == '!' || c == '?' ||
        c == ':' || c == ';' ||
        c == '-' || c == '_') {
      out += c;
    } else {
      if (!space) {
        out += ' ';
        space = true;
      }
    }
  }

  // Remove AI disclaimers
  out.replace("As an AI language model", "");
  out.replace("As an AI model", "");
  out.replace("I am an AI model", "");

  out.trim();
  return out.isEmpty() ? "No valid response received." : out;
}


// -----------------------------------------------------------------------------
// Safe & Efficient WiFi Connection
// -----------------------------------------------------------------------------
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Connecting WiFi");
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(300);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi failed. Retrying...");
  }
}


// -----------------------------------------------------------------------------
// Reliable AI Chat (Retry Protected)
// -----------------------------------------------------------------------------
String getAIResponse(const String &msg) {
  String response;
  response.reserve(512);

  for (uint8_t i = 0; i < 2; i++) {
    response = aiClient.chat(msg);
    if (!response.isEmpty()) break;
    delay(400);
  }
  return response;
}


// -----------------------------------------------------------------------------
// Setup
// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(600);

  connectWiFi();

  // Gemini parameters (accurate + efficient)
  String params;
  params.reserve(128);
  params = "{";
  params += "\"topP\":0.9,";
  params += "\"temperature\":0.7,";
  params += "\"maxOutputTokens\":";
  params += Gemini_Max_Tokens;
  params += ",";
  params += "\"candidateCount\":1";
  params += "}";

  if (aiClient.setChatParameters(params)) {
    Serial.println("System Ready");
  } else {
    Serial.println("Using default parameters");
  }
}


// -----------------------------------------------------------------------------
// Main Loop
// -----------------------------------------------------------------------------
void loop() {
  connectWiFi();

  Serial.println("\nEnter message:");
  while (!Serial.available()) delay(30);

  String userMessage = Serial.readStringUntil('\n');
  userMessage.trim();
  if (userMessage.isEmpty()) return;

  Serial.println("Sending...");

  String raw   = getAIResponse(userMessage);
  String clean = filterResponse(raw);

  Serial.println("\nAI Response:");
  Serial.println(clean);
}
