
void init_logger_screen(void); // [miml:init]
void finalize_logger_screen(void); // [miml:final]
extern void getMessage_logger_screen(const char *src, unsigned char *buffer, int length); // [miml:receiver]
extern void getTemp_logger_screen(const char *src, char *buffer, int length); // [miml:receiver]
extern void getMouseMessage_logger_screen(const char *src, unsigned char *buffer, int length); // [miml:receiver]
