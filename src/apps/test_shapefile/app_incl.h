/**
 * app_incl.h
 *   app include header file
 */
#ifndef _APP_INCL_H_
#define _APP_INCL_H_

#ifdef    __cplusplus
extern "C" {
#endif

#include <common/mscrtdbg.h>
#include <common/cstrbuf.h>
#include <common/emerglog.h>

#ifdef HAS_LIBCLOGGER
#   include <clogger/logger_api.h>
#endif

/* using pthread or pthread-w32 */
#include <sched.h>
#include <pthread.h>

/* shapefile api */
#include <shapefile/shapefile_api.h>


#ifdef __WINDOWS__
    # include <Windows.h>
    # include <common/win32/getoptw.h>

    # if !defined(__MINGW__)
        // link to pthread-w32 lib for MS Windows
        #pragma comment(lib, "pthreadVC2.lib")

        // link to libclogger.lib for MS Windows
        #pragma comment(lib, "libclogger.lib")
    # endif
#else
    # include <getopt.h>
#endif


static const char APP_NAME[]     = "test_shapefile";
static const char APP_VERSION[]  = "0.0.1" ;

#ifdef HAS_LIBCLOGGER
    extern clog_logger app_logger;
#endif

static void app_opts_init(void);
static void app_opts_uninit(void);


struct app_opts_t
{
    /* clogger ident for app */
    cstrbuf ident;

    /* TODO: add your options */
    //->
} app_opts;


static void app_opts_init()
{
    bzero(&app_opts, sizeof(app_opts));

    app_opts.ident = cstrbufNew(0, APP_NAME, -1);

    /* TODO: free your options */
    //->
}


static void app_opts_uninit(void)
{
    cstrbufFree(&app_opts.ident);

    /* TODO: free your options */
    //->
}


static void print_version()
{
    #ifdef NDEBUG
        #ifdef DEBUG
        # undef DEBUG
        #endif
        
        #ifdef _DEBUG
        # undef _DEBUG
        #endif

        printf("\033[47;35m%s, NDEBUG Version: %s, Build: %s %s\033[0m\n", APP_NAME, APP_VERSION, __DATE__, __TIME__);
    #else
        #ifndef DEBUG
        # define DEBUG  1
        #endif
        
        #ifndef _DEBUG
        # define _DEBUG  1
        #endif

        printf("\033[47;35m%s, DEBUG Version: %s, Build: %s %s\033[0m\n", APP_NAME, APP_VERSION, __DATE__, __TIME__);
    #endif

    exit(0);
}


static void print_usage()
{
    printf("\033[35mUsage:\033[0m %s [Options]\n", APP_NAME);

    printf("\033[35mOptions:\033[0m\n");
    printf("  %-30s  \033[35m show app help \033[0m\n", "-h, --help");
    printf("  %-30s  \033[35m show app version \033[0m\n", "-V, --version");

    /* TODO: print other options */
    //->

    exit(0);
}


static void parse_arguments (int argc, char *argv[])
{
    int opt, optindex, flag;

    const struct option longopts[] = {
        {"help", no_argument, 0, 'h'}
        ,{"version", no_argument, 0, 'V'}
        ,{"req-arg", required_argument, &flag, 1000 }
        ,{"opt-arg", optional_argument, &flag, 1001 }
        ,{0, 0, 0, 0}
    };

    app_opts_init();

    while ((opt = getopt_long_only(argc, argv, "hV", longopts, &optindex)) != -1) {
        switch (opt) {
        case '?':
            fprintf(stderr, "\033[1;31m[error]\033[0m option not defined.\n");
            exit(EXIT_FAILURE);
            break;

        case 'h':
            print_usage();
            break;
        
        case 'V':
            print_version();
            break;

        case 0:
            switch(flag) {
            case 1000:
                // req-arg = optarg
                break;
            case 1001:
                // opt-arg = optarg
                break;
            /* TODO: parse other options */
            //->

            }
            break;
        }
    }
}


#ifdef    __cplusplus
}
#endif

#endif /* _APP_INCL_H_ */