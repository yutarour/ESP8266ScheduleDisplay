#include <Adafruit_ST7735.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "NTP.h"
#include "time.h"

#include "img.h"
#include "web.h"

#define TFT_CS D1
#define TFT_RST D2
#define TFT_DC D3
#define TFT_LED D8
const char *ssid = "<SSID HERE>";
const char *password = "<WIFI PASS HERE>";

ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTP ntp(ntpUDP);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);


struct times {
  int year;
  short month;
  short day;
  short hour;
  short minute;
  short second;
  String eventName;
};

const size_t eventMaxCount = 500;
struct times events[eventMaxCount] = {
  { 2023, 6, 23, 10, 40, 0, "School End" },
  { 2023, 5, 1, 7, 30, 0, "AP Test" }
};

size_t eventCount = 2;

void addTime(const times &timeToAdd) {
  if (eventCount >= eventMaxCount)
    return;  //maxed out

  events[eventCount++] = timeToAdd;
}

void removeTime(int idx) {
  if (idx >= eventCount)
    return;  //nothing to remove

  for (int i = idx; i < eventCount - 1; i++)
    events[i] = events[i + 1];

  eventCount -= 1;
}

//sort events by time
void sortByTime() {
  times temp;
  for (int i = 0; i < eventCount; i++) {
    for (int j = 0; j < eventCount; j++) {
      int iSeconds = (events[i].year * 31536000) + (events[i].month * 2592000) + (events[i].day * 86400) + (events[i].hour * 3600) + (events[i].minute * 60) + events[i].second;
      int jSeconds = (events[j].year * 31536000) + (events[j].month * 2592000) + (events[j].day * 86400) + (events[j].hour * 3600) + (events[j].minute * 60) + events[j].second;
      if (iSeconds < jSeconds) {
        temp = events[i];
        events[i] = events[j];
        events[j] = temp;
      }
    }
  }
}

void drawPixArr(int x, int y, const uint16_t *img) {
  for (int c = 2; c < img[0] * img[1]; c++) {
    tft.drawPixel(x + (c - 2) % img[0], y + c / img[0], img[c]);
  }
}

//timediff calculator
bool isLeapYear(int year) {
  if (year % 4 == 0) {
    if (year % 100 == 0) {
      if (year % 400 == 0) {
        return true;
      }
    }
  }
  return false;
}


int daysInMonth(int year, int month) {
  if (month == 2) {
    if (isLeapYear(year)) return 29;
    else return 28;
  } else if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
  else return 31;
}

times timeDiff(times start_time, times end_time) {
  int start_secs = (start_time.year * 365 * 24 * 60 * 60) + (start_time.month * 30 * 24 * 60 * 60) + (start_time.day * 24 * 60 * 60) + (start_time.hour * 60 * 60) + (start_time.minute * 60) + start_time.second;
  int end_secs = (end_time.year * 365 * 24 * 60 * 60) + (end_time.month * 30 * 24 * 60 * 60) + (end_time.day * 24 * 60 * 60) + (end_time.hour * 60 * 60) + (end_time.minute * 60) + end_time.second;
  int diff_secs = end_secs - start_secs;

  int diff_days = floor(diff_secs / (24 * 60 * 60));
  int years = floor(diff_days / 365);
  int remaining_days = diff_days % 365;

  int months = 0;
  while (remaining_days > 0) {
    int days_in_month_i = daysInMonth(start_time.year + years, start_time.month + months);
    if (remaining_days >= days_in_month_i) {
      remaining_days -= days_in_month_i;
      months += 1;
    } else break;
  }

  int days = remaining_days;
  int remaining_secs = diff_secs % (24 * 60 * 60);
  int hours = floor(remaining_secs / (60 * 60));
  remaining_secs = remaining_secs % (60 * 60);
  int minutes = floor(remaining_secs / 60);
  int seconds = remaining_secs % 60;

  times countdown = {
    years,
    months,
    days,
    hours,
    minutes,
    seconds,
    end_time.eventName
  };
  return countdown;
}

void copyString(const char *src, char *dest, int count) {
  for (int i = 0; i < count; i++)
    dest[i] = src[i];
}

bool backLightStatus = true;
void handleBackLight(){
  if (backLightStatus){
    digitalWrite(TFT_LED,LOW);
    backLightStatus=!backLightStatus;
  }
  else{
    backLightStatus=!backLightStatus;
    digitalWrite(TFT_LED,HIGH);
  }
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plane", "");
}

void handleAdd() {
  if (server.hasArg("year") && server.hasArg("month") && server.hasArg("day") && server.hasArg("hour") && server.hasArg("minute") && server.hasArg("second") && server.hasArg("event")) {
    if (server.arg("year") != "Year" && server.arg("month") != "Month" && server.arg("day") != "Day" && server.arg("hour") != "Hour" && server.arg("minute") != "Minute" && server.arg("second") != "Second") {
      times recv = {
        server.arg("year").toInt(),
        server.arg("month").toInt(),
        server.arg("day").toInt(),
        server.arg("hour").toInt(),
        server.arg("minute").toInt(),
        server.arg("second").toInt(),
        server.arg("event")
      };
      //copyString(server.arg("event"), recv.EventName, server.arg("event").length());

      addTime(recv);
      Serial.print("There are ");
      Serial.print(eventCount);
      Serial.println(" Objects in events");
      sortByTime();
    }
  } else {
    server.send(400, "text/plain", "Missing params");
  }
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plane", "");
}

void handleRoot() {
  String tosend = schedule_page;
  for(int i = 0;i<eventCount;i++){
   tosend+=events[i].eventName+" "+events[i].year+"/"+events[i].month+"/"+events[i].day+" "+events[i].hour+":"+events[i].minute+":"+events[i].second+"<br>";
  }
  server.send(200, "text/html", tosend);
}

void removeEntry(){
  if(server.hasArg("eventname")){
    for(int i =0;i<eventCount;i++){
      if(events[i].eventName.equalsIgnoreCase(server.arg("eventname"))){
        removeTime(i);
      }
    }
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plane", "");
    sortByTime();
  }
  else{
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plane", "");
  }
}

void handleNotFound() {
  server.send(404, "text/plain", "Not Found");
}

void setup() {
  Serial.begin(9600);

  //ntp, wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  ntp.ruleDST("PDT", Second, Sun, Mar, 2, -420);
  ntp.ruleSTD("PST", First, Sun, Nov, 2, -480);
  ntp.begin();

  //setup mdns
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  if (MDNS.begin("scheduler")) Serial.println("MDNS Started");

  //setup paths
  server.on("/", handleRoot);
  server.on("/add", handleAdd);
  server.on("/backlight",handleBackLight);
  server.onNotFound(handleNotFound);

  //TFT init
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(135);
  tft.setTextWrap(false);
  //clear screen
  tft.fillScreen(ST77XX_BLACK);

  //start server
  server.begin();
}

String toPrint = "";
bool side = false;
long ntpUpdateDelay = 0;
long rectFillDelay = 0;
long charFlipDelay = 0;
int anim_frame = 0;
long animDelay = 0;


void loop() {
  //animator
  if (millis() - animDelay > 6) {
    switch (anim_frame) {
      case 0:
        drawPixArr(0, 22, img0);
        break;
      case 1:
        drawPixArr(0, 22, img1);
        break;
      case 2:
        drawPixArr(0, 22, img2);
        break;
      case 3:
        drawPixArr(0, 22, img3);
        break;
      case 4:
        drawPixArr(0, 22, img4);
        break;
      case 5:
        drawPixArr(0,22,img5);
        break;
      case 6:
        drawPixArr(0,22,img6);
        break;
      case 7:
        drawPixArr(0,22,img7);
        break;
      case 8:
        drawPixArr(0,22,img8);
        break;
      case 9:
        drawPixArr(0,22,img9);
        break;
      case 10:
        drawPixArr(0,22,img10);
        break;
      case 11:
        drawPixArr(0,22,img11);
        break;
    }
    if(anim_frame ==11) anim_frame=0;
    else anim_frame+=1;
  }

  // put your main code here, to run repeatedly:
  if (millis() - ntpUpdateDelay > 1000) {
    ntp.update();
    ntpUpdateDelay = millis();
  }
  tft.setCursor(0, 0);
  tft.setTextSize(1);
  tft.setTextColor(0xFFFFFF, 0x000000);
  tft.print(ntp.formattedTime("%A %B %d %Y"));
  tft.setCursor(0, 10);
  tft.print(ntp.formattedTime("%T"));
  tft.print("   ");

  times now = {
    ntp.year(),
    ntp.month(),
    ntp.day(),
    ntp.hours(),
    ntp.minutes(),
    ntp.seconds(),
    ""
  };

  tft.setCursor(0, 74);

  for (int i = 0; i < eventCount; i++) {
    toPrint = "";
    tft.setCursor(0, tft.getCursorY() + i * 5);
    //if (millis()-rectFillDelay>1000){
    //tft.drawRect(0, tft.getCursorY(), tft.width(), tft.height(), 0x000000);
    //  rectFillDelay=millis();
    //}

    times countdown = timeDiff(now, events[i]);
    //print countdown
    if (countdown.year >= 1) {
      //tft.print(countdown.year);
      //tft.print(F("Y"));
      toPrint += countdown.year;
      toPrint += "Y";
    }
    if (countdown.month >= 1) {
      //tft.print(countdown.month);
      //tft.print(F("M"));
      toPrint += countdown.month;
      toPrint += "M";
    }
    //tft.print(countdown.day);
    //tft.print("D ");
    toPrint += countdown.day;
    toPrint += "D ";
    //tft.print(countdown.hour);
    //tft.print(F(":"));
    toPrint += countdown.hour;
    toPrint += ":";
    //tft.print(countdown.minute);
    //tft.print(F(":"));
    toPrint += countdown.minute;
    toPrint += ":";
    //tft.print(countdown.second);
    //tft.print(F(" Until "));
    toPrint += countdown.second;
    toPrint += " Until ";
    //tft.print(countdown.eventName);
    //tft.print(" ");
    toPrint += countdown.eventName;

    //alternate sides when the string will not fit
    if (toPrint.length() > 26) {
      if (millis() - charFlipDelay > 5000) {
        side = !side;
        charFlipDelay = millis();
      }

      if (side) {
        tft.print(toPrint.substring(26));
        for (int s = toPrint.substring(26).length(); s < toPrint.length(); s++) {
          tft.print(" ");
        }
      } else {
        tft.print(toPrint);
      }
    }

    else {
      tft.print(toPrint);
    }


    tft.println();
  }

  //clear screen at 12 am once
  if (ntp.hours() == 0) {
    tft.fillScreen(ST77XX_BLACK);
  }
  server.handleClient();
  MDNS.update();
}
