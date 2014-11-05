#include <jni.h>
#include <sys/stat.h>

#include "jni_reader.h"
#include "jni_worker.h"
#include "jni_accountmanager.h"
#include "../logger.h"

extern "C" {

//Java, then package name, then class name, then method name, separated by underscores (also underscores per "." in the package name)
//Basically if you get an error calling this stuff, the solution is replacing every . in the error with an underscore

const char* coreClassPath = CLASS_PATH "Core";
const char* readerClassPath = CLASS_PATH "Reader";
const char* writerClassPath = CLASS_PATH "Writer";
const char* accountManagerClassPath = CLASS_PATH "AccountManager";

JNIEXPORT jstring JNICALL helloWorld(JNIEnv* env, jobject obj){
    return env->NewStringUTF("Hello from Favor's native interface!");
}

JNIEXPORT void JNICALL init(JNIEnv* env, jobject obj, jstring path, jboolean first){
    favor::dbPath = env->GetStringUTFChars(path, 0);

    if (first){
        struct stat sb;
        int res = stat(favor::dbPath, &sb);
        if (res == 0 && sb.st_mode && S_IFDIR){
        favor::logger::info("Database directly already exists");
        }
        else if (ENOENT == errno){
        res = mkdir(favor::dbPath, 0770);
        }
    }
    //TODO: check results, make sure they're good
    favor::initialize();
    if (first) favor::worker::buildDatabase();
    favor::reader::refreshAll();

}

JNIEXPORT void JNICALL cleanup(JNIEnv* env, jobject obj){
    favor::cleanup();
}

static JNINativeMethod coreMethodTable[] = {
        {"helloWorld", "()Ljava/lang/String;", (void*) helloWorld},
        {"init", "(Ljava/lang/String;Z)V", (void*) init},
        {"cleanup", "()V", (void*) cleanup}
};

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    jclass core = env->FindClass(coreClassPath);
    jclass reader = env->FindClass(readerClassPath);
    jclass writer = env->FindClass(writerClassPath);
    jclass accountManager = env->FindClass(accountManagerClassPath);

    if (!core){
        favor::logger::error("JNI could not find core favor library class");
        favor::jni::throwFavorException(env, "Could not initialize Favor: missing class");
        return -1;
    }
    if (!reader){
        favor::logger::error("JNI could not find reader favor library class");
        favor::jni::throwFavorException(env, "Could not initialize Favor: missing class");
        return -1;
    }
    if (!writer) {
        favor::logger::error("JNI could not find writer favor library class");
        favor::jni::throwFavorException(env, "Could not initialize Favor: missing class");
        return -1;
    }
    if (!accountManager){
        favor::logger::error("JNI could not find accountManager library class");
        favor::jni::throwFavorException(env, "Could not initialize Favor: missing class");
        return -1;
    }

    env->RegisterNatives(core, coreMethodTable, sizeof(coreMethodTable) / sizeof(coreMethodTable[0]));
    env->RegisterNatives(reader, favor::jni::readerMethodTable, sizeof(favor::jni::readerMethodTable) / sizeof (favor::jni::readerMethodTable[0]));
    env->RegisterNatives(accountManager, favor::jni::accountManagerMethodTable, sizeof(favor::jni::accountManagerMethodTable) / sizeof(favor::jni::accountManagerMethodTable[0]));

    return JNI_VERSION_1_6;
}

}
