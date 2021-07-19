/**
 * test_shapefile_jniwrapper.c
 *   A test app using shapefile_jniwrapper.
 *   https://blog.csdn.net/qq_32583189/article/details/53172316
 */
#include <common/mscrtdbg.h>
#include <common/cstrbuf.h>
#include <common/emerglog.h>
#include <common/jniutils.h>

#include <com_github_jni_libshapefile_JNIWrapper.h>

static const char THIS_FILE[] = "test_shapefile_jniwrapper.c";

static struct JNIRuntimeContext jniCtx = {0, NULL, NULL, NULL};

static void appexit_cleanup(void)
{
    JNIRuntimeContextUninit(&jniCtx);
}


int main(int argc, char *argv[])
{
    WINDOWS_CRTDBG_ON

    if (JNIRuntimeContextInit(1, 8, &jniCtx) == 0) {
        atexit(appexit_cleanup);

        Java_com_github_jni_libshapefile_JNIWrapper_JNI_1shapefile_1lib_1version(jniCtx.jenv, NULL);
    }

    return 0;
}