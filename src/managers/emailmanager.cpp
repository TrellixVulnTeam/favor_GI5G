#include "../accountmanager.h"
#include "managers.h"
#include "../logger.h"

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
      
      //TODO: figure out what VMIME throws if there's no internet connectivity at all (this shouldn't be hard), and rethrow it as a favor::networkException
      
      //TODO: catch any OTHER vmime exceptions (all of them, I hope they all inheret from a vmime::exception base) and log their what(), then rethrow them as
      //favor::emailException objects
      
      //Login
      shared_ptr<session> sess = make_shared <session>();
      utility::url url("imaps://imap.gmail.com:993");
      //imaps is distinct from say, imap, so when we set store properties we need to be very careful to match the exact protocol we're using.
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
      vector<shared_ptr<vmime::net::folder>> folders = st->getRootFolder()->getFolders(true);
      
      shared_ptr<vmime::net::folder> sent = null;
      shared_ptr<vmime::net::folder> inbox = null;
      
      
      //TODO: making very dumb regex error here and I don't know what it is, apparently
      std::regex sentRegex("(sent)", std::regex::ECMAScript | std::regex::icase);
      
      if(regex_search("sent", sentRegex, std::regex::ECMAScript | std::regex::icase)) cout << "match" << endl;
      else cout << "nomatch " << endl;
      
      //TODO: except is sent/inbox not found, or if duplicates of either are found
      
      for(int i = 0; i < folders.size(); ++i){
	cout << folders[i]->getName().getBuffer() << endl;
	if(regex_match(folders[i]->getName().getBuffer(), sentRegex)){
	  if(!sent) sent = folders[i];
	  else {
	    logger::error("Competing sent folders in:" /*TODO: folder list printing method*/);
	  }
	}else if(folders[i]->getName().getBuffer() == "INBOX"){
	  if(!inbox) inbox = folders[i];
	  else {
	    logger::error("Competing INBOX folders");
	  }
	}
      }
      
      if(sent){
	logger::info("Found sent folder as \"" + sent->getName().getBuffer() + "\"");
	if(!sent->isOpen()) sent->open(vmime::net::folder::MODE_READ_ONLY); //Not that this should ever already be open...
      } else{
	logger::error("Could not find sent folder");
      }
      
      if(inbox){
	logger::info("Found \"INBOX\"");
	if(!inbox->isOpen()) inbox->open(vmime::net::folder::MODE_READ_ONLY); //Not that this should ever already be open...
      } else{
	logger::error("Could not find INBOX");
      }
     
      //TODO: isOpen() seems to be lying to us, so we're getting an exception here.
      if(!inbox->isOpen()) {
	inbox->open(vmime::net::folder::MODE_READ_ONLY); //Not checking if the folder is already open was causing problems, I think it gets opened when we look for the sent folder
      }
      
      return; //after this is slow, testing stuff before it.
      
      //TODO: bad response  
      vmime::net::messageSet wantedMessages(inbox->UIDSearch(cmd));
      vector<shared_ptr<vmime::net::message>> messages = inbox->getAndFetchMessages(wantedMessages, 
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