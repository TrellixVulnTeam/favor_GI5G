#include "../accountmanager.h"
#include "managers.h"

#include <vmime/vmime.hpp>

using namespace vmime;
using namespace vmime::net;

namespace favor{

// Certificate verifier (TLS/SSL)
class TrustingCertificateVerifier : public vmime::security::cert::defaultCertificateVerifier
{
public:

	void verify(vmime::shared_ptr<vmime::security::cert::certificateChain> chain)
	{
	}
};

    EmailManager::EmailManager(string accNm, string detailsJson) : AccountManager(accNm, TYPE_EMAIL, detailsJson){}
    void EmailManager::fetchMessages(){
      utility::url url("imaps://imap.gmail.com:993");
      shared_ptr<session> sess = make_shared <session>();
      sess->getProperties()["store.imap.auth.username"] = "nope";
      sess->getProperties()["store.imap.auth.password"] = "nope";
      vmime::shared_ptr<vmime::net::store> st = sess->getStore(url);
      st->connect();
      //store->setCertificateVerifier(yourCertificateVerifier);
      vmime::net::folder::path path = st->getDefaultFolder()->getFullPath();
      vmime::net::folder::path path2;
      path2 /= vmime::net::folder::path::component("INBOX");
      vmime::shared_ptr<vmime::net::folder> fld = st->getFolder(path2);
      fld->open(vmime::net::folder::MODE_READ_ONLY);
    }
    
    void EmailManager::fetchContacts(){
      
    }
}