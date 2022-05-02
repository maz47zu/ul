#include <ArduinoJson.h>


#ifndef _HIVE_
#define _HIVE_



class Hive {
  private:
    //TODO id to powinno byc String i mac
    int id;
    String motherId;
    float temperature;
    float humidity;
    float weight;
  public:
    Hive(int id, String motherId, float temperature, float humidity, float weight) {
      this->id = id;
      this->motherId = motherId;
      this->temperature = temperature;
      this->humidity = humidity;
      this->weight = weight;
    }

    String asJson() {
      StaticJsonDocument<200> doc;
      doc["id"] = id;
      doc["motherId"] = motherId;
      doc["temperature"] = temperature;
      doc["humidity"] = humidity;
      doc["weight"] = weight;
      String requestBody;
      serializeJson(doc, requestBody);
      return requestBody;
    }

};

#endif _HIVE_
