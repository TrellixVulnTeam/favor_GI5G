#include <jni.h>
#include <vector>
#include "../favor.h"
#include <string>

extern "C" {

//Java, then package name, then class name, then method name, separated by underscores (also underscores per "." in the package name)
//Basically if you get an error calling this stuff, the solution is replacing every . in the error with an underscore

#define CLASS_PATH "com/favor/lib/"
const char* coreClassPath = CLASS_PATH "Core";
const char* readerClassPath = CLASS_PATH "Reader";
const char* writerClasspath = CLASS_PATH "Writer";

JNIEXPORT jstring
JNICALL
helloWorld(JNIEnv* env, jobject obj){
    return env->NewStringUTF("Hello from Favor's native interface!");
}

static JNINativeMethod coreMethodTable[] = {
    {"helloWorld", "()Ljava/lang/String;", (void *) helloWorld}
};

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    jclass core = env->FindClass(coreClassPath);
    jclass reader = env->FindClass(readerClassPath);
    jclass writer = env->FindClass(writerClasspath);

    //TODO: write something here before we fail
    if (!core) return -1;
    if (!reader) return -1;
    if (!writer) return -1;
    // Get jclass with env->FindClass.
    // Register methods with env->RegisterNatives.

    env->RegisterNatives(core, coreMethodTable, sizeof(coreMethodTable) / sizeof(coreMethodTable[0]));

    return JNI_VERSION_1_6;
}

}
