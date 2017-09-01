/*********
 TODO configurer la sensibilité depuis le site web
 TODO configurer la puissance de l'arosage depuis le site web
*********/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;
const uint8_t pin_moist = 17;

#include <SimpleDHT.h>
int pinDHT22 = 16;
SimpleDHT22 dht22;

MDNSResponder mdns;

// Replace with your network credentials
const char* ssid = "NONNON2";
const char* password = "ceciest1acceswifi";

ESP8266WebServer server(80);

int moisture;
float airmoisture;
float temperature;
int puissance = 0;
int sensibilite = 670;
int intervalle = 1000 * 15;
int refreshtime = 1000;
int tpsprec1 = 0;
int tpsprec2 = 0;

void setup(void){

  // hardware
  pinMode(12, OUTPUT);
  Wire.begin();
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(System5x7);
  oled.clear();
  oled.print("Setup OK");
  //

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (mdns.begin("arroseur", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

String getPage(){
  String page = "<html lang='fr'><head><meta http-equiv='refresh' content='15' name='viewport' content='width=device-width, initial-scale=1'/>";
  page += "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css'><script src='https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js'></script><script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js'></script>";
  page += "<title>Arroseur en ligne</title></head><body>";
  page += "<div class='container-fluid'>";
  page += "<div class='row'>";
  page += "<div class='col-md-12'>";
  page += "<h1>Arroseur - o - matic</h1>";
  page += "<table class='table'>";
  page += "<thead><tr><th>Capteur</th><th>Valeur</th></tr></thead>";
  page += "<tbody>";
  page += "<tr><td>Humidit&eacute sol</td><td>";
  page += moisture;
  page += "</td></tr>";
  page += "<tr><td>Humidit&eacute air</td><td>";
  page += airmoisture;
  page += "%</td></tr>";
  page += "<tr><td>Temp&eacuterature</td><td>";
  page += temperature;
  page += "&degC</td></tr>";
  page += "</tbody>";
  page += "</table>";
  page += "<form action='/' method='POST'>";
  page += "<button class='btn btn-success btn-lg btn-block' type='button submit' value='1' name='arroser'>Arroser</button>";
  page += "</form>";
  page += "</div>";
  page += "</div>";
  page += "</div>";
  // textbox
  page += "<div class = 'row'>";
  page += "<div class='text-center'>";
  page += "<div class ='col-sm-3'>";
  page += "<form action='/' method='POST'>";
  page += "<input class ='form-control' type='text' name='puistextbox'>";
  page += "</div>";

  page += "<div class='col-sm-3'>";
  page += "<button class ='btn btn-info btn-large' type='button submit'>Puissance</button>";
  page += "</div>";
  page += "</form>";

  page += "<div class ='col-sm-3'>";
  page += "<form action='/' method='POST'>";
  page += "<input class ='form-control' type='text' name='sensitextbox'>";
  page += "</div>";

  page += "<div class='col-sm-3'>";
  page += "<button class ='btn btn-info btn-large' type='button submit'>Sensibilite</button>";
  page += "</div>";
  page += "</form>";
  page += "</div>";

  page += "</div></body></html>";
  return page;
}

void handleRoot() {
  if (server.hasArg("arroser")){
    handleArroser();
  }
  if (server.hasArg("puistextbox")){
    puissance = server.arg("puistextbox").toInt();
  }
  if (server.hasArg("sensitextbox")){
    sensibilite = server.arg("sensitextbox").toInt();
  }
  server.send(600, "text/html", getPage());
}

void handleArroser(){
  Serial.print("Arroser!!");
  analogWrite(12,puissance);
  delay(1000);
  analogWrite(12,0);
}

void loop(void){
  server.handleClient();

  unsigned long tpscour = millis();

  // Lecture humidite
  moisture = analogRead(pin_moist);
  if (tpscour - tpsprec1 > refreshtime)
  {
    tpsprec1 = tpscour;
    // Affichage
    oled.clear();
    oled.print("Humidite sol :");
    oled.println(moisture);
    oled.print("Humidite air :");
    oled.print(airmoisture);
    oled.println("%");
    oled.print("Temperature :");
    oled.print(temperature);
    oled.println("C");
    oled.print("Puissance :");
    oled.println(puissance);
    oled.print("Sensibilite :");
    oled.println(sensibilite);
    oled.print("IP :");
    oled.println(WiFi.localIP());
  }

  // Arrosage auto
  if (tpscour - tpsprec2 > intervalle)
  {
    tpsprec2 = tpscour;
    // Lecture DHT11 (frequence max pour dht11 : 1,5Hz)
    int err = SimpleDHTErrSuccess;
    if ((err = dht22.read2(pinDHT22, &temperature, &airmoisture, NULL)) != SimpleDHTErrSuccess) {
      Serial.print("Read DHT11 failed, err="); Serial.println(err);delay(1000);
      oled.println("Erreur DHT");
      return;
    }

    oled.println("Arrosage!");
    analogWrite(12, puissance);
    delay(4000);
    analogWrite(12, 0);
  }
}
