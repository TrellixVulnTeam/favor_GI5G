#ifndef favor_jni_accountmanager_include
#define favor_jni_accountmanager_include
#include <jni.h>
#include "jni_exceptions.h"
#include "jni_reader.h"
#include "jni_string.h"
#include "../accountmanager.h"
#include "../reader.h"
#include "../worker.h"

/*
private native void _saveMessages(int type, boolean[] sent, long[] id, long[] date, String[] address, boolean[] media, String[] msg) throws FavorException; //TODO:
 */


namespace favor{
    namespace jni{



        JNIEXPORT void JNICALL _saveMessages(JNIEnv* env, jobject callingObj, jint type, jstring name, jbooleanArray sent, jlongArray ids, jlongArray dates, jobjectArray addresses, jbooleanArray media, jobjectArray bodies){
            jboolean* sentArray = env->GetBooleanArrayElements(sent, NULL);
            jlong* idArray = env->GetLongArrayElements(ids, NULL);
            jlong* dateArray = env->GetLongArrayElements(dates, NULL);
            jboolean* mediaArray = env->GetBooleanArrayElements(media, NULL);

            jsize msgCount = env->GetArrayLength(sent);

            JNIString accountName(env, name);
            AccountManager* accManager = findAccountManager(accountName, (MessageType) type);

            for (int i = 0; i < msgCount; ++i){
                JNIString address(env, (jstring) env->GetObjectArrayElement(addresses, i));
                JNIString body(env, (jstring) env->GetObjectArrayElement(bodies, i));
                //TODO: date may need to become a time_t
                accManager->holdMessage(sentArray[i], idArray[i], dateArray[i], address, mediaArray[i], body);
                address.deleteRefAndInvalidate();
                body.deleteRefAndInvalidate();
            }

            env->ReleaseBooleanArrayElements(sent, sentArray, JNI_ABORT);
            env->ReleaseLongArrayElements(ids, idArray, JNI_ABORT);
            env->ReleaseLongArrayElements(dates, dateArray, JNI_ABORT);
            env->ReleaseBooleanArrayElements(media, mediaArray, JNI_ABORT);

            logger::info("Saving "+as_string((int)msgCount)+" messages");
            jniExcept(
                accManager->saveHeldMessages();
            )
        }

        //This is a heavy method. We're doing multiple JNI calls for every contact; we could potentially get around this with some really obnoxious
        //process like jamming all of the relevant characters into a bye array ourselves, deliminating them, and then separating them into usable data
        //again at the C++ layer, but right now something like that is premature. This method doesn't need to be super performant anyway because it's a
        //writer.
        JNIEXPORT void JNICALL _saveAddresses(JNIEnv* env, jobject callingObj, jint type, jobjectArray addressStrings, jintArray countArray,jobjectArray nameStrings){

            jsize addrCount = env->GetArrayLength(addressStrings);
            jint* counts = env->GetIntArrayElements(countArray, NULL);
            std::unordered_map<std::string, int> countedAddresses;
            std::unordered_map<std::string, std::string> addressNames;

            logger::info("Save "+as_string((int)addrCount)+" addresses of type "+MessageTypeName[type]);
            for (int i = 0; i < addrCount; ++i){
                JNIString addr(env, (jstring) env->GetObjectArrayElement(addressStrings, i));
                JNIString name(env, (jstring) env->GetObjectArrayElement(nameStrings, i));
                countedAddresses[addr] = counts[i];
                if (!name.isNull()) addressNames[addr] = name;

                addr.deleteRefAndInvalidate();
                name.deleteRefAndInvalidate();
            }
            env->ReleaseIntArrayElements(countArray, counts, JNI_ABORT);

            jniExcept(
                    worker::recomputeAddressTable(countedAddresses, addressNames, (MessageType)type);
            )

        }

        //TODO: method untested
        JNIEXPORT jobjectArray JNICALL contactAddresses(JNIEnv* env, jobject callingObj, jint type){

            list<Address> ret;
            auto contacts = reader::contactList();
            logger::info("ContactsList length: "+as_string((int)contacts->size()));
            for (auto it = contacts->begin(); it != contacts->end(); ++it){
                if (it->hasType((MessageType) type)){
                    for (int i = 0; i < it->getAddresses().size(); ++i){
                        if (it->getAddresses()[i].type == type) ret.push_back(it->getAddresses()[i]);
                    }
                }
            }

            //TODO: ret may very well be in excess of 16, in which case we should inform the JVM we'll be using X extra local references
            //or can we save the references returned by NewStringUTF and release them instead?
            logger::info("Get "+as_string((int)ret.size())+" contacts of type "+MessageTypeName[type]); //TODO: TESTCODE
            jobjectArray arr = (jobjectArray) env->NewObjectArray(ret.size(), java_string, 0);
            if (env->ExceptionOccurred() || arr == NULL){
                //Something went wrong, we failed to make our array
                env->DeleteLocalRef(arr);
                logger::error("Unable to requisition array for contactAddresses");
                env->ExceptionClear();
                return NULL;
            }

            int i = 0;
            for (auto it = ret.begin(); it != ret.end(); ++it){
                logger::info("Get "+it->addr); //TODO: TESTCODE
                env->SetObjectArrayElement(arr, i, env->NewStringUTF(it->addr.c_str()));
                ++i;
            }

            return arr;
        }

        JNIEXPORT void JNICALL _destroy(JNIEnv* env, jobject callingObj, jstring accName, jint type){

            JNIString name(env, accName);
            logger::info("Destroy account "+name.getString()+" of type "+MessageTypeName[(int)type]);
            jniExcept(
                    AccountManager* acc = findAccountManager(name, static_cast<MessageType>(type));
                    acc->destroy();
            )

        }

        JNIEXPORT void JNICALL _update(JNIEnv* env, jobject callingObj, jstring accName, jint type, jboolean addresses){

            JNIString name(env, accName);
            jniExcept(
                    AccountManager* acc = findAccountManager(name, static_cast<MessageType>(type));
                    if (addresses) acc->updateAddresses();
                    else acc->updateMessages();
            )
        }

        JNIEXPORT void JNICALL _create(JNIEnv* env, jobject callingObj, jstring name, jint type, jstring detailsJson) {
            JNIString nameString(env, name);
            JNIString detailsJsonString(env, detailsJson);
            logger::info("Create new account "+nameString.getString()+" of type "+MessageTypeName[(int)type]);
            jniExcept(
                    AccountManager::addAccount(nameString, (favor::MessageType)((int)type), detailsJsonString);
            )
        }

        static JNINativeMethod accountManagerMethodTable[] = {
                {"_create", "(Ljava/lang/String;ILjava/lang/String;)V", (void*) _create},
                {"_destroy", "(Ljava/lang/String;I)V", (void*) _destroy},
                {"_update", "(Ljava/lang/String;IZ)V", (void*) _update},
                {"contactAddresses", "(I)[Ljava/lang/String;", (void*) contactAddresses},
                {"_saveAddresses", "(I[Ljava/lang/String;[I[Ljava/lang/String;)V", (void*) _saveAddresses},
                {"_saveMessages", "(ILjava/lang/String;[Z[J[J[Ljava/lang/String;[Z[Ljava/lang/String;)V", (void*) _saveMessages}
        };
}
}

#endif