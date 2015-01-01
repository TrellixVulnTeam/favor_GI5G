/*
Copyright (C) 2015  Joshua Tanner (mindful.jt@gmail.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



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

        Contact findContact(long id){
            auto contacts = reader::contactList();
            for (auto it = contacts->begin(); it != contacts->end(); ++it){
                if (it->id == id){
                    return *it;
                }
            }
            logger::error("Requested contact with nonexistant id "+as_string(id));
            throw badUserDataException("Requested contact with nonexistant id");
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
                        logger::error("Error inserting account "+as_string(*(*it))+" into array");
                        env->ExceptionClear();
                    }
                }
                else {
                    logger::info((*it)->accountName);
                    jobject obj = env->NewObject(account_manager, account_manager_constructor, env->NewStringUTF((*it)->accountName.c_str()), (*it)->type);
                    env->SetObjectArrayElement(arr, i, obj);
                    if (obj == NULL || env->ExceptionOccurred()){
                        logger::error("Error inserting account "+as_string(*(*it))+" into array");
                        env->ExceptionClear();
                    }
                }
                ++i;
            }
            return arr;
        }

        //TODO:  contacts and allAddresses entirely unrelated
        JNIEXPORT jobjectArray JNICALL contacts(JNIEnv* env, jclass clss){
            auto contactList = reader::contactList();
            jobjectArray arr = (jobjectArray) env->NewObjectArray(contactList->size(), contact, 0);

            if(env->ExceptionOccurred() || arr == NULL){
                //Something went wrong, we failed to make our array
                env->DeleteLocalRef(arr);
                logger::error("Unable to requisition array for contacts");
                env->ExceptionClear();
                return NULL;
            }

            int i = 0;
            for (auto it = contactList->begin(); it != contactList->end(); ++it){
                jobject obj = env->NewObject(contact, contact_constructor, (jlong)it->id, env->NewStringUTF(it->displayName.c_str()));
                env->SetObjectArrayElement(arr, i, obj);
                if (obj == NULL || env->ExceptionOccurred()){
                    logger::error("Error inserting contact "+as_string(*it)+" into array");
                    env->ExceptionClear();
                }
                ++i;
            }
            return arr;
        }

        JNIEXPORT jobjectArray JNICALL allAddresses(JNIEnv* env, jclass clss, jboolean contactRelevantOnly){
            auto addresses = reader::addresses(FLAG_ALL, (bool) contactRelevantOnly);
            jobjectArray arr = (jobjectArray) env->NewObjectArray(addresses->size(), address, 0);

            if(env->ExceptionOccurred() || arr == NULL){
                //Something went wrong, we failed to make our array
                env->DeleteLocalRef(arr);
                logger::error("Unable to requisition array for allAddresses");
                env->ExceptionClear();
                return NULL;
            }

            int i = 0;
            for (auto it = addresses->begin(); it != addresses->end(); ++it){
                jobject obj = env->NewObject(address, address_constructor, env->NewStringUTF(it->addr.c_str()), (jlong) it->count, (jlong)it->contactId, (jint)it->type);
                env->SetObjectArrayElement(arr, i, obj);
                if (obj == NULL || env->ExceptionOccurred()){
                    logger::error("Error inserting address "+as_string(*it)+" into array");
                    env->ExceptionClear();
                }
                ++i;
            }
            return arr;
        }

        static JNINativeMethod readerMethodTable[] = {
                {"accountManagers", "()[L" CLASS_PATH "AccountManager;", (void*) accountManagers},
                {"_contacts", "()[L" CLASS_PATH "Contact;", (void*) contacts},
                {"allAddresses", "(Z)[L" CLASS_PATH "Address;", (void*) allAddresses}
        };

    }
}



#endif