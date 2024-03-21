#include <chrono>
#include <sstream>
#include <cstdlib>

using namespace std;

chrono::duration<int> string_to_duration(const string& src)
{
  chrono::duration<int> duration(0);
  istringstream stream(src);

  while (stream.peek() > 0)
  {
    unsigned int number;
    char unit = 's';
    stream >> number;
    if (stream.peek() > 0)
      stream >> unit;
    switch (unit)
    {
    case 's': duration += chrono::seconds(number); break;
    case 'm': duration += chrono::minutes(number); break;
    case 'h': duration += chrono::hours(number); break;
    case 'd': duration += chrono::days(number); break;
    }
  }
  return duration;
}

chrono::duration<int> duration_from_env(const char* varname, chrono::duration<int> default_value)
{
  char* variable = getenv(varname);

  return variable ? string_to_duration(variable) : default_value;
}
