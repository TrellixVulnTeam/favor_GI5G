#ifndef favor_jni_accountmanager_include
#define favor_jni_accountmanager_include
#include <jni.h>
#include "jni_exceptions.h"
#include "jni_reader.h"
#include "../accountmanager.h"
#include "../reader.h"
#include "../worker.h"


//TODO:
//private native String[] contactAddresses() throws FavorException;
//This should be almost exactly like the c++ accountmanager implementation, just get a type from our java class and do the same thing


namespace favor{
    namespace jni{

        JNIEXPORT jobjectArray JNICALL contactAddresses(JNIEnv* env, jobject callingObj, jint type){

            list<Address> ret;
            auto contacts = reader::contactList((MessageType)type);
            for (auto it = contacts->begin(); it != contacts->end(); ++it){
                for (int i = 0; i < it->getAddresses().size(); ++i){
                    ret.push_back(it->getAddresses()[i]);
                }
            }

            jobjectArray arr = (jobjectArray) env->NewObjectArray(ret.size(), java_string, 0);
            if(env->ExceptionOccurred() || arr == NULL){
                //Something went wrong, we failed to make our array
                env->DeleteLocalRef(arr);
                logger::error("Unable to requisition array for contactAddresses");
                env->ExceptionClear();
                return NULL;
            }

            //TODO: jam these into a java array like the reader does and then return them
        }

        JNIEXPORT void JNICALL _destroy(JNIEnv* env, jobject callingObj, jstring accName, jint type){

            string name = env->GetStringUTFChars(accName, NULL);
            logger::info("Destroy account "+name+" of type "+MessageTypeName[(int)type]);
            jniExcept(
                    AccountManager* acc = findAccountManager(name, (MessageType)type);
                    acc->destroy();
            )

        }

        JNIEXPORT void JNICALL _update(JNIEnv* env, jobject callingObj, jstring accName, jint type, jboolean contacts){

            string name = env->GetStringUTFChars(accName, NULL);
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
                    AccountManager::addAccount(nameString, (favor::MessageType)((int)type), detailsJsonString);
            )


            jobject obj;
            if (type == TYPE_ANDROIDTEXT){
                obj = env->NewObject(android_text_manager, android_text_manager_constructor, name);
            } else {
                obj = env->NewObject(account_manager, account_manager_constructor, name, type);
            }

            if (obj == NULL || env->ExceptionOccurred()){
                logger::error("Error instantiating account "+nameString);
                env->ExceptionClear();
            }

            return obj;
        }

        static JNINativeMethod accountManagerMethodTable[] = {
                {"create", "(Ljava/lang/String;ILjava/lang/String;)L" CLASS_PATH "AccountManager;", (void*) create},
                {"_destroy", "(Ljava/lang/String;I)V", (void*) _destroy},
                {"_update", "(Ljava/lang/String;IZ)V", (void*) _update}
        };
}
}

#endif