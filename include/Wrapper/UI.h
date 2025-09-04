#ifndef UI
#define UI
#include <Wrapper/IO.h>
#include <Layer_File.h>

enum WindowID {
    UI_PROG,
    UI_LOGS,
    UI_IR,
    UI_MC,
    UI_CNT,
};

void onStartup(File_Context* ctx);

void onShutdown();

#endif