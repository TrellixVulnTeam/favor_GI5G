#ifndef favor_jni_worker_include
#define favor_jni_worker_include
#include <jni.h>
#include "../logger.h"
#include "jni_exceptions.h"
#include "../reader.h"


namespace favor{
    namespace jni{

        //TODO: it seems to work for the most part, but we need to test it with stuff in the database.
        JNIEXPORT jobjectArray JNICALL accountManagers(JNIEnv* env, jclass clss){
            jclass accountManager = env->FindClass("com/favor/library/AccountManager");
            jmethodID accountManagerConstructor = env->GetMethodID(accountManager, "<init>", "(Ljava/lang/String;I)V");
            if (accountManager == NULL || accountManagerConstructor == NULL){
                logger::error("Could not find Accountmanager class and/or its constructor");
                env->ExceptionClear();
                throwFavorException(env, "Could not find Accountmanager class and/or its constructor");
                return NULL;
            }

            jclass androidManager = env->FindClass("com/favor/library/AndroidTextManager");
            jmethodID androidManagerConstructor = env->GetMethodID(androidManager, "<init>", "(Ljava/lang/String;)V");
            if (androidManager == NULL | androidManagerConstructor == NULL){
                logger::error("Could not find AndroidTextManager class and/or its constructor");
                env->ExceptionClear();
                throwFavorException(env, "Could not find AndroidTextManager class and/or its constructor");
                return NULL;
            }


            auto accounts = reader::accountList();
            jobjectArray arr = (jobjectArray) env->NewObjectArray(accounts->size(), accountManager, 0);
            if(env->ExceptionOccurred() || arr == NULL){
                //Something went wrong, we failed to make our array
                env->DeleteLocalRef(arr);
                logger::error("Unable to requisition array for accountManagers");
                env->ExceptionClear();
                return NULL;
            }
            int i = 0;
            for (auto it = accounts->begin(); it != accounts->end(); ++it){
                if ((*it)->type == TYPE_ANDROIDTEXT){
                    jobject obj = env->NewObject(androidManager, androidManagerConstructor, env->NewStringUTF((*it)->accountName.c_str()));
                    env->SetObjectArrayElement(arr, i, obj);
                    if (obj == NULL || env->ExceptionOccurred()){
                        logger::error("Error inserting account "+(*it)->accountName+" into array");
                        env->ExceptionClear();
                    }
                }
                else {
                    jobject obj = env->NewObject(accountManager, accountManagerConstructor, env->NewStringUTF((*it)->accountName.c_str()), (*it)->type);
                    env->SetObjectArrayElement(arr, i, obj);
                    if (obj == NULL || env->ExceptionOccurred()){
                        logger::error("Error inserting account "+(*it)->accountName+" into array");
                        env->ExceptionClear();
                    }
                }
                ++i;
            }
            return arr;
        }

        static JNINativeMethod readerMethodTable[] = {
                {"accountManagers", "()[L" CLASS_PATH "AccountManager;", (void*) accountManagers}
        };

    }
}



#endif