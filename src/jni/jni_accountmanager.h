#ifndef favor_jni_accountmanager_include
#define favor_jni_accountmanager_include
#include <jni.h>
#include "jni_exceptions.h"
#include "../accountmanager.h"
#include "../reader.h"
#include "../worker.h"

/*
    public native void fetch() throws FavorException; //TODO: this is also overriden for AndroidTextManagers

    public native void delete() throws FavorException; //TODO:
 */


namespace favor{
    namespace jni{

        JNIEXPORT jobject JNICALL create(JNIEnv* env, jobject callingObj, jstring name, jint type, jstring detailsJson) {
            const char* nameChars = env->GetStringUTFChars(name, NULL);
            const char* jsonChars = env->GetStringUTFChars(detailsJson, NULL);
            string nameString(nameChars);
            string detailsJsonString(jsonChars);
            logger::info("Create account "+nameString+" of type "+MessageTypeName[(int)type]);
            try {
                //TODO: MUCH TESTCODE, LOGGING AND TRUNCATION TO BE REMOVED
                logger::info("BEFORE DB HIT, truncating tables");
                worker::truncateDatabase();
                logger::info("POST TRUNCATE");
                AccountManager::addAccount(nameString, (favor::MessageType) type, detailsJsonString);
                logger::info("AFTER DB HIT");
            }
            catch (favor::exception &e){
                //I assume this cast is safe...
                throwFavorException(env, e.what());
                return NULL;
            }

            jclass accountManager = env->FindClass("com/favor/library/AccountManager");
            jmethodID accountManagerConstructor = env->GetMethodID(accountManager, "<init>", "(Ljava/lang/String;I)V");
            if (accountManager == NULL || accountManagerConstructor == NULL){
                logger::error("Could not find Accountmanager class and/or its constructor");
                throwFavorException(env, "Could not find Accountmanager class and/or its constructor");
                return NULL;
            }

            jclass androidManager = env->FindClass("com/favor/library/AndroidTextManager");
            jmethodID androidManagerConstructor = env->GetMethodID(androidManager, "<init>", "(Ljava/lang/String;)V");
            if (androidManager == NULL | androidManagerConstructor == NULL){
                logger::error("Could not find AndroidTextManager class and/or its constructor");
                throwFavorException(env, "Could not find AndroidTextManager class and/or its constructor");
                return NULL;
            }

            jobject obj;
            if (type == TYPE_ANDROIDTEXT){
                obj = env->NewObject(androidManager, androidManagerConstructor, name);
            } else {
                obj = env->NewObject(accountManager, accountManagerConstructor, name, type);
            }

            if (obj == NULL || env->ExceptionOccurred()){
                logger::error("Error instantiating account "+nameString);
                env->ExceptionClear();
            }

            return obj;
        }

        static JNINativeMethod accountManagerMethodTable[] = {
                {"create", "(Ljava/lang/String;ILjava/lang/String;)L" CLASS_PATH "AccountManager;", (void*) create}
        };
}
}

#endif