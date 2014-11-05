#include "androidtextmanager.h"

namespace favor{
    const char* AndroidTextManager::addrListName  = "managedAddresses";
    JavaVM* AndroidTextManager::vm = NULL;

    AndroidTextManager::AndroidTextManager(string accNm, string detailsJson) : AccountManager(accNm, TYPE_EMAIL, detailsJson) {
        if (json.HasMember(addrListName)){
            rapidjson::Value& addrsVal = json[addrListName];
            if (!addrsVal.IsArray()) throw badUserDataException("Managed addresses list improperly formatted in "+accountName +" json");
            else {
                for (auto it = addrsVal.Begin(); it!= addrsVal.End(); ++it){
                    //TODO: check to make sure all of the addresses are properly formatted phone numbers
                    managedAddresses.insert(it->GetString());
                }
            }
        }
        else {
            rapidjson::Value addrsVal;
            addrsVal.SetArray();
            json.AddMember(rapidjson::Value(addrListName, json.GetAllocator()).Move(), addrsVal, json.GetAllocator());
        }
    }

    void AndroidTextManager::setVM(JavaVM* inputVm){
        //TODO: vm is undefined referencel? why?
        vm = inputVm;
    }

    void AndroidTextManager::updateFetchData(){
        //TODO:
    }



    //TODO: next two methods should hit static methods for crawling text data. Remember there should only ever be one android text manager
    //http://stackoverflow.com/questions/12420463/jni-keeping-global-reference-to-the-environment

    void AndroidTextManager::fetchAddresses() {
        //TODO: this will be a (static) java method
    }

    void AndroidTextManager::fetchMessages() {
        //TODO: this will be a (static) java method
    }


}