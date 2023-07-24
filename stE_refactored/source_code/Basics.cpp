#include "Basics.h"
#include <time.h>

std::string get_date_string() {
	char buffer[80];
	time_t seconds = time(NULL);
	tm timeinfo;
	localtime_s(&timeinfo, &seconds);
	static const char* format = "%x";
	strftime(buffer, 80, format, &timeinfo);
	return std::string(buffer);
}

std::string get_time_string() {
	char buffer[80];
	time_t seconds = time(NULL);
	tm timeinfo;
	localtime_s(&timeinfo, &seconds);
	static const char* format = "%X";
	strftime(buffer, 80, format, &timeinfo);
	return std::string(buffer);
}