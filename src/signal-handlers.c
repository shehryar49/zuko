#include "signal-handlers.h"
#include <signal.h>
#include <stdlib.h>

#ifndef _WIN32
    #include <unistd.h>
#endif

void signal_handler(int signum)
{
    if (signum == SIGABRT || signum == SIGFPE || signum == SIGILL || signum == SIGSEGV)
    {
        char buff[] = "Oops! Either the interpreter or one of the loaded modules just crashed. Please report this incident.\n";
        #ifdef _WIN32
            size_t written = _write(_fileno(stderr), buff, sizeof(buff));
        #else
            size_t written __attribute__((unused)) =
            write(STDERR_FILENO, buff, sizeof(buff));
        #endif
        exit(EXIT_FAILURE);
    }
}

void setup_signal_handlers()
{
    // Handle crashes
    // Very unlikely but ...
    // Shit happens 
    signal(SIGFPE, signal_handler);
    signal(SIGILL, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGSEGV, signal_handler); 
}
