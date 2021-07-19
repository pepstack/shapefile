/**
 * JNIWrapper.java
 *
 *   $ cd $projectdir/jni/src/main/java
 *   $ javah -classpath . -jni com.github.jni.libshapefile.JNIWrapper
 */
package com.github.jni.libshapefile;

import com.github.jni.JNIUtils;


public class JNIWrapper {

    static {
        final String jniLibsPrefix = JNIUtils.getJniLibsPrefix(true);

        System.load(JNIUtils.concatPaths(jniLibsPrefix, "libshapefile_dll.dll"));
        System.load(JNIUtils.concatPaths(jniLibsPrefix, "shapefile_jniwrapper.dll"));
    }


    public native void JNI_shapefile_lib_version();


    public static void main(String[] args) {
        JNIWrapper jniCall = new JNIWrapper();

        jniCall.JNI_shapefile_lib_version();
    }
}