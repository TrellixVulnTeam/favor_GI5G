#include "linemanager.h"

namespace favor{

    LineManager::LineManager(string accNm, string detailsJson) : AccountManager(accNm, TYPE_LINE, detailsJson) {}


    void LineManager::updateFetchData(){
        //TODO:
    }


    void LineManager::fetchAddresses() {
        //TODO
    }

    void LineManager::fetchMessages() {
        //TODO
    }


}