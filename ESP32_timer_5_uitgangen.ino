/*
* MIT License
*
* Copyright (c) 2024 thieu-b55
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

/*
Libraries
    *** USE THESE LIBRARIES ***
    *
    * AsyncTCP            https://github.com/me-no-dev/AsyncTCP
    * ESPAAsyncWebServer  https://github.com/me-no-dev/ESPAsyncWebServer
    * 
    * Also use the latest ESP32 arduino core by Espressif Systems  >>> see Board manager 
    * At this moment (09/09/2024) 3.0.3 
    * 
    * *************************
*/
/*
 * GPIO21 >> SDA   
 * GPIO22 >> SCL
 */


#include "Arduino.h"
#include "WiFi.h"
#include "FS.h"
#include <LittleFS.h>
#include "Wire.h"
#include <AsyncTCP.h>  
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

#define FORMAT_LITTLEFS_IF_FAILED       true
#define CONFIG_LITTLEFS_SPIFFS_COMPAT   1

#define DS3231SN                        0x68

#define OUTPUT_0                        13
#define OUTPUT_1                        18
#define OUTPUT_2                        19
#define OUTPUT_3                        16
#define OUTPUT_4                        17

#define MINUUT_PULS                     39

#define INPUT_0                         23
#define INPUT_1                         25
#define INPUT_2                         26
#define INPUT_3                         27
#define INPUT_4                         32

#define AKTIEF                           1
#define MAANDAG                          1
#define ZATERDAG                         6
#define ZONDAG                           7
#define DAGELIJKS                        8
#define WEEKDAGEN                        9
#define WEEKEND                         10

void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void testFile(fs::FS &fs, const char * path);
byte dec_naar_bcd(byte waarde);
byte bcd_naar_dec(byte waarde);
void schrijf_DS3231SN();
void lees_DS3231SN();
void minuut_voorbij();
void input_inlezen();
void output_schrijven();
void html_input();

bool minuut_interrupt_bool = false;

char dag_tijd_char[25];
char tijd_char[15];
char uitschakel_char[15];
char schakel_char[15];
char lees_char[15];
char module_tijd_char[15];
char schrijf_char[35];
char waarde_char[15];

const char* APSSID = "ESP32Timers";
const char* APPSWD = "ESP32pswd";
const char* weekdagen_char[] = {"dummy", "Maandag", "Dinsdag", "Woensdag", "Donderdag", "Vrijdag", "Zaterdag", "Zondag", "Dagelijks", "Weekdagen", "Weekend"};
const char* TIMER_MIN = "timer_min";
const char* TIMER_PLUS = "timer_plus";
const char* MODULE_PLUS = "module_plus";
const char* MODULE_MIN  = "module_min";
const char* MODULE_ = "module_";
const char* AKTIEF_ = "aktief_";
const char* INGANG_ = "ingang_";
const char* DAG_ = "dag_";
const char* TIJD_ = "tijd_";
const char* DUURTIJD_ = "duurtijd_";
const char* TIMER_BEVESTIG = "timer_bevestig";
const char* INSTEL_DAG_ = "instel_dag_";
const char* INSTEL_TIJD_ ="instel_tijd_";
const char* TIJD_BEVESTIG = "tijd_bevestig";

int seconde_int;
int minuut_int;
int uur_int;
int dag_int;
int timer_teller_int = 0;
int module_teller_int = 0;
int programma_teller_int = 0;
int aktief_int = 0;
int ingang_int = 0;
int timer_dag_int = 0;
int timer_uur_int = 0;
int timer_minuten_int = 0;
int duurtijd_int = 0;
int instel_dag_int = 0;
int instel_uur_int = 0;
int instel_minuut_int = 0;
int schakel_uur_int = 0;
int schakel_minuten_int = 0;
int timer_teller = 0;
int module_teller = 0;
/*
 * instellingen array
 *  5 outputs
 *  7 programma's (modules) per output
 *  module opbouw
 *  positie 0     module nummer
 *  positie 1     indien 1 >> aktief
 *  positie 2     waarde die ingang moet hebben om module te kunne starten
 *  positie 3     schakeldag      
 *  positie 4     schakeluur      
 *  positie 5     schakelminuut   
 *  positie 6     0 >> uitschakelen na xx minuten / 1 >> uitschakelen hh:mm
 *  positie 7     uitschakeluur bij positie 6 == 1
 *  positie 8     uitschakeltijd na xx minuten bij positie 6 == 0 
 *                uitschakelminuten bij positie 6 == 1
 *                
 */
int instellingen_array[5][7][9] = {{{0, 0, 0, 0, 0, 0, 0, 0, 0},  
                                    {1, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {2, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {3, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {4, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {5, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {6, 0, 0, 0, 0, 0, 0, 0, 0}},
                                   {{0, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {1, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {2, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {3, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {4, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {5, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {6, 0, 0, 0, 0, 0, 0, 0, 0}},
                                   {{0, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {1, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {2, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {3, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {4, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {5, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {6, 0, 0, 0, 0, 0, 0, 0, 0}},
                                   {{0, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {1, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {2, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {3, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {4, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {5, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {6, 0, 0, 0, 0, 0, 0, 0, 0}},
                                   {{0, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {1, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {2, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {3, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {4, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {5, 0, 0, 0, 0, 0, 0, 0, 0},
                                    {6, 0, 0, 0, 0, 0, 0, 0, 0}}};
                                     
int hulp_array[5][7][2] = {{{0, 0},                         //uitgang - resterende tijd
                            {0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0}},
                           {{0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0}},
                           {{0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0}},
                           {{0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0}},
                           {{0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0}}};

int input_array[5] = {0, 0, 0, 0, 0};                            
                                                             
String waarde_string = "              ";
String alarm_tijd_string = "              ";
String instel_tijd_string = "              ";
String uitschakel_tijd_string = "              ";

void setup(){
  delay(2500);
  Serial.begin(115200);
  pinMode(OUTPUT_0, OUTPUT);
  pinMode(OUTPUT_1, OUTPUT);
  pinMode(OUTPUT_2, OUTPUT);
  pinMode(OUTPUT_3, OUTPUT);
  pinMode(OUTPUT_4, OUTPUT);
  digitalWrite(OUTPUT_0, false);
  digitalWrite(OUTPUT_1, false);
  digitalWrite(OUTPUT_2, false);
  digitalWrite(OUTPUT_3, false);
  digitalWrite(OUTPUT_4, false);
  pinMode(MINUUT_PULS, INPUT);
  attachInterrupt(digitalPinToInterrupt(MINUUT_PULS), minuut_voorbij, FALLING);
  pinMode(INPUT_0, INPUT_PULLDOWN);
  pinMode(INPUT_1, INPUT_PULLDOWN);
  pinMode(INPUT_2, INPUT_PULLDOWN);
  pinMode(INPUT_3, INPUT_PULLDOWN);
  pinMode(INPUT_4, INPUT_PULLDOWN);
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(APSSID, APPSWD);
  html_input();
  Wire.begin();                       // SDA >> GPIO21    SCL >> GPIO22
  Wire.setClock(40000);               // slow chinese DS3231 modules
  Wire.beginTransmission(DS3231SN);
  Wire.write(0x0B);
  Wire.write(0x80);
  Wire.write(0x80);
  Wire.write(0x80);
  Wire.write(0x46); 
  Wire.write(0x00);
  Wire.endTransmission();
  if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
    Serial.println("LittleFS Mount Failed");
    return;
  }
  testFile(LittleFS, "/pos000");
  for(int x = 0; x < 5; x ++){
    for(int y = 0; y < 7; y ++){
      for(int z = 0; z < 9; z ++){
        char lees_char[10];
        String lees_string = "/pos" + String(x) + String(y) + String(z);
        lees_string.toCharArray(lees_char, lees_string.length() + 1);
        readFile(LittleFS, lees_char);
        instellingen_array[x][y][z] = waarde_string.toInt();
      }
    }
  }
}

void loop(){
  if(minuut_interrupt_bool){
    minuut_interrupt_bool = false;
    lees_DS3231SN();
    Wire.beginTransmission(DS3231SN);
    Wire.write(0x0F);
    Wire.write(0x00);
    Wire.endTransmission();
    input_inlezen();
    for(timer_teller = 0; timer_teller < 5; timer_teller ++){
      for(module_teller = 0; module_teller < 7; module_teller ++){
        if(instellingen_array[timer_teller][module_teller][1] == AKTIEF){
          if(hulp_array[timer_teller][module_teller][0] == 1){
            if(instellingen_array[timer_teller][module_teller][6] == 0){                                 // uitschakelen na xx minuten gekozen
              hulp_array[timer_teller][module_teller][1] --;                                             // uitgang gestuurd >> minuten nog te gaan-1
              if(hulp_array[timer_teller][module_teller][1] == 0){                                       // indien 0 >> uitgang 0
                hulp_array[timer_teller][module_teller][0] = 0;
              }
            }
            if(instellingen_array[timer_teller][module_teller][6] == 1){                                // uitschakelen op tijd uu:mm gekozen
              if((instellingen_array[timer_teller][module_teller][7] == uur_int) && (instellingen_array[timer_teller][module_teller][8] == minuut_int)){
                hulp_array[timer_teller][module_teller][0] = 0;
              }
            }
          }
          if(instellingen_array[timer_teller][module_teller][2] == input_array[timer_teller]){          // ingang = gewenste ingang ????
            if((instellingen_array[timer_teller][module_teller][3] == DAGELIJKS)
              || ((instellingen_array[timer_teller][module_teller][3] == WEEKDAGEN) && ((dag_int >= MAANDAG ) && (dag_int < ZATERDAG)))
              || ((instellingen_array[timer_teller][module_teller][3] == WEEKEND) && ((dag_int >= ZATERDAG ) && (dag_int <= ZONDAG)))
              || (instellingen_array[timer_teller][module_teller][3] == dag_int)){
              if((uur_int == instellingen_array[timer_teller][module_teller][4]) && (minuut_int == instellingen_array[timer_teller][module_teller][5])){
                hulp_array[timer_teller][module_teller][0] = 1;
                hulp_array[timer_teller][module_teller][1] = instellingen_array[timer_teller][module_teller][8];
              }
            }
          }
        }
      }
    }
    output_schrijven();
  }
}

void readFile(fs::FS &fs, const char * path){
    File file = fs.open(path);
    if(!file){
      return;
    }
    if(file.isDirectory()){
      return;
    }
    int teller = 0;
    memset(lees_char, 0, sizeof(lees_char));
    while(file.available()){
        lees_char[teller] += file.read();
        teller ++;
    }
    waarde_string = String(lees_char);
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    File file = fs.open(path, FILE_WRITE);
    file.print(message);
    file.close();
}

void testFile(fs::FS &fs, const char * path){
  File file = fs.open(path);
  if(!file){
    for(int x = 0; x < 5; x ++){
      for(int y = 0; y < 7; y ++){
        for(int z = 0; z < 9; z ++){
          String schrijf_string = "/pos" + String(x) + String(y) + String(z);
          schrijf_string.toCharArray(schrijf_char, schrijf_string.length() + 1);
          char waarde_char[5];
          String tmp_waarde_string;
          tmp_waarde_string = String(instellingen_array[x][y][z]);
          tmp_waarde_string.toCharArray(waarde_char, tmp_waarde_string.length() + 1);
          writeFile(LittleFS, schrijf_char, waarde_char);
        }
      }
    }
  }
}

byte dec_naar_bcd(byte waarde){
  return (((waarde / 10) << 4) + (waarde % 10));
}

byte bcd_naar_dec(byte waarde){
  return (((waarde >> 4) * 10) + (waarde % 16));
}

void schrijf_DS3231SN(){
  Wire.beginTransmission(DS3231SN);
  Wire.write(0x00);
  Wire.write(dec_naar_bcd(seconde_int));
  Wire.write(dec_naar_bcd(minuut_int));
  Wire.write(dec_naar_bcd(uur_int) &0x3f);
  Wire.write(dag_int);
  Wire.endTransmission();
}

void lees_DS3231SN(){
  Wire.beginTransmission(DS3231SN);
  Wire.write(0x01);
  Wire.endTransmission();
  Wire.requestFrom(DS3231SN, 3);
  minuut_int = (bcd_naar_dec(Wire.read()));
  uur_int = (bcd_naar_dec(Wire.read()));
  dag_int = Wire.read();
}

void IRAM_ATTR minuut_voorbij(){
  minuut_interrupt_bool = true;
}

void input_inlezen(){
  input_array[0] = digitalRead(INPUT_0);
  input_array[1] = digitalRead(INPUT_1);
  input_array[2] = digitalRead(INPUT_2);
  input_array[3] = digitalRead(INPUT_3);
  input_array[4] = digitalRead(INPUT_4);
}

void output_schrijven(){
  digitalWrite(OUTPUT_0, (hulp_array[0][0][0] || hulp_array[0][1][0] || hulp_array[0][2][0] || hulp_array[0][3][0] || hulp_array[0][4][0] || hulp_array[0][5][0] || hulp_array[0][6][0])); 
  digitalWrite(OUTPUT_1, (hulp_array[1][0][0] || hulp_array[1][1][0] || hulp_array[1][2][0] || hulp_array[1][3][0] || hulp_array[1][4][0] || hulp_array[1][5][0] || hulp_array[1][6][0])); 
  digitalWrite(OUTPUT_2, (hulp_array[2][0][0] || hulp_array[2][1][0] || hulp_array[2][2][0] || hulp_array[2][3][0] || hulp_array[2][4][0] || hulp_array[2][5][0] || hulp_array[2][6][0])); 
  digitalWrite(OUTPUT_3, (hulp_array[3][0][0] || hulp_array[3][1][0] || hulp_array[3][2][0] || hulp_array[3][3][0] || hulp_array[3][4][0] || hulp_array[3][5][0] || hulp_array[3][6][0])); 
  digitalWrite(OUTPUT_4, (hulp_array[4][0][0] || hulp_array[4][1][0] || hulp_array[4][2][0] || hulp_array[4][3][0] || hulp_array[4][4][0] || hulp_array[4][5][0] || hulp_array[4][6][0])); 
}

const char index_html[] = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <iframe style="display:none" name="hidden-form"></iframe>
    <title>ESP32 Timers</title>
    <meta name="viewport" content="width=device-width, initial-scale=.80">
    <style>
      div.kader {
          position: relative;
          left: 0px;
          width: 400px;
      }
      div.kader_2 {
        position: absolute;
        left : 80px;
        width: 80px;
      }
      div.kader_3 {
        position: absolute;
        left : 160px;
        width: 80px;
      }
      div.kader_4 {
        position: absolute;
        left : 240px;
        width: 80px;
      }
      div.vak_6_1 {
        position: absolute;
        left : 0px;
        width: 66px;
      }
      div.vak_6_2 {
        position: absolute;
        left : 67px;
        width: 66px;
      }
      div.vak_6_3 {
        position: absolute;
        left : 133px;
        width: 66px;
      }
      div.vak_6_4 {
        position: absolute;
        left : 199px;
        width: 66px;
      }
      div.vak_6_5 {
        position: absolute;
        left : 265px;
        width: 66px;
      }
      div.vak_6_6 {
        position: absolute;
        left : 331px;
        width: 66px;
      }
      div.blanco_20{
        width: auto;
        height: 20px;
      }
      div.blanco_30{
        width: auto;
        height: 30px;
      }
      div.blanco_40{
        width: auto;
        height: 40px;
      }
      div.blanco_50{
        width: auto;
        height: 50px;
      }
      div.blanco_60{
        width: auto;
        height: 60px;
      }
      div.blanco_80{
        width: auto;
        height: 80px;
      }
    </style>
  </head>
  <body>
    <center>
    <h3><center> ESP32 Timer 5 uitgangen </center></h3>
    <small>
    <div class="kader">
      <b><center>%dag_tijd%</center></b>
      <div class="blanco_30">&nbsp;</div>
      <b><center>Uitgang keuze</center></b>
      <div class="blanco_20">&nbsp;</div>
      <form action="/get" target="hidden-form">
        <b>
          <div class="kader_2">
            <center><input type="submit" name= "timer_min" value="   -   " onclick="bevestig()"></center>
          </div>
          <div class="kader_3">
            <center><input type="text"  value="%timer_keuze%" style="text-align:center;" size=1></center>
          </div>
          <div class="kader_4">
            <center><input type="submit" name = "timer_plus" value="   +   " onclick="bevestig()"></center>
          </div>
        </b>
      </form>
      <div class="blanco_60">&nbsp;</div>
      <b>
        <div class="vak_6_1">
          <center>timer</center>
        </div>
        <div class="vak_6_2">
          <center>ingang</center>
        </div>
        <div class="vak_6_3">
          <center>dag</center>
        </div>
        <div class="vak_6_4">
          <center>tijd</center>
        </div>
        <div class="vak_6_5">
          <center>duurtijd</center>
        </div>
        <div class="vak_6_6">
          <center>uit</center>
        </div>
      </b>
      <div class="blanco_30">&nbsp;</div>
      <div class="vak_6_1">
        <center>%module_0%</center>
      </div>
      <div class="vak_6_2">
        <center>%ingang_0%</center>
      </div>
      <div class="vak_6_3">
        <center>%dag_0%</center>
      </div>
      <div class="vak_6_4">
        <center>%tijd_0%</center>
      </div>
      <div class="vak_6_5">
        <center>%duurtijd_0%</center>
      </div>
      <div class="vak_6_6">
        <center>%uit_0%</center>
      </div>
      <div class="blanco_20">&nbsp;</div>
      <div class="vak_6_1">
        <center>%module_1%</center>
      </div>
      <div class="vak_6_2">
        <center>%ingang_1%</center>
      </div>
      <div class="vak_6_3">
        <center>%dag_1%</center>
      </div>
      <div class="vak_6_4">
        <center>%tijd_1%</center>
      </div>
      <div class="vak_6_5">
        <center>%duurtijd_1%</center>
      </div>
      <div class="vak_6_6">
        <center>%uit_1%</center>
      </div>
      <div class="blanco_20">&nbsp;</div>
      <div class="vak_6_1">
        <center>%module_2%</center>
      </div>
      <div class="vak_6_2">
        <center>%ingang_2%</center>
      </div>
      <div class="vak_6_3">
        <center>%dag_2%</center>
      </div>
      <div class="vak_6_4">
        <center>%tijd_2%</center>
      </div>
      <div class="vak_6_5">
        <center>%duurtijd_2%</center>
      </div>
      <div class="vak_6_6">
        <center>%uit_2%</center>
      </div>
      <div class="blanco_20">&nbsp;</div>
      <div class="vak_6_1">
        <center>%module_3%</center>
      </div>
      <div class="vak_6_2">
        <center>%ingang_3%</center>
      </div>
      <div class="vak_6_3">
        <center>%dag_3%</center>
      </div>
      <div class="vak_6_4">
        <center>%tijd_3%</center>
      </div>
      <div class="vak_6_5">
        <center>%duurtijd_3%</center>
      </div>
      <div class="vak_6_6">
        <center>%uit_3%</center>
      </div>
      <div class="blanco_20">&nbsp;</div>
      <div class="vak_6_1">
        <center>%module_4%</center>
      </div>
      <div class="vak_6_2">
        <center>%ingang_4%</center>
      </div>
      <div class="vak_6_3">
        <center>%dag_4%</center>
      </div>
      <div class="vak_6_4">
        <center>%tijd_4%</center>
      </div>
      <div class="vak_6_5">
        <center>%duurtijd_4%</center>
      </div>
      <div class="vak_6_6">
        <center>%uit_4%</center>
      </div>
      <div class="blanco_20">&nbsp;</div>
      <div class="vak_6_1">
        <center>%module_5%</center>
      </div>
      <div class="vak_6_2">
        <center>%ingang_5%</center>
      </div>
      <div class="vak_6_3">
        <center>%dag_5%</center>
      </div>
      <div class="vak_6_4">
        <center>%tijd_5%</center>
      </div>
      <div class="vak_6_5">
        <center>%duurtijd_5%</center>
      </div>
      <div class="vak_6_6">
        <center>%uit_5%</center>
      </div>
      <div class="blanco_20">&nbsp;</div>
      <div class="vak_6_1">
        <center>%module_6%</center>
      </div>
      <div class="vak_6_2">
        <center>%ingang_6%</center>
      </div>
      <div class="vak_6_3">
        <center>%dag_6%</center>
      </div>
      <div class="vak_6_4">
        <center>%tijd_6%</center>
      </div>
      <div class="vak_6_5">
        <center>%duurtijd_6%</center>
      </div>
      <div class="vak_6_6">
        <center>%uit_6%</center>
      </div>
      <div class="blanco_60">&nbsp;</div>
      <b><center>Timers uitgang <var> %timer_nummer%</var> instellen</center></b>
      <div class="blanco_20">&nbsp;</div>
      <form action="/get" target="hidden-form">
        <b>
          <div class="vak_6_1">
            <center><input type="submit" name= "module_plus" value="   +   " onclick="bevestig()"></center>
          </div>
          <div class="vak_6_2">
            <center>actief</center>
          </div>
          <div class="vak_6_3">
            <center>ingang</center>
          </div>
          <div class="vak_6_4">
            <center>dag</center>
          </div>
          <div class="vak_6_5">
            <center>tijd</center>
          </div>
          <div class="vak_6_6">
            <center>duurtijd</center>
          </div>
        </b>
        <div class="blanco_40">&nbsp;</div>
        <div class="vak_6_1">
          <center><input type="text" value=%module% name="module_" style="text-align:center;" size=1></center>
        </div>
        <div class="vak_6_2">
          <center><input type="text" value=%aktief% name="aktief_" style="text-align:center;" size=1></center>
        </div>
        <div class="vak_6_3">
          <center><input type="text" value=%ingang% name="ingang_" style="text-align:center;" size=1></center>
        </div>
        <div class="vak_6_4">
          <center><input type="text" value=%dag% name="dag_" style="text-align:center;" size=1></center>
        </div>
        <div class="vak_6_5">
          <center><input type="text" value=%tijd% name="tijd_" style="text-align:center;" size=1></center>
        </div>
        <div class="vak_6_6">
          <center><input type="text" value=%duurtijd% name="duurtijd_" style="text-align:center;" size=1></center>
        </div>
        <div class="blanco_40">&nbsp;</div>
        <div class="vak_6_1">
          <center><input type="submit" name="module_min" value="   -   " onclick="bevestig()"></center>
        </div>
        <div class="kader_3">
          <center><input type="submit" name="timer_bevestig" value=" OK " onclick="bevestig()"></center>
        </div>
        <div class="blanco_60">&nbsp;</div>
      </form>
      <b><center>Klok instellen</center></b>
      <div class="blanco_20">&nbsp;</div>
      <b>
        <div class="vak_6_3">
          <center>dag</center>
        </div>
        <div class="vak_6_4">
          <center>tijd</center>
        </div>
      </b>
      <div class="blanco_30">&nbsp;</div>
      <form action="/get" target="hidden-form">
        <div class="vak_6_3">
          <center><input type="text" value=%instel_dag% name="instel_dag_" style="text-align:center;" size=1></center>
        </div>
        <div class="vak_6_4">
          <center><input type="text" value=%instel_tijd% name="instel_tijd_" style="text-align:center;" size=1></center>
        </div>
        <div class="blanco_40">&nbsp;</div>
        <div class="kader_3">
          <center><input type="submit" name="tijd_bevestig" value=" OK " onclick="bevestig()"></center>
        </div>
      </form>
    </div>
    </small>
    </center>
    <br>
    <br>
    <h6>thieu-b55 mei 2024</h6>
    <script>
      function bevestig(){
        setTimeout(function(){document.location.reload();},250);
      }
    </script>
  </body>  
</html>
)rawliteral";

String processor(const String& var){
  if(var == "dag_tijd"){
    memset(dag_tijd_char, 0, sizeof(dag_tijd_char));
    sprintf(dag_tijd_char, "%s  %02d:%02d", weekdagen_char[dag_int], uur_int, minuut_int);
    return(dag_tijd_char);
  }
  if(var == "timer_keuze"){
    return(String(timer_teller_int));
  }
  if(var == "module_0"){
    if(instellingen_array[timer_teller_int][0][1] == 1){
      return(String(instellingen_array[timer_teller_int][0][0]));
    }
    else{
      return("");
    }
  }
  if(var == "ingang_0"){
    if(instellingen_array[timer_teller_int][0][1] == 1){
      return(String(instellingen_array[timer_teller_int][0][2]));
    }
    else{
      return("");
    }
  }
  if(var == "dag_0"){
    if(instellingen_array[timer_teller_int][0][1] == 1){
      return(String(instellingen_array[timer_teller_int][0][3]));
    }
    else{
      return("");
    }
  }
  if(var == "tijd_0"){
    if(instellingen_array[timer_teller_int][0][1] == 1){
      sprintf(tijd_char, "%02d:%02d", instellingen_array[timer_teller_int][0][4], instellingen_array[timer_teller_int][0][5]);
      return(tijd_char);
    }
    else{
      return("");
    }
  }
  if(var == "duurtijd_0"){
    if(instellingen_array[timer_teller_int][0][1] == 1){
      if(instellingen_array[timer_teller_int][0][6] == 0){
        return(String(instellingen_array[timer_teller_int][0][8]));
      }
      if(instellingen_array[timer_teller_int][0][6] == 1){
        sprintf(uitschakel_char, "%02d:%02d", instellingen_array[timer_teller_int][0][7], instellingen_array[timer_teller_int][0][8]);
        return(uitschakel_char);
      }
    }
    else{
      return("");
    }
  }
  if(var == "uit_0"){
    if(instellingen_array[timer_teller_int][0][1] == 1){
      return(String(hulp_array[timer_teller_int][0][0]));
    }
    else{
      return("");
    }
  }
  if(var == "module_1"){
    if(instellingen_array[timer_teller_int][1][1] == 1){
      return(String(instellingen_array[timer_teller_int][1][0]));
    }
    else{
      return("");
    }
  }
  if(var == "ingang_1"){
    if(instellingen_array[timer_teller_int][1][1] == 1){
      return(String(instellingen_array[timer_teller_int][1][2]));
    }
    else{
      return("");
    }
  }
  if(var == "dag_1"){
    if(instellingen_array[timer_teller_int][1][1] == 1){
      return(String(instellingen_array[timer_teller_int][1][3]));
    }
    else{
      return("");
    }
  }
  if(var == "tijd_1"){
    if(instellingen_array[timer_teller_int][1][1] == 1){
      sprintf(tijd_char, "%02d:%02d", instellingen_array[timer_teller_int][1][4], instellingen_array[timer_teller_int][1][5]);
      return(tijd_char);
    }
    else{
      return("");
    }
  }
  if(var == "duurtijd_1"){
    if(instellingen_array[timer_teller_int][1][1] == 1){
      if(instellingen_array[timer_teller_int][1][6] == 0){
        return(String(instellingen_array[timer_teller_int][1][8]));
      }
      if(instellingen_array[timer_teller_int][1][6] == 1){
        sprintf(uitschakel_char, "%02d:%02d", instellingen_array[timer_teller_int][1][7], instellingen_array[timer_teller_int][1][8]);
        return(uitschakel_char);
      }
    }
    else{
      return("");
    }
  }
  if(var == "uit_1"){
    if(instellingen_array[timer_teller_int][1][1] == 1){
      return(String(hulp_array[timer_teller_int][1][0]));
    }
    else{
      return("");
    }
  }
  if(var == "module_2"){
    if(instellingen_array[timer_teller_int][2][1] == 1){
      return(String(instellingen_array[timer_teller_int][2][0]));
    }
    else{
      return("");
    }
  }
  if(var == "ingang_2"){
    if(instellingen_array[timer_teller_int][2][1] == 1){
      return(String(instellingen_array[timer_teller_int][2][2]));
    }
    else{
      return("");
    }
  }
  if(var == "dag_2"){
    if(instellingen_array[timer_teller_int][2][1] == 1){
      return(String(instellingen_array[timer_teller_int][2][3]));
    }
    else{
      return("");
    }
  }
  if(var == "tijd_2"){
    if(instellingen_array[timer_teller_int][2][1] == 1){
      sprintf(tijd_char, "%02d:%02d", instellingen_array[timer_teller_int][2][4], instellingen_array[timer_teller_int][2][5]);
      return(tijd_char);
    }
    else{
      return("");
    }
  }
  if(var == "duurtijd_2"){
    if(instellingen_array[timer_teller_int][2][1] == 1){
      if(instellingen_array[timer_teller_int][2][6] == 0){
        return(String(instellingen_array[timer_teller_int][2][8]));
      }
      if(instellingen_array[timer_teller_int][2][6] == 1){
        sprintf(uitschakel_char, "%02d:%02d", instellingen_array[timer_teller_int][2][7], instellingen_array[timer_teller_int][2][8]);
        return(uitschakel_char);
      }
    }
    else{
      return("");
    }
  }
  if(var == "uit_2"){
    if(instellingen_array[timer_teller_int][2][1] == 1){
      return(String(hulp_array[timer_teller_int][2][0]));
    }
    else{
      return("");
    }
  }
  if(var == "module_3"){
    if(instellingen_array[timer_teller_int][3][1] == 1){
      return(String(instellingen_array[timer_teller_int][3][0]));
    }
    else{
      return("");
    }
  }
  if(var == "ingang_3"){
    if(instellingen_array[timer_teller_int][3][1] == 1){
      return(String(instellingen_array[timer_teller_int][3][2]));
    }
    else{
      return("");
    }
  }
  if(var == "dag_3"){
    if(instellingen_array[timer_teller_int][3][1] == 1){
      return(String(instellingen_array[timer_teller_int][3][3]));
    }
    else{
      return("");
    }
  }
  if(var == "tijd_3"){
    if(instellingen_array[timer_teller_int][3][1] == 1){
      sprintf(tijd_char, "%02d:%02d", instellingen_array[timer_teller_int][3][4], instellingen_array[timer_teller_int][3][5]);
      return(tijd_char);
    }
    else{
      return("");
    }
  }
  if(var == "duurtijd_3"){
    if(instellingen_array[timer_teller_int][3][1] == 1){
      if(instellingen_array[timer_teller_int][3][6] == 0){
        return(String(instellingen_array[timer_teller_int][3][8]));
      }
      if(instellingen_array[timer_teller_int][3][6] == 1){
        sprintf(uitschakel_char, "%02d:%02d", instellingen_array[timer_teller_int][3][7], instellingen_array[timer_teller_int][3][8]);
        return(uitschakel_char);
      }
    }
    else{
      return("");
    }
  }
  if(var == "uit_3"){
    if(instellingen_array[timer_teller_int][3][1] == 1){
      return(String(hulp_array[timer_teller_int][3][0]));
    }
    else{
      return("");
    }
  }
  if(var == "module_4"){
    if(instellingen_array[timer_teller_int][4][1] == 1){
      return(String(instellingen_array[timer_teller_int][4][0]));
    }
    else{
      return("");
    }
  }
  if(var == "ingang_4"){
    if(instellingen_array[timer_teller_int][4][1] == 1){
      return(String(instellingen_array[timer_teller_int][4][2]));
    }
    else{
      return("");
    }
  }
  if(var == "dag_4"){
    if(instellingen_array[timer_teller_int][4][1] == 1){
      return(String(instellingen_array[timer_teller_int][4][3]));
    }
    else{
      return("");
    }
  }
  if(var == "tijd_4"){
    if(instellingen_array[timer_teller_int][4][1] == 1){
      sprintf(tijd_char, "%02d:%02d", instellingen_array[timer_teller_int][4][4], instellingen_array[timer_teller_int][4][5]);
      return(tijd_char);
    }
    else{
      return("");
    }
  }
  if(var == "duurtijd_4"){
    if(instellingen_array[timer_teller_int][4][1] == 1){
      if(instellingen_array[timer_teller_int][4][6] == 0){
        return(String(instellingen_array[timer_teller_int][4][8]));
      }
      if(instellingen_array[timer_teller_int][4][6] == 1){
        sprintf(uitschakel_char, "%02d:%02d", instellingen_array[timer_teller_int][4][7], instellingen_array[timer_teller_int][4][8]);
        return(uitschakel_char);
      }
    }
    else{
      return("");
    }
  }
  if(var == "uit_4"){
    if(instellingen_array[timer_teller_int][4][1] == 1){
      return(String(hulp_array[timer_teller_int][4][0]));
    }
    else{
      return("");
    }
  }
  if(var == "module_5"){
    if(instellingen_array[timer_teller_int][5][1] == 1){
      return(String(instellingen_array[timer_teller_int][5][0]));
    }
    else{
      return("");
    }
  }
  if(var == "ingang_5"){
    if(instellingen_array[timer_teller_int][5][1] == 1){
      return(String(instellingen_array[timer_teller_int][5][2]));
    }
    else{
      return("");
    }
  }
  if(var == "dag_5"){
    if(instellingen_array[timer_teller_int][5][1] == 1){
      return(String(instellingen_array[timer_teller_int][5][3]));
    }
    else{
      return("");
    }
  }
  if(var == "tijd_5"){
    if(instellingen_array[timer_teller_int][5][1] == 1){
      sprintf(tijd_char, "%02d:%02d", instellingen_array[timer_teller_int][5][4], instellingen_array[timer_teller_int][5][5]);
      return(tijd_char);
    }
    else{
      return("");
    }
  }
  if(var == "duurtijd_5"){
    if(instellingen_array[timer_teller_int][5][1] == 1){
      if(instellingen_array[timer_teller_int][5][6] == 0){
        return(String(instellingen_array[timer_teller_int][5][8]));
      }
      if(instellingen_array[timer_teller_int][5][6] == 1){
        sprintf(uitschakel_char, "%02d:%02d", instellingen_array[timer_teller_int][5][7], instellingen_array[timer_teller_int][5][8]);
        return(uitschakel_char);
      }
    }
    else{
      return("");
    }
  }
  if(var == "uit_5"){
    if(instellingen_array[timer_teller_int][5][1] == 1){
      return(String(hulp_array[timer_teller_int][5][0]));
    }
    else{
      return("");
    }
  }
  if(var == "module_6"){
    if(instellingen_array[timer_teller_int][6][1] == 1){
      return(String(instellingen_array[timer_teller_int][6][0]));
    }
    else{
      return("");
    }
  }
  if(var == "ingang_6"){
    if(instellingen_array[timer_teller_int][6][1] == 1){
      return(String(instellingen_array[timer_teller_int][6][2]));
    }
    else{
      return("");
    }
  }
  if(var == "dag_6"){
    if(instellingen_array[timer_teller_int][6][1] == 1){
      return(String(instellingen_array[timer_teller_int][6][3]));
    }
    else{
      return("");
    }
  }
  if(var == "tijd_6"){
    if(instellingen_array[timer_teller_int][6][1] == 1){
      sprintf(tijd_char, "%02d:%02d", instellingen_array[timer_teller_int][6][4], instellingen_array[timer_teller_int][6][5]);
      return(tijd_char);
    }
    else{
      return("");
    }
  }
  if(var == "duurtijd_6"){
    if(instellingen_array[timer_teller_int][6][1] == 1){
      if(instellingen_array[timer_teller_int][6][6] == 0){
        return(String(instellingen_array[timer_teller_int][6][8]));
      }
      if(instellingen_array[timer_teller_int][6][6] == 1){
        sprintf(uitschakel_char, "%02d:%02d", instellingen_array[timer_teller_int][6][7], instellingen_array[timer_teller_int][6][8]);
        return(uitschakel_char);
      }
    }
    else{
      return("");
    }
  }
  if(var == "uit_6"){
    if(instellingen_array[timer_teller_int][6][1] == 1){
      return(String(hulp_array[timer_teller_int][6][0]));
    }
    else{
      return("");
    }
  }
  if(var == "timer_nummer"){
    return(String(timer_teller_int));
  }
  if(var == "module"){
    return(String(instellingen_array[timer_teller_int][module_teller_int][0]));
  }
  if(var == "aktief"){
    return(String(instellingen_array[timer_teller_int][module_teller_int][1]));
  }
  if(var == "ingang"){
    return(String(instellingen_array[timer_teller_int][module_teller_int][2]));
  }
  if(var == "dag"){
    return(String(instellingen_array[timer_teller_int][module_teller_int][3]));
  }
  if(var == "tijd"){
    sprintf(module_tijd_char, "%02d:%02d", instellingen_array[timer_teller_int][module_teller_int][4], instellingen_array[timer_teller_int][module_teller_int][5]);
    return(module_tijd_char);
  }
  if(var == "duurtijd"){
    if(instellingen_array[timer_teller_int][module_teller_int][6] == 0){
      return(String(instellingen_array[timer_teller_int][module_teller_int][8]));
    }
    if(instellingen_array[timer_teller_int][module_teller_int][6] == 1){
      memset(schakel_char, 0, sizeof(schakel_char));
      sprintf(schakel_char, "%02d:%02d", instellingen_array[timer_teller_int][module_teller_int][7], instellingen_array[timer_teller_int][module_teller_int][8]);
      return(schakel_char);
    }
  }
  if(var == "instel_dag"){
    return(String(dag_int));
  }
  if(var == "instel_tijd"){
    memset(tijd_char, 0, sizeof(tijd_char));
    sprintf(tijd_char, "%02d:%02d", uur_int, minuut_int);
    return(tijd_char);
  }
  return("");
}

void html_input(){
  server.begin();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){
    int temp;
    char terminator = char(0x0a);
    if(request->hasParam(TIMER_MIN)){
      timer_teller_int --;
      if(timer_teller_int < 0){
        timer_teller_int = 0;
      }
    }
    if(request->hasParam(TIMER_PLUS)){
      timer_teller_int ++;
      if(timer_teller_int > 4){
        timer_teller_int = 4;
      }
    }
    if(request->hasParam(MODULE_PLUS)){
      module_teller_int ++;
      if(module_teller_int > 6){
        module_teller_int = 6;
      }
    }
    if(request->hasParam(MODULE_MIN)){
      module_teller_int --;
      if(module_teller_int < 0){
        module_teller_int = 0;
      }
    }
    if(request->hasParam(AKTIEF_)){
      aktief_int = ((request->getParam(AKTIEF_)->value()) + String(terminator)).toInt();
    }
    if(request->hasParam(INGANG_)){
      ingang_int = ((request->getParam(INGANG_)->value()) + String(terminator)).toInt();
    }
    if(request->hasParam(DAG_)){
      timer_dag_int = ((request->getParam(DAG_)->value()) + String(terminator)).toInt();
    }
    if(request->hasParam(TIJD_)){
      alarm_tijd_string = "";
      alarm_tijd_string = (request->getParam(TIJD_)->value()) + String(terminator);
    }
    if(request->hasParam(DUURTIJD_)){
      uitschakel_tijd_string = "";
      uitschakel_tijd_string = (request->getParam(DUURTIJD_)->value()) + String(terminator);
    }
    if(request->hasParam(TIMER_BEVESTIG)){
      String schrijf_string_1 = "/pos" + String(timer_teller_int) + String(module_teller_int);
      String tmp_waarde_string = "          ";
      if((aktief_int == 0) || (aktief_int == 1)){
        instellingen_array[timer_teller_int][module_teller_int][1] = aktief_int;
        String schrijf_string = schrijf_string_1 + "1";
        memset(schrijf_char, 0, sizeof(schrijf_char));
        schrijf_string.toCharArray(schrijf_char, schrijf_string.length() + 1);
        tmp_waarde_string = String(aktief_int);
        memset(waarde_char, 0, sizeof(waarde_char));
        tmp_waarde_string.toCharArray(waarde_char, tmp_waarde_string.length() + 1);
        writeFile(LittleFS, schrijf_char, waarde_char);
      }
      if(ingang_int == 0){
        hulp_array[timer_teller_int][module_teller_int][0] = 0;
        hulp_array[timer_teller_int][module_teller_int][1] = 0;
      }
      if((ingang_int == 0) || (ingang_int == 1)){
        instellingen_array[timer_teller_int][module_teller_int][2] = ingang_int;
        String schrijf_string = schrijf_string_1 + "2";
        memset(schrijf_char, 0, sizeof(schrijf_char));
        schrijf_string.toCharArray(schrijf_char, schrijf_string.length() + 1);
        tmp_waarde_string = String(ingang_int);
        memset(waarde_char, 0, sizeof(waarde_char));
        tmp_waarde_string.toCharArray(waarde_char, tmp_waarde_string.length() + 1);
        writeFile(LittleFS, schrijf_char, waarde_char);
      }
      if((timer_dag_int > 0) && (timer_dag_int < 11)){
        instellingen_array[timer_teller_int][module_teller_int][3] = timer_dag_int;
        String schrijf_string = schrijf_string_1 + "3";
        memset(schrijf_char, 0, sizeof(schrijf_char));
        schrijf_string.toCharArray(schrijf_char, schrijf_string.length() + 1);
        tmp_waarde_string = String(timer_dag_int);
        memset(waarde_char, 0, sizeof(waarde_char));
        tmp_waarde_string.toCharArray(waarde_char, tmp_waarde_string.length() + 1);
        writeFile(LittleFS, schrijf_char, waarde_char);
      }
      temp = alarm_tijd_string.indexOf(":");
      if(temp != -1){
        timer_uur_int = alarm_tijd_string.substring(0, temp).toInt();
        timer_minuten_int = alarm_tijd_string.substring(temp + 1).toInt();
        if((timer_uur_int > -1) && (timer_uur_int < 24) && (timer_minuten_int > -1) && (timer_minuten_int < 60)){
          instellingen_array[timer_teller_int][module_teller_int][4] = timer_uur_int;
          instellingen_array[timer_teller_int][module_teller_int][5] = timer_minuten_int;
          String schrijf_string = schrijf_string_1 + "4";
          memset(schrijf_char, 0, sizeof(schrijf_char));
          schrijf_string.toCharArray(schrijf_char, schrijf_string.length() + 1);
          tmp_waarde_string = String(timer_uur_int);
          memset(waarde_char, 0, sizeof(waarde_char));
          tmp_waarde_string.toCharArray(waarde_char, tmp_waarde_string.length() + 1);
          writeFile(LittleFS, schrijf_char, waarde_char);
          schrijf_string = schrijf_string_1 + "5";
          memset(schrijf_char, 0, sizeof(schrijf_char));
          schrijf_string.toCharArray(schrijf_char, schrijf_string.length() + 1);
          tmp_waarde_string = String(timer_minuten_int);
          memset(waarde_char, 0, sizeof(waarde_char));
          tmp_waarde_string.toCharArray(waarde_char, tmp_waarde_string.length() + 1);
          writeFile(LittleFS, schrijf_char, waarde_char);
        }
      }
      temp = uitschakel_tijd_string.indexOf(":");
      if(temp == -1){
        duurtijd_int = uitschakel_tijd_string.toInt();      
        if(duurtijd_int > 0){
          instellingen_array[timer_teller_int][module_teller_int][6] = 0;
          String schrijf_string = schrijf_string_1 + "6";
          memset(schrijf_char, 0, sizeof(schrijf_char));
          schrijf_string.toCharArray(schrijf_char, schrijf_string.length() + 1);
          tmp_waarde_string = String(0);
          memset(waarde_char, 0, sizeof(waarde_char));
          tmp_waarde_string.toCharArray(waarde_char, tmp_waarde_string.length() + 1);
          writeFile(LittleFS, schrijf_char, waarde_char);
          instellingen_array[timer_teller_int][module_teller_int][8] = duurtijd_int;
          schrijf_string = schrijf_string_1 + "8";
          memset(schrijf_char, 0, sizeof(schrijf_char));
          schrijf_string.toCharArray(schrijf_char, schrijf_string.length() + 1);
          tmp_waarde_string = String(duurtijd_int);
          memset(waarde_char, 0, sizeof(waarde_char));
          tmp_waarde_string.toCharArray(waarde_char, tmp_waarde_string.length() + 1);
          writeFile(LittleFS, schrijf_char, waarde_char);
        }
      }
      if(temp != -1){
        schakel_uur_int = uitschakel_tijd_string.substring(0, temp).toInt();
        schakel_minuten_int = uitschakel_tijd_string.substring(temp + 1).toInt();
        if((schakel_uur_int > -1) && (schakel_uur_int < 24) && (schakel_minuten_int > -1) && (schakel_minuten_int < 60)){
          instellingen_array[timer_teller_int][module_teller_int][6] = 1;
          instellingen_array[timer_teller_int][module_teller_int][7] = schakel_uur_int;
          instellingen_array[timer_teller_int][module_teller_int][8] = schakel_minuten_int;
          String schrijf_string = schrijf_string_1 + "6";
          memset(schrijf_char, 0, sizeof(schrijf_char));
          schrijf_string.toCharArray(schrijf_char, schrijf_string.length() + 1);
          tmp_waarde_string = String(1);
          memset(waarde_char, 0, sizeof(waarde_char));
          tmp_waarde_string.toCharArray(waarde_char, tmp_waarde_string.length() + 1);
          writeFile(LittleFS, schrijf_char, waarde_char);
          schrijf_string = schrijf_string_1 + "7";
          memset(schrijf_char, 0, sizeof(schrijf_char));
          schrijf_string.toCharArray(schrijf_char, schrijf_string.length() + 1);
          tmp_waarde_string = String(schakel_uur_int);
          memset(waarde_char, 0, sizeof(waarde_char));
          tmp_waarde_string.toCharArray(waarde_char, tmp_waarde_string.length() + 1);
          writeFile(LittleFS, schrijf_char, waarde_char);
          schrijf_string = schrijf_string_1 + "8";
          memset(schrijf_char, 0, sizeof(schrijf_char));
          schrijf_string.toCharArray(schrijf_char, schrijf_string.length() + 1);
          tmp_waarde_string = String(schakel_minuten_int);
          memset(waarde_char, 0, sizeof(waarde_char));
          tmp_waarde_string.toCharArray(waarde_char, tmp_waarde_string.length() + 1);
          writeFile(LittleFS, schrijf_char, waarde_char);
        }
      }
      
    }
    if(request->hasParam(INSTEL_DAG_)){
      instel_dag_int = ((request->getParam(INSTEL_DAG_)->value()) + String(terminator)).toInt();
    }
    if(request->hasParam(INSTEL_TIJD_)){
      instel_tijd_string = "";
      instel_tijd_string = (request->getParam(INSTEL_TIJD_)->value()) + String(terminator);
    }
    if(request->hasParam(TIJD_BEVESTIG)){
      if((instel_dag_int > 0) && (instel_dag_int < 8)){
        int temp = instel_tijd_string.indexOf(":");
        if(temp != -1){
          instel_uur_int = instel_tijd_string.substring(0, temp).toInt();
          instel_minuut_int = instel_tijd_string.substring(temp + 1).toInt();
          if((instel_uur_int > -1) && (instel_uur_int < 24) && (instel_minuut_int > -1) && (instel_minuut_int < 60)){
            uur_int =instel_uur_int;
            minuut_int = instel_minuut_int;
            seconde_int = 0;
            dag_int = instel_dag_int;
            schrijf_DS3231SN();
            lees_DS3231SN();
          }
        }
      }
    }
  });
}
