// demo test
// - rlyeh, public domain

#define ALL_C
#include "ds/all.c"

#define APP_TITLE "demo"
#define APP_VERSION 0100

int main() {
    LOGINFO(#init, "%s v%o", APP_TITLE, APP_VERSION);

    //event_loop();

    LOGINFO(#quit, "done.");
}
