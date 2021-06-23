/**
 * app_main.c
 *   A test app using libshapefile.
 */
#include "app_incl.h"

static const char THIS_FILE[] = "app_main.c";

#ifdef HAS_LIBCLOGGER
    clog_logger app_logger = NULL;
#endif


static void appexit_cleanup(void)
{
#ifdef HAS_LIBCLOGGER
    logger_manager_uninit();
#endif
    app_opts_uninit();
}


int main(int argc, char *argv[])
{
    WINDOWS_CRTDBG_ON

    parse_arguments(argc, argv);

#ifdef HAS_LIBCLOGGER
    logger_manager_init(NULL, cstrbufGetStr(app_opts.ident), 0);
#endif

    atexit(appexit_cleanup);

#ifdef HAS_LIBCLOGGER
    app_logger = logger_manager_load(NULL);
#endif



#ifdef HAS_LIBCLOGGER
    LOGGER_INFO(app_logger, "app end.");
#endif

    system("pause");
    return 0;
}