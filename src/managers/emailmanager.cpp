#include "../accountmanager.h"
#include "managers.h"

#include <vmime/vmime.hpp>

using namespace vmime;
using namespace vmime::net;

namespace favor{
  
    /*
    * If you need to do more complex verifications on certificates, you will
    * have to write your own verifier. Your verifier should inherit from the
    * vmime::security::cert::certificateVerifier class and implement the method
    * verify(). Then, if the specified certificate chain is trusted, simply return from the function,
    * or else throw a certificate verification exception.
    */
    // Certificate verifier (TLS/SSL)
    class TrustingCertificateVerifier : public vmime::security::cert::certificateVerifier
    {
    public: 
	    void verify(vmime::shared_ptr<vmime::security::cert::certificateChain> certs, const string& hostname) override {return;}
    };

    EmailManager::EmailManager(string accNm, string detailsJson) : AccountManager(accNm, TYPE_EMAIL, detailsJson){}
    void EmailManager::fetchMessages(){
      shared_ptr<session> sess = make_shared <session>();
      utility::url url("imaps://imap.gmail.com:993");
      //imaps is distinct from say, imap, so when we set store properties we need to be very careful to match the exact protocol we're using.
      sess->getProperties()["store.imaps.auth.username"] = "nope";
      sess->getProperties()["store.imaps.auth.password"] = "nope";
      vmime::shared_ptr<vmime::net::store> st = sess->getStore(url);
      st->setCertificateVerifier(vmime::make_shared<TrustingCertificateVerifier>());
      st->connect();
      vmime::net::folder::path path = st->getDefaultFolder()->getFullPath();
      vmime::net::folder::path path2;
      path2 /= vmime::net::folder::path::component("INBOX");
      vmime::shared_ptr<vmime::net::folder> fld = st->getFolder(path2);
      fld->open(vmime::net::folder::MODE_READ_ONLY);
    }
    
    void EmailManager::fetchContacts(){
      
    }
}