/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <jni.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include <GLES2/gl2.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android-dl.h>

#define LOG(prio, tag, a, args...) __android_log_print(prio, tag, "[%s::%d]"#a"",__FUNCTION__, __LINE__, ##args);
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

typedef void (*fty_glutMainLoopEvent)();
typedef void (*fty_glutInit)(int*, char**);
typedef void (*fty_glutInitDisplayMode)(unsigned int);
typedef int (*fty_glutCreateWindow)(const char*);
typedef void (*fty_glutDestroyWindow)(int);
typedef void (*fty_glutPostRedisplay)();
typedef void (*fty_glutSwapBuffers)();
typedef int (*fty_glutGetWindow)();
typedef void (*fty_glutSetWindow)(int);
typedef void (*fty_glutReshapeWindow)(int ,int);
typedef void (*fty_glutDisplayFunc)(void (*)());
typedef void (*fty_glutReshapeFunc)(void (*)(int, int));
typedef void (*fty_glutTimerFunc)(unsigned int, void (*)(int), int);
typedef int (*fty_glutGet)(unsigned int);
typedef void (*fty_glutKeyboardFunc)(void (*)(unsigned char, int, int));
typedef void (*fty_glutMouseFunc)(void (*)(int, int, int, int));
typedef void (*fty_glutMouseWheelFunc)(void (*)(int, int, int, int));
typedef void (*fty_glutSetWindowTitle)(char const*);
typedef void (*fty_glutIdleFunc)(void(*)());
typedef void (*fty_glutInitWindowSize)(int, int);
typedef int (*fty_glutGetModifiers)();

#define REGISTER_FUNCTION(lib, function)\
    void (*reg_fn_##function)(fty_##function);\
    *(void**)(&reg_fn_##function) = dlsym(lib, "reg_fn_" #function);\
    if (function == NULL) {\
        LOGW("could not find reg_fn_" #function " from " #lib);\
        return;\
    } else {\
        LOGI("loaded reg_fn_" #function " from " #lib);\
        reg_fn_##function(function);\
        LOGI("registerd "#function);\
    }\

static void init_servo()
{
    LOGI("initializing native application for Servo");

    setenv("RUST_LOG", "servo,gfx,msg,util,script,layers,js,glut,std,rt,extra", 1);

//    setenv("SERVO_URL", "/mnt/sdcard/html/demo.html", 1);
//    setenv("RUST_THREADS", "1", 1);
    
//    char* size_stack = getenv("RUST_MIN_STACK");
//    char* rust_log = getenv("RUST_LOG");
//    char* servo_url = getenv("SERVO_URL");

//    LOGI("Stack Size is : %s", size_stack);
//    LOGI("RUST_LOG flag is : %s", rust_log);
//    LOGI("loading url is : %s", servo_url);
    

    LOGI("load servo library");
    void* libservo = android_dlopen("/data/data/com.example.ServoAndroid/lib/libservo.so");
    if (libservo == NULL) {
        LOGW("failed to load servo lib: %s", dlerror());
        return;
    }
    void* crate_map = dlsym(libservo, "_rust_crate_map_toplevel");
    if (crate_map == NULL) {
        LOGW("failed to load the crate map from the servo lib: %s", dlerror());
        return;
    }

    LOGI("load rust-glut library");
    void* libglut = android_dlopen("/data/data/com.example.ServoAndroid/lib/libglut-f186cebf-0.1.so");
    if (libglut == NULL) {
        LOGW("failed to load rust-glut lib: %s", dlerror());
        return;
    }

    REGISTER_FUNCTION(libglut, glutMainLoopEvent);
    REGISTER_FUNCTION(libglut, glutInit);
    REGISTER_FUNCTION(libglut, glutInitDisplayMode);
    REGISTER_FUNCTION(libglut, glutCreateWindow);
    REGISTER_FUNCTION(libglut, glutDestroyWindow);
    REGISTER_FUNCTION(libglut, glutPostRedisplay);
    REGISTER_FUNCTION(libglut, glutSwapBuffers);
    REGISTER_FUNCTION(libglut, glutGetWindow);
    REGISTER_FUNCTION(libglut, glutSetWindow);
    REGISTER_FUNCTION(libglut, glutReshapeWindow);
    REGISTER_FUNCTION(libglut, glutDisplayFunc);
    REGISTER_FUNCTION(libglut, glutReshapeFunc);
    REGISTER_FUNCTION(libglut, glutTimerFunc);
    REGISTER_FUNCTION(libglut, glutGet);
    REGISTER_FUNCTION(libglut, glutKeyboardFunc);
    REGISTER_FUNCTION(libglut, glutMouseFunc);
    REGISTER_FUNCTION(libglut, glutMouseWheelFunc);
    REGISTER_FUNCTION(libglut, glutSetWindowTitle);
    REGISTER_FUNCTION(libglut, glutIdleFunc);
    REGISTER_FUNCTION(libglut, glutInitWindowSize);
    REGISTER_FUNCTION(libglut, glutGetModifiers);

    
    LOGI("load libstd library to set the cratemap");
    void* libstd = android_dlopen("/data/data/com.example.ServoAndroid/lib/libstd-966edb7e-0.10-pre.so");
    if (libstd == NULL) {
        LOGW("failed to load libstd: %s", dlerror());
        return;
    }
    
    void (*rust_set_crate_map)(void*) = (void (*)(void*))dlsym(libstd, "rust_set_crate_map");
    if (rust_set_crate_map == NULL) {
        LOGW("failed to load rust_set_crate_map from libstd: %s", dlerror());
        return;
    }
    rust_set_crate_map(crate_map);

    void (*main)(int, char**);
    *(void**)(&main) = dlsym(libservo, "android_start");
    if (main) {
        LOGI("go into android_start()");
        static char* argv[] = {"servo", "/mnt/sdcard/html/about-mozilla.html"};
        (*main)(2, argv);
        return;
    }
    LOGW("could not find android_start() in the libServo shared library");
}

extern "C" void *stderr_thread(void *) {
    int pipes[2];
    pipe(pipes);
    dup2(pipes[1], STDERR_FILENO);
    FILE *inputFile = fdopen(pipes[0], "r");
    char readBuffer[1024];
    while (1) {
        fgets(readBuffer, sizeof(readBuffer), inputFile);
        __android_log_write(2, "stderr", readBuffer);
    }
        return NULL;
}

extern "C" void *stdout_thread(void *) {
    int pipes[2];
    pipe(pipes);
    dup2(pipes[1], STDOUT_FILENO);
    FILE *inputFile = fdopen(pipes[0], "r");
    char readBuffer[1024];
    while (1) {
        fgets(readBuffer, sizeof(readBuffer), inputFile);
        __android_log_write(2, "stdout", readBuffer);
    }
        return NULL;
}

pthread_t stderr_tid = -1;
pthread_t stdout_tid = -1;

static void init_std_threads() {
  pthread_create(&stderr_tid, NULL, stderr_thread, NULL);
  pthread_create(&stdout_tid, NULL, stdout_thread, NULL);
}

static void shutdown_std_threads() {
  // FIXME(larsberg): this needs to change to signal the threads
  // to exit, as pthread_cancel is not implemented on Android.
}


const int W = 2560;
const int H = 1600;

static int init_display() {
    LOGI("initialize GLUT window");

    glutInitWindowSize(W, H);
    return 0;
}

int main(int argc, char* argv[])
{
    init_display();
    init_std_threads();
    init_servo();
    shutdown_std_threads();

    return 0;
}
