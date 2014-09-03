#include "../accountmanager.h"
#include "managers.h"


namespace favor{
    EmailManager::EmailManager(string accNm, string detailsJson) : AccountManager(accNm, TYPE_EMAIL, detailsJson){}
    void EmailManager::fetchMessages(){
      
    }
    void EmailManager::fetchContacts(){
      
    }
}