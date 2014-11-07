#include "skypemanager.h"

namespace favor{

    SkypeManager::SkypeManager(string accNm, string detailsJson) : AccountManager(accNm, TYPE_SKYPE, detailsJson) {}


    void SkypeManager::updateFetchData(){
        //TODO:
    }


    void SkypeManager::fetchAddresses() {
        //TODO
    }

    void SkypeManager::fetchMessages() {
        //TODO
    }


}