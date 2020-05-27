#include "AdmTime.h"
#include <ctime>

// TODO: make a namespace for this
// TODO: nuke or do sth with chrono

bool isDate(int year, int month, int day)
{
	time_t tTime = time(0);
	struct tm *stTimeNow = localtime(&tTime);

	year -= 1900;

	if (year > 0 && stTimeNow->tm_year != year)
			return false;

	if (month > 0 && ((stTimeNow->tm_mon + 1) != month))
			return false;

	if (day > 0 && stTimeNow->tm_mday != day)
			return false;
	else
		return true;
}

void getDate(int &year, int &month, int &day)
{
	time_t tTime = time(0);
	struct tm *stTimeNow = localtime(&tTime);

	year = stTimeNow->tm_year + 1900;
	month = stTimeNow->tm_mon + 1;
	day = stTimeNow->tm_mday;
}

bool isTime(int hour, int minute, int second)
{
	time_t tTime = time(0);
	struct tm *stTimeNow = localtime(&tTime);

	if (hour >= 0 && stTimeNow->tm_hour != hour)
		return false;

	if (minute >= 0 && stTimeNow->tm_min != minute)
		return false;

	if (second >= 0 && stTimeNow->tm_sec != second)
		return false;

	return true;
}

void getTime(int &hour, int &minute, int &second)
{
	time_t tTime = time(0);
	struct tm *stTimeNow = localtime(&tTime);

	hour = stTimeNow->tm_hour;
	minute = stTimeNow->tm_min;
	second = stTimeNow->tm_sec;
}
