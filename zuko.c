#include "include/repl.h"
#include "include/misc.h"


int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        repl_init();
        repl();
        return 0;
    }
    char* source_code = NULL;
    const char* filename;
    if (argc >= 3 && strncmp(argv[1], "-c", 2) == 0)
        run_zuko_code("argv",strdup(argv[2]), argc, argv);
    else
        run_zuko_file(argv[1],argc,argv);
    // Hasta La Vista Baby
    return 0;
}
