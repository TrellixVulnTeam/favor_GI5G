#ifndef favor_jni_worker_include
#define favor_jni_worker_include
#include <jni.h>
#include "../logger.h"
#include "jni_exceptions.h"
#include "../reader.h"


namespace favor{
    namespace jni{

        //TODO: there are spots to fill in here. that said, it seems to work for the most part, but we need to test it with stuff in the database.
        JNIEXPORT jobjectArray JNICALL accountManagers(JNIEnv* env, jclass clss){
            jclass accountManager = env->FindClass("com/favor/library/AccountManager");
            jmethodID accountManagerConstructor = env->GetMethodID(accountManager, "<init>", "(Ljava/lang/String;I)V");
            if (accountManager == NULL || accountManagerConstructor == NULL){
                logger::error("Could not find Accountmanager class and/or its constructor");
                //TODO:
            }

            jclass androidManager = env->FindClass("com/favor/library/AndroidTextManager");
            jmethodID androidManagerConstructor = env->GetMethodID(androidManager, "<init>", "(Ljava/lang/String;)V");
            if (androidManager == NULL | androidManagerConstructor == NULL){
                logger::error("Could not find AndroidTextmanager class and/or its constructor");
                //TODO:
            }


            auto accounts = reader::accountList();
            jobjectArray arr = (jobjectArray) env->NewObjectArray(accounts->size(), accountManager, 0);
            if(env->ExceptionOccurred() || arr == NULL){
                //Something went wrong, we failed to make our array
                env->DeleteLocalRef(arr);
                //TODO: I'm not sure, just make sure we don't hold the accountList() lock forever
            }
            int i = 0;
            for (auto it = accounts->begin(); it != accounts->end(); ++it){
                if ((*it)->type == TYPE_ANDROIDTEXT){
                    jobject obj = env->NewObject(androidManager, androidManagerConstructor, env->NewStringUTF((*it)->accountName.c_str()));
                    env->SetObjectArrayElement(arr, i, obj);
                    if (obj == NULL || env->ExceptionOccurred()){
                        //TODO:
                    }
                }
                else {
                    jobject obj = env->NewObject(accountManager, accountManagerConstructor, env->NewStringUTF((*it)->accountName.c_str()), (*it)->type);
                    env->SetObjectArrayElement(arr, i, obj);
                    if (obj == NULL || env->ExceptionOccurred()){
                        //TODO:
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