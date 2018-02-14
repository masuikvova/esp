#include <RTClib.h>
RTC_DS1307 rtc;
void setupRTC();
void setupDataTime(int _year, int _month, int _day, int _hour, int _minutes);

void setupRTC() {
  rtc.begin();
 if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(__DATE__, __TIME__)); // Use ONCE then comment out and reload
  }
}

void setupDataTime(int _year, int _month, int _day, int _hour, int _minutes) {
  rtc.adjust(DateTime(_year, _month, _day, _hour, _minutes, 0)); // Set time manually ONCE as above
}

