#ifndef _CLIENTWRAPPER_
#define _CLIENTWRAPPER_

#include <WiFi.h>
#include <HTTPClient.h>
#include "Hive.h"


class ClientWrapper {
  private:
    String url;

  public:
    ClientWrapper() {
      ;
    }

    void init(String ssid, String password, String url) {
      this->url = url;
      WiFi.begin(ssid.c_str(), password.c_str());
      Serial.println("Connecting");
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("");
      Serial.print("Connected to WiFi network with IP Address: ");
      Serial.println(WiFi.localIP());
    }

    //TODO nie void
    void updateSingleHive(Hive hive) {
      HTTPClient http;
      http.begin(url + "hives/update");
      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST(hive.asJson());
    }
};

#endif _CLIENTWRAPPER_
