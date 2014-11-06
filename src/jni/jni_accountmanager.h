#ifndef favor_jni_accountmanager_include
#define favor_jni_accountmanager_include
#include <jni.h>
#include "jni_exceptions.h"
#include "jni_reader.h"
#include "../accountmanager.h"
#include "../reader.h"
#include "../worker.h"



namespace favor{
    namespace jni{

        JNIEXPORT void JNICALL _destroy(JNIEnv* env, jobject callingObj){
            jclass clazz = env->FindClass("com/favor/library/AccountManager");

            jfieldID nameFieldID = env->GetFieldID(clazz, "accountName", "Ljava/lang/String;");
            jstring jstr = (jstring) env->GetObjectField(callingObj, nameFieldID);

            jfieldID typeFieldID = env->GetFieldID(clazz, "type", "I");
            jint type = env->GetIntField(callingObj, typeFieldID);

            if (jstr == NULL || env->ExceptionOccurred()){
                logger::error("_delete could not get account name in JNI");
                env->ExceptionClear();
                throwFavorException(env, "Delete method could not get account name");
                return;
            }

            string name = env->GetStringUTFChars(jstr, NULL);
            jniExcept(
                    AccountManager* acc = findAccountManager(name, (MessageType)type);
                    acc->destroy();
            )

        }

        JNIEXPORT void JNICALL _update(JNIEnv* env, jobject callingObj, jboolean contacts){
            jclass clazz = env->FindClass("com/favor/library/AccountManager");

            jfieldID nameFieldID = env->GetFieldID(clazz, "accountName", "Ljava/lang/String;");
            jstring jstr = (jstring) env->GetObjectField(callingObj, nameFieldID);

            jfieldID typeFieldID = env->GetFieldID(clazz, "type", "I");
            jint type = env->GetIntField(callingObj, typeFieldID);

            if (jstr == NULL || env->ExceptionOccurred()){
                logger::error("fetch could not get account name in JNI");
                env->ExceptionClear();
                throwFavorException(env, "Fetch method could not get account name");
                return;
            }

            string name = env->GetStringUTFChars(jstr, NULL);
            jniExcept(
                    AccountManager* acc = findAccountManager(name, (MessageType) type);
                    if (contacts) acc->updateContacts();
                    else acc->updateMessages();
            )
        }

        JNIEXPORT jobject JNICALL create(JNIEnv* env, jobject callingObj, jstring name, jint type, jstring detailsJson) {
            const char* nameChars = env->GetStringUTFChars(name, NULL);
            const char* jsonChars = env->GetStringUTFChars(detailsJson, NULL);
            string nameString(nameChars);
            string detailsJsonString(jsonChars);
            logger::info("Create account "+nameString+" of type "+MessageTypeName[(int)type]);
            jniExceptReturnNull(
                    AccountManager::addAccount(nameString, (favor::MessageType) type, detailsJsonString);
            )

            jclass accountManager = env->FindClass("com/favor/library/AccountManager");
            jmethodID accountManagerConstructor = env->GetMethodID(accountManager, "<init>", "(Ljava/lang/String;I)V");

            jclass androidManager = env->FindClass("com/favor/library/AndroidTextManager");
            jmethodID androidManagerConstructor = env->GetMethodID(androidManager, "<init>", "(Ljava/lang/String;)V");

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
                {"create", "(Ljava/lang/String;ILjava/lang/String;)L" CLASS_PATH "AccountManager;", (void*) create},
                {"_destroy", "()V", (void*) _destroy},
                {"_update", "(Z)V", (void*) _update}
        };
}
}

#endif