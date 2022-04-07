/*
   ESP32 NET-Radio © 2020/21 P.Klingbeil Vers.2 peter.klingbeil@film-werk56.de
   Spielt http/https mp3 Audio, auch von umgelenkten URL's (301/302).

   helloMp3.h Copyright (C) 2017 Marcin Szalomski Licensed under GNU GPLv3

   Für die Verbindungssteuerung und Wiedergabe wird der Code vs1053_ext.cpp benutzt:
   © GPL3+ Lizenz https://github.com/schreibfaul1/ESP32-vs1053_ext Oct 31 2021, Author: Wolle

   HTTP MP3 MPEG Layer 1+2 Streaming

   Senderliste:

   0:Jazz Radio Berlin
   1:DLF
   2:Bremen-2
   3:DLF Kultur
   4:HR-3
   5:NDR-2
   6:Jazz aus der Schweiz
   7:Niederland Jazz NPO

   VS1053B - connections detail:

   XRST = EN (D3)
   MISO = D19
   MOSI = D23
   SCLK = D18
   VCC = 5V oder 3.3 V
   Gnd = Gnd

*/

#include "Arduino.h"
#include "WiFi.h"
#include "SPI.h"
#include "vs1053_ext.h" // New ESP_VS1053_Library!

#include <Preferences.h>  //For reading and writing into the ROM memory
Preferences preferences;

#include "helloMp3.h"

//OLED 64*128 display headers
#include "SSD1306.h"
SSD1306  display(0x3c, 21, 22);

String ssid =     "WLAN_Netzwerk";    //  your network SSID (name)
String password = "tortik92";    //  your network password

// Wiring of VS1053 board (SPI connected in a standard way)
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18
#define VS1053_CS     32
#define VS1053_DCS    33
#define VS1053_DREQ   35
#define POTENTIOMETER 34

unsigned int counter, old_counter, new_counter; // Station counter
int change = 13, volume = 12, volume2 = 14; // Taste Dig. Eingang fuer Kanal und Lautstärke +/-
bool x = true, y = true;               // Tasten Status
unsigned int VOLUME = 10;              // volume level 0-20
String x4 = "                    ";    // Station name 20 digits

VS1053 mp3(VS1053_CS, VS1053_DCS, VS1053_DREQ);  // init VS1053

void setup ()
{
  analogReadResolution(10);  // für potentiometer, genauigkeit in bit --> liefert werte von 0 -1024
  //Begin display
  display.init();
  display.invertDisplay();
  display.setFont(ArialMT_Plain_16);
  display.normalDisplay();
  display.setColor(WHITE);
  display.drawString(0, 0, "      NET Radio");
  display.setFont(ArialMT_Plain_10);

  // Digital Eingaenge festlegen
  pinMode(change, INPUT_PULLUP);
  pinMode(volume, INPUT_PULLUP);
  pinMode(volume2, INPUT_PULLUP);

  // initialize SPI
  Serial.begin(115200);
  delay(500);

  Serial.println("Display Init done..");  // Debug

  // initialize SPI bus;
  SPI.begin();
  Serial.println("SPI Bus Init done..");  // Debug

  // initialize Network
  String x1 = "Warte auf Verbindung";
  display.drawString(14, 27, x1);
  display.display();

  WiFi.disconnect();
  Serial.println("WiFi Init starting..");  // Debug
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) delay(1500);

  // initialize VS1053 player
  mp3.begin();
  mp3.setVolume(VOLUME);
  Serial.println("MP3 Player Init done..");  // Debug

  //player.switchToMp3Mode(); // Most VS1053 modules will start up in MIDI mode

  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.normalDisplay();
  display.setColor(WHITE);
  display.drawString(0, 0, "      NET Radio");
  display.setFont(ArialMT_Plain_10);

  display.setColor(WHITE);
  x1 = "Connected";
  display.drawString(35, 15, x1);

  delay(250);
  x1 = "Station";
  display.drawString(45, 27, x1);

  preferences.begin("my-app", false);
  counter = preferences.getUInt("counter", 0);
  old_counter = counter;

  Serial.printf("Current counter value: %u\n", counter);
  delay(100);

  mp3.playbuff(hello2, sizeof(hello2)); //VS1053 is wake up & running
  station_connect(counter);  // Verbinde mit Station
}

void loop()
{
  mp3.loop();  // mp3 Steuerung und Wiedergabe

   
  if (digitalRead(change) == 0 and x == true) // Sender
  {
    x = false;
    old_counter = counter;
    counter = counter + 1;
    if (counter > 7) counter = 0;
    preferences.putUInt("counter", counter);
    new_counter = counter;
    Serial.printf("Set counter to new_value: %u\n", counter);

    if (old_counter != new_counter)
    {
      //player.softReset();
      station_connect(counter);  // Verbinde mit Station
    }
  }
  //vllt auf int geben
  mp3.setVolume((int)((analogRead(POTENTIOMETER)/750.00)*20));
  //VOLUME=(analogRead(POTENTIOMETER)/700.00)*20;
  //Serial.println((int)((analogRead(POTENTIOMETER)/750.00)*20));
}

void station_connect (int station_no ) // Senderliste
{
  delay(250);
  Serial.println("Connected now");

  if (station_no == 0) x4 = " Jazz Radion Berlin "; // Display station names
  if (station_no == 1) x4 = "          DLF       ";
  if (station_no == 2) x4 = "  Radio Bremen 2    ";
  if (station_no == 3) x4 = "     DLF Kultur     ";
  if (station_no == 4) x4 = "      NDR Blue      ";
  if (station_no == 5) x4 = "         NDR 1      ";
  if (station_no == 6) x4 = " Swiss Jazz Radio   ";
  if (station_no == 7) x4 = "      NPO Jazz      ";

  x = true;
  preferences.putUInt("counter", new_counter);

  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "      NET Radio");
  display.setFont(ArialMT_Plain_10);

  String x1 = "Connected";
  display.drawString(35, 15, x1);
  x1 = "Station";
  display.drawString(45, 27, x1);
  display.drawString(20, 37, x4);
  display.display();

  switch (station_no) {   // play station index-number

    case 0:
      mp3.connecttohost("https://d1qg6pckcqcdk0.cloudfront.net/adultcontemporary/stewartrod_chrgold27_youngturks.m4a");
      break;

    case 1:
      mp3.connecttohost("https://us2.internet-radio.com/proxy/chglobal?mp=/stream;");
      break;

    case 2:
      mp3.connecttohost("http://149.13.0.80/veronika64");
      break;

    case 3:
      mp3.connecttohost("https://st02.sslstream.dlf.de/dlf/02/128/mp3/stream.mp3");
      break;

    case 4:
      mp3.connecttohost("http://icecast.ndr.de/ndr/ndrblue/live/mp3/128/stream.mp3");
      break;

    case 5:
      mp3.connecttohost("http://icecast.ndr.de/ndr/ndr1wellenord/kiel/mp3/128/stream.mp3");
      break;

    case 6:
      mp3.connecttohost("stream.srg-ssr.ch/m/rsj/mp3_128");
      break;

    case 7:
      mp3.connecttohost("https://stream.radiovlna.sk/vlna-hi.mp3");
      break;
  }
}


// Zusätzliche Infos für den Debuging Monitor

void vs1053_showstation(const char *info) {         // called from vs1053
  Serial.print("STATION:      ");
  Serial.println(info);
}

void vs1053_lasthost(const char *info) {            // really connected URL
  Serial.print("lastURL:      ");
  Serial.println(info);
}
