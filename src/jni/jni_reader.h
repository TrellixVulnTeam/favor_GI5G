#ifndef favor_jni_worker_include
#define favor_jni_worker_include
#include <jni.h>
#include "jni_globals.h"
#include "jni_exceptions.h"
#include "../reader.h"
#include "../logger.h"


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

            //This could conceivably produce more than 16 refs, but it seems highly unlikely. Also I don't know if the possible slowdown is even enough
            //to warrant deleting refs for objects we've created...

            auto accounts = reader::accountList();
            jobjectArray arr = (jobjectArray) env->NewObjectArray(accounts->size(), account_manager, 0);
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
                    logger::info((*it)->accountName);
                    jobject obj = env->NewObject(android_text_manager, android_text_manager_constructor, env->NewStringUTF((*it)->accountName.c_str()));
                    env->SetObjectArrayElement(arr, i, obj);
                    if (obj == NULL || env->ExceptionOccurred()){
                        logger::error("Error inserting account "+(*it)->accountName+" into array");
                        env->ExceptionClear();
                    }
                }
                else {
                    logger::info((*it)->accountName);
                    jobject obj = env->NewObject(account_manager, account_manager_constructor, env->NewStringUTF((*it)->accountName.c_str()), (*it)->type);
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