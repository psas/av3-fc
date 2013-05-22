#include <cstdio>
#include <cstdarg>
#include <string>

const short NONE = -1;
const short BOLD = 1;
const short UNDERSCORE = 4;
const short BLINK = 5;
const short REVERSE = 7;
const short CONCEAL = 8;
const short BLACK = 30;
const short RED = 31;
const short GREEN = 32;
const short YELLOW = 33;
const short BLUE = 34;
const short MAGENTA = 35;
const short CYAN = 36;
const short WHITE = 37;

int color_print(short, short, short, std::string, ...);

int rgb_info(std::string, ...);
int rgb_debug(std::string, ...);
int rgb_warn(std::string, ...);
int rgb_error(std::string, ...);
int rgb_success(std::string, ...);
int rgb_critical(std::string, ...);


