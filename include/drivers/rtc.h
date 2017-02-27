#ifndef _RTC_H_
#define _RTC_H_

struct time {
	uint16_t year;
	uint16_t month;
	uint16_t day;
	uint16_t hour;
	uint16_t minute;
	uint16_t second;
};

size_t rtc_time(struct time* t);

#endif