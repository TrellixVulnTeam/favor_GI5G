#include "androidtextmanager.h"

namespace favor{
    JavaVM* AndroidTextManager::vm = NULL;

    AndroidTextManager::AndroidTextManager(string accNm, string detailsJson) : AccountManager(accNm, TYPE_ANDROIDTEXT, detailsJson) {}

    void AndroidTextManager::setVM(JavaVM* inputVm){
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