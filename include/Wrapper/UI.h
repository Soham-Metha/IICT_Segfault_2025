#ifndef UI
#define UI
#include <Wrapper/IO.h>
#include <Frontend/Layer_File.h>

enum {
    UI_PROG,
    UI_LOGS,
    UI_TOKEN_CHART,
    UI_CNT
};


void onStartup(File_Context* ctx);

void onShutdown();

#endif