#include <math.h>
#include <string.h>

#include "helpers.h"

double rad_to_deg(double rad)
{
	double deg = rad * 180.0 / M_PI;
	return deg;
}

inline bool streq(const char *str1, const char *str2)
{
  return !strcmp(str1, str2);
}
