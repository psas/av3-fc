#include "coloredprint.h"
#include <iostream>
#include <cstdio>
using namespace std;

const int VA_SIZE = 100;

int color_print(const short text_attrs, const short background,
    const short foreground, string arg){

    if (!(foreground == NONE && background == NONE && text_attrs == NONE))
        cout << "\033[";

    if (text_attrs != NONE)
        cout << text_attrs << ";";
    if (foreground != NONE)
        cout << (foreground) << ";";
    if (background != NONE)
        cout << (background+10);

    cout << arg;
}


int rgb_info(string format,...){
    va_list args;
    string arg = NULL;
    va_start(args,VA_SIZE);
    arg = va_arg(args, char*);
    color_print(NONE, NONE, BLUE, format, arg);
    va_end(args);
    cout << "\033[;0m" << endl;
}
int rgb_debug(std::string format, ...){
    va_list args;
    va_start(args,VA_SIZE);
    string arg = va_arg(args, char*);
    color_print(NONE, NONE, CYAN, format, &args);
    va_end(args);
    cout << "\033[;0m" << endl;
}
int rgb_warn(std::string format, ...){
    va_list args;
    va_start(args,VA_SIZE);
    string arg = va_arg(args, char*);
    color_print(BOLD, NONE, YELLOW, format, args);
    va_end(args);
    cout << "\033[;0m" << endl;
}
int rgb_error(std::string format, ...){
    va_list args;
    va_start(args,VA_SIZE);
    string arg = va_arg(args, char*);
    color_print(BOLD, RED, WHITE, format, args);
    va_end(args);
    cout << "\033[;0m" << endl;
}
int rgb_success(std::string &format, ...){
    va_list args;
    va_start(args,VA_SIZE);
    string arg = va_arg(args, char*);
    color_print(NONE, NONE, GREEN, format, args);
    va_end(args);
    cout << "\033[;0m" << endl;
}
