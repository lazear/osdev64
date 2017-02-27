/* Michael Lazear (C) 2007 */

#include <arch/x86_64/kernel.h>
#include <drivers/rtc.h>
#include <stdint.h>

#define RTC_BASE	0x70
#define RTC_DATA	0x71
#define RTC_SEC		0x00
#define RTC_MIN		0x02
#define RTC_HOUR	0x04
#define RTC_DAY		0x07
#define RTC_MON		0x08
#define RTC_YEAR	0x09

static uint32_t rtc_read(uint8_t b) {
	outb(RTC_BASE, b);
	uint8_t in = inb(RTC_DATA);
	uint32_t out;
	out = ((in >> 4) & 0x0F) * 10;
	out += (in & 0x0F);
	return out;
}

size_t rtc_time(struct time* time) {
	size_t t;
	uint32_t i;
	uint32_t sec = rtc_read(RTC_SEC);
	uint32_t min = rtc_read(RTC_MIN);
	uint32_t hour = rtc_read(RTC_HOUR);
	uint32_t day = rtc_read(RTC_DAY);
	uint32_t mon = rtc_read(RTC_MON);
	uint32_t year = rtc_read(RTC_YEAR);

	uint32_t months[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 304, 334};

	if (year < 70) 
		year += 100;	
	year += 1900;

	time->year = year;
	time->month = mon;
	time->day = day;
	time->hour = hour;
	time->minute = min;
	time->second = sec;

	for (i = 1970; i < year; i++)
	{
		if ((i % 4) != 0)
			day += 365;
		else if ((i % 400) != 0)
			day += 366;
		else if ((i % 100) != 0)
			day += 365;
		else 
			day += 366;
	}

	day += months[mon-1];
	day++;

	t = day * 24;	// hours
	t += hour;		// current hour
	t *= 60;			// minutes
	t += min;		// current minute
	t *= 60;			// seconds
	t += sec;		// current second
	return t;
}


