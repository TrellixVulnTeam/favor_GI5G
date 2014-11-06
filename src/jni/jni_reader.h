#ifndef favor_jni_worker_include
#define favor_jni_worker_include
#include <jni.h>
#include "../logger.h"
#include "jni_exceptions.h"
#include "../reader.h"


namespace favor{
    namespace jni{

        //JNI helper method, though not actually exposed to the java layer
        AccountManager* findAccountManager(const string& name, const MessageType type){
            //This runs in N, which is objectively slow, but this doens't need to be performant
            //and there should only ever be a tiny number of accounts anyway
            auto accounts = reader::accountList();
            for (auto it = accounts->begin(); it != accounts->end(); ++it){
                if ((*it)->accountName == name && (*it)->type == type){
                    return *it;
                }
            }
            return NULL;
        }

        JNIEXPORT jobjectArray JNICALL accountManagers(JNIEnv* env, jclass clss){
            jclass accountManager = env->FindClass("com/favor/library/AccountManager");
            jmethodID accountManagerConstructor = env->GetMethodID(accountManager, "<init>", "(Ljava/lang/String;I)V");

            jclass androidManager = env->FindClass("com/favor/library/AndroidTextManager");
            jmethodID androidManagerConstructor = env->GetMethodID(androidManager, "<init>", "(Ljava/lang/String;)V");


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