#include "../accountmanager.h"
#include "managers.h"

#include <vmime/vmime.hpp>

using namespace vmime;

namespace favor{
    EmailManager::EmailManager(string accNm, string detailsJson) : AccountManager(accNm, TYPE_EMAIL, detailsJson){}
    void EmailManager::fetchMessages(){
      shared_ptr<net::session> theSession = make_shared <net::session>();
    }
    void EmailManager::fetchContacts(){
      
    }
}