#ifndef favor_jni_accountmanager_include
#define favor_jni_accountmanager_include
#include <jni.h>
#include "../accountmanager.h"
#include "../reader.h"

/*
    public native void fetch() throws FavorException; //TODO: this is also overriden for AndroidTextManagers

    public native void delete() throws FavorException; //TODO:
 */


namespace favor{
    namespace jni{

        JNIEXPORT jobject JNICALL create(JNIEnv* env, jobject obj, jstring name, jint type, jstring detailsJson) {
            const char* nameChars = env->GetStringUTFChars(name, NULL);
            const char* jsonChars = env->GetStringUTFChars(detailsJson, NULL);
            string nameString(nameChars);
            string detailsJsonString(jsonChars);
            //TODO: catch any exceptions this might throw and rethrow them as a java favorException
            AccountManager::addAccount(nameString, (favor::MessageType) type, detailsJsonString);

    }
}
}

#endif