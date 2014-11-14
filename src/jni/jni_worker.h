#ifndef favor_jni_reader_include
#define favor_jni_reader_include
#include <jni.h>
#include <reader.h>
#include "jni_exceptions.h"
#include "jni_string.h"
#include "../worker.h"

namespace favor{
    namespace jni{

        JNIEXPORT void JNICALL _createContact(JNIEnv* env, jobject callingObj, jstring address, jint type, jstring displayName, jboolean addressExistsHint){
            //TODO: this isn't finding/associating with the address properly when we pass it one that exists
            JNIString addressString(env, address);
            JNIString displaynameString(env, displayName);
            logger::info("Create contact with address: "+addressString.getString()+" and name "+displaynameString.getString());
            //Short circuiting saves us the method call if we know the address exists
            if (addressExistsHint || reader::addressExists(addressString, static_cast<MessageType>(type))){
                //We believe our reader or hint, make an address object and pass it in like it was from the database
                jniExcept(
                    Address addrObj(addressString, -1, -1, static_cast<MessageType>(type));
                        logger::info("Address exists, creating from "+as_string(addrObj));
                    worker::createContactFromAddress(addrObj, displaynameString);
                )
            } else {
                //There's no address already, so we make one
                logger::info("No address found, creating with address");
                jniExcept(
                    worker::createContactWithAddress(addressString, static_cast<MessageType>(type), displaynameString);
                )
            }

        }

        static JNINativeMethod workerMethodTable[] = {
                {"_createContact", "(Ljava/lang/String;ILjava/lang/String;Z)V", (void*) _createContact}
        };

    }
}



#endif