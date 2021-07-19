/**
 * jniutils.h
 *   JNI utility api. 
 */
#include "misc.h"
#include <jni.h>

#ifndef _JNI_UTILS_H_
#define _JNI_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

struct JNIRuntimeContext
{
    jint jniVersion;
    HINSTANCE jvmdll;
    JavaVM * jvm;
    JNIEnv * jenv;
};


NOWARNING_UNUSED(static)
int JNIRuntimeContextInit (int jniMajorVersion, int jniMinorVersion, struct JNIRuntimeContext *jniCtx)
{
    typedef jint(JNICALL *ProcCreateJavaVM)(JavaVM **, void**, void *);

    jint jniVersion = JNI_VERSION_1_8;
    HINSTANCE jvmdll = NULL;
    JavaVM * jvm = NULL;
    JNIEnv * jenv = NULL;

    jint res;

    ProcCreateJavaVM jvm_CreateJavaVM;

    char javahome[200];
    char jvmdllpath[260];
    char classpath[1200];

    JavaVMInitArgs vm_args;
    JavaVMOption options[30];

    memset(&vm_args, 0, sizeof(vm_args));
    memset(&options, 0, sizeof(options));

    jniCtx->jniVersion = 0;

    res = getenv_with_prefix("JAVA_HOME", NULL, javahome, sizeof(javahome));
    if (! res) {
        printf("env not found: JAVA_HOME\n");
        return (-1);
    }
    if (res < 0) {
        printf("insufficent buf for env: JAVA_HOME\n");
        return (-1);
    }

    res = getenv_with_prefix("CLASSPATH", "-Djava.class.path=", classpath, sizeof(classpath));
    if (! res) {
        printf("env not found: CLASSPATH\n");
        return (-1);
    }
    if (res < 0) {
        printf("insufficent buf for env: CLASSPATH\n");
        return (-1);
    }

    snprintf_chkd_V1(jvmdllpath, sizeof(jvmdllpath), "%s\\jre\\bin\\server\\jvm.dll", javahome);

    // 注意: 务必使用动态载入 jvm.dll 方式调用 JNI_CreateJavaVM
    jvmdll = LoadLibraryA(jvmdllpath);
    if (!jvmdll) {
        printf("failed to load: %s\n", jvmdllpath);
        return (-1);
    }

    printf("success load: %s\n", jvmdllpath);

    jvm_CreateJavaVM = (ProcCreateJavaVM) GetProcAddress(jvmdll, "JNI_CreateJavaVM");
    if (!jvm_CreateJavaVM) {
        printf("failed to GetProcAddress: JNI_CreateJavaVM\n");
        FreeLibrary(jvmdll);
        return (-1);
    }

    options[0].optionString = "-Djava.compiler=NONE";
    options[1].optionString = classpath;
    options[2].optionString = "-verbose:jni";

    if (jniMajorVersion && jniMinorVersion) {
        jniVersion = (jniMajorVersion << 16) | jniMinorVersion;
    }

    vm_args.version = jniVersion;
    vm_args.ignoreUnrecognized = JNI_TRUE;
    vm_args.options = options;
    vm_args.nOptions = 3;

    // 务必关闭异常: (VS2015: Ctrl+Alt+E -> Win32 Exceptioins -> 0xc0000005 Access violation)
    //   https://stackoverflow.com/questions/36250235/exception-0xc0000005-from-jni-createjavavm-jvm-dll
    res = jvm_CreateJavaVM(&jvm, (void**)&jenv, &vm_args);
    if (res == 0) {
        printf("successfully created JVM.\n");
        jniCtx->jvmdll = jvmdll;
        jniCtx->jvm = jvm;
        jniCtx->jenv = jenv;
        jniCtx->jniVersion = jniVersion;
        return 0;
    } else {
        printf("failed to create JVM.\n");
        FreeLibrary(jvmdll);
        return (-1);
    }
}


NOWARNING_UNUSED(static)
void JNIRuntimeContextUninit (struct JNIRuntimeContext *jniCtx)
{
    JavaVM * jvm = jniCtx->jvm;

    if (jniCtx->jniVersion) {
        jniCtx->jniVersion = 0;

        if (jvm) {
            (*jvm)->DetachCurrentThread(jniCtx->jvm);
            (*jvm)->DestroyJavaVM(jniCtx->jvm);
            FreeLibrary(jniCtx->jvmdll);
        }
    }
}

#ifdef __cplusplus
}
#endif

#endif  /* _JNI_UTILS_H_ */
