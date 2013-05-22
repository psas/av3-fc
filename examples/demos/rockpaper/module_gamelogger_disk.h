
extern void init_gamelogger_disk(void); // [miml:init]
extern void getGameMessage_gamelogger_disk(char *src, char *buffer, int len); // [miml:receiver]
extern void getMouseMessage_gamelogger_disk(const char *src, unsigned char *buffer, int len); // [miml:receiver]
extern void finalize_gamelogger_disk(void); // [miml:final]
