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
      
      //Login
      shared_ptr<session> sess = make_shared <session>();
      utility::url url("imaps://imap.gmail.com:993");
      //imaps is distinct from say, imap, so when we set store properties we need to be very careful to match the exact protocol we're using.
      sess->getProperties()["store.imaps.auth.username"] = "nope";
      sess->getProperties()["store.imaps.auth.password"] = "nope";
      vmime::shared_ptr<vmime::net::store> st = sess->getStore(url);
      st->setCertificateVerifier(vmime::make_shared<TrustingCertificateVerifier>());
      st->connect();
      
      //Dummy data
      vector<std::string> addresses;
      
      //Assemble cmd
      std::string cmd("");
      std::string addressField("FROM");
      long lastUID = 1; //UIDs are from 1, always
      for (int i = 1; i < addresses.size(); ++i){cmd+="OR ";} //One less "OR " than the number of addresses is important, so we start from 1 here
      for (int i = 0; i < addresses.size(); ++i){cmd+=(addressField+" \""+addresses[i]+"\" ");}
      cmd+=("UID "+to_string(lastUID)+":*");
      
      std::cout << cmd << endl;
      
      //Find sent folder
      vector<shared_ptr<vmime::net::folder>> folders = st->getDefaultFolder()->getFolders(true);
      shared_ptr<vmime::net::folder> sent = null;
      std::regex sentRegex("sent", std::regex_constants::icase);
      for(int i = 0; i < folders.size(); ++i){
	if(regex_match(folders[i]->getName().getBuffer(), sentRegex)){
	  sent = folders[i];
	  //TODO: test this, and also handle competing or non-existant folder names. We might want to think about applying a similar concept to throwing exceptions
	  //as whether or not to log things. I say this because it's somewhat ambiguous whether to throw an exception for problems identifying the sent folder, but
	  //the top level definitely needs to know about it somehow... in any case, should be logged
	}
      }
      
      //TODO: look for the SENT folder with some simple case insensitive string matching
     
      
      vmime::net::folder::path path = st->getDefaultFolder()->getFullPath();
      vmime::net::folder::path path2;
      path2 /= vmime::net::folder::path::component("INBOX");
      vmime::shared_ptr<vmime::net::folder> fld = st->getFolder(path2);
      fld->open(vmime::net::folder::MODE_READ_ONLY);
      //vmime::net::messageSet wantedMessages(fld->UIDSearch("UID 5:5"));
      vmime::net::messageSet wantedMessages(fld->UIDSearch(cmd));
      vector<shared_ptr<vmime::net::message>> messages = fld->getAndFetchMessages(wantedMessages, 
	  vmime::net::fetchAttributes::FLAGS | vmime::net::fetchAttributes::ENVELOPE);
      for(unsigned int i = 0; i < messages.size(); ++i){
	vmime::utility::outputStreamAdapter out(cout);
	//messages[i]->extract(out);
	//cout << "--------------------------------" << endl;
      }
      cout << "size: " << messages.size() << endl;
      
      
      
    }
    
    void EmailManager::fetchContacts(){
      
    }
}