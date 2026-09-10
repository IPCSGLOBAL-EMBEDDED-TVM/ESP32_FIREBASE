/* Author : Adam D John */

#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include "ExampleFunctions.h"
#include <ArduinoJson.h>

#define WIFI_SSID     "ENTER YOUR WIFI SSID"
#define WIFI_PASSWORD "ENTER YOUR WIFI PASSWORD"

#define WEB_API_KEY   "ENTER YOUR API KEY"
#define DATABASE_URL  "ENTER YOUR APP URL"
#define USER_EMAIL    "ENTER YOUR MAIL  ID"
#define USER_PASS     "ENTER YOUR USER PASSWORD"

void processData(AsyncResult &aResult);

UserAuth user_auth(WEB_API_KEY, USER_EMAIL, USER_PASS);

SSL_CLIENT ssl_client;
SSL_CLIENT stream_ssl_client;

FirebaseApp app;

using AsyncClient = AsyncClientClass;

AsyncClient aClient(ssl_client);
AsyncClient streamClient(stream_ssl_client);

RealtimeDatabase Database;

String listenerPath = "board1/outputs/digital/";

const int output1 = 12;
const int output2 = 13;
const int output3 = 14;

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 10000;


void initWiFi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi connected");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}


void setup()
{
  Serial.begin(115200);

  pinMode(output1, OUTPUT);
  pinMode(output2, OUTPUT);
  pinMode(output3, OUTPUT);

  digitalWrite(output1, LOW);
  digitalWrite(output2, LOW);
  digitalWrite(output3, LOW);

  initWiFi();

  ssl_client.setInsecure();
  stream_ssl_client.setInsecure();

  ssl_client.setTimeout(1000);
  ssl_client.setBufferSizes(4096, 1024);

  stream_ssl_client.setTimeout(1000);
  stream_ssl_client.setBufferSizes(4096, 1024);

  initializeApp(
    aClient,
    app,
    getAuth(user_auth),
    processData,
    "authTask"
  );

  app.getApp<RealtimeDatabase>(Database);

  Database.url(DATABASE_URL);

  streamClient.setSSEFilters(
    "get,put,patch,keep-alive,cancel,auth_revoked"
  );

  Database.get(
    streamClient,
    listenerPath,
    processData,
    true,
    "streamTask"
  );

  Serial.println("Firebase stream started");
}


void loop()
{
  app.loop();

  if (app.ready())
  {
    unsigned long currentTime = millis();

    if (currentTime - lastSendTime >= sendInterval)
    {
      lastSendTime = currentTime;

      Serial.printf(
        "Program running for %lu ms\n",
        currentTime
      );
    }
  }
}


void processData(AsyncResult &aResult)
{
  if (!aResult.isResult())
    return;

  if (aResult.isEvent())
  {
    Firebase.printf(
      "Event task: %s, msg: %s, code: %d\n",
      aResult.uid().c_str(),
      aResult.eventLog().message().c_str(),
      aResult.eventLog().code()
    );
  }

  if (aResult.isDebug())
  {
    Firebase.printf(
      "Debug task: %s, msg: %s\n",
      aResult.uid().c_str(),
      aResult.debug().c_str()
    );
  }

  if (aResult.isError())
  {
    Firebase.printf(
      "Error task: %s, msg: %s, code: %d\n",
      aResult.uid().c_str(),
      aResult.error().message().c_str(),
      aResult.error().code()
    );
  }

  if (aResult.available())
  {
    RealtimeDatabaseResult &RTDB =
      aResult.to<RealtimeDatabaseResult>();

    if (RTDB.isStream())
    {
      Serial.println("----------------------------");

      Firebase.printf(
        "Task: %s\n",
        aResult.uid().c_str()
      );

      Firebase.printf(
        "Event: %s\n",
        RTDB.event().c_str()
      );

      Firebase.printf(
        "Path: %s\n",
        RTDB.dataPath().c_str()
      );

      Firebase.printf(
        "Data: %s\n",
        RTDB.to<const char *>()
      );

      Firebase.printf(
        "Type: %d\n",
        RTDB.type()
      );


      // Initial JSON data
      if (RTDB.type() == 6)
      {
        DynamicJsonDocument doc(1024);

        String jsonData = RTDB.to<String>();

        DeserializationError error =
          deserializeJson(doc, jsonData);

        if (error)
        {
          Serial.print("JSON error: ");
          Serial.println(error.c_str());
          return;
        }

        for (JsonPair kv : doc.as<JsonObject>())
        {
          int gpioPin =
            atoi(kv.key().c_str());

          bool state =
            kv.value().as<bool>();

          if (gpioPin == output1 ||
              gpioPin == output2 ||
              gpioPin == output3)
          {
            digitalWrite(
              gpioPin,
              state ? HIGH : LOW
            );

            Serial.printf(
              "GPIO %d -> %s\n",
              gpioPin,
              state ? "ON" : "OFF"
            );
          }
        }
      }


      // Single GPIO update
      if (RTDB.type() == 4 ||
          RTDB.type() == 1)
      {
        int GPIO_number =
          RTDB.dataPath().substring(1).toInt();

        bool state =
          RTDB.to<bool>();

        if (GPIO_number == output1 ||
            GPIO_number == output2 ||
            GPIO_number == output3)
        {
          digitalWrite(
            GPIO_number,
            state ? HIGH : LOW
          );

          Serial.printf(
            "GPIO %d -> %s\n",
            GPIO_number,
            state ? "ON" : "OFF"
          );
        }
      }
    }
    else
    {
      Serial.printf(
        "Task: %s, payload: %s\n",
        aResult.uid().c_str(),
        aResult.c_str()
      );
    }
  }
}