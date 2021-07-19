/**
 * shapefile_jniwrapper.c
 */
#include <com_github_jni_libshapefile_JNIWrapper.h>

#include <shapefile/shapefile_api.h>


void JNICALL Java_com_github_jni_libshapefile_JNIWrapper_JNI_1shapefile_1lib_1version(JNIEnv *env, jobject obj)
{
    const char *libname;
    const char * libversion = shapefile_lib_version(&libname);

    printf("[%s:%d %s] %s-%s\n", __FILE__, __LINE__, __FUNCTION__, libname, libversion);
}