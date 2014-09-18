#include "../accountmanager.h"
#include "managers.h"
#include "../logger.h"

#include <regex> 
/* "Why is such a useful/broad STD header only included in this specific manager?", you may ask. Because, as of
 * writing this, gcc 4.8 (which is what a lot of people are using, and more importantly is what we're compiling our
 * Android code with) doesn't implement all of the <regex> functions. See: http://stackoverflow.com/a/11635087
 * In short, this should work fine if compiled with gcc 4.9, but we're keeping <regex> out of the larger project until it is
 * at least functional for our Android compilations
 */

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
    //TODO: We might want to actually verify something here. See the VMIME documentation
    class TrustingCertificateVerifier : public vmime::security::cert::certificateVerifier
    {
    public: 
	    void verify(vmime::shared_ptr<vmime::security::cert::certificateChain> certs, const string& hostname) override {return;}
    };

    EmailManager::EmailManager(string accNm, string detailsJson) : AccountManager(accNm, TYPE_EMAIL, detailsJson){}
    
    string EmailManager::folderList(vector<shared_ptr<vmime::net::folder>> folders){
      //TODO: We're basically just JSONifying the array with folders as their getName().getBuffer() members. There might be a smarter way to do this with our JSON library...
      string out("[");
      for(int i = 0; i < folders.size(); ++i){
	out+="\""+folders[i]->getName().getBuffer()+"\"";
	if(i!=(folders.size()-1)) out+=", ";
      }
      out+="]";
      return out;
    }
    
    shared_ptr<vmime::net::store> EmailManager::login(){
      //TODO: get the actual credentials from our JSON store, and use that info to figure out the mail url as well (though it should always be imaps)
      //Also when we do this, update the "Successfully connected to" logger to include the username (obviously not the password)
      
      //TODO: catch authentication errors, maybe rethrow these as something specific to credential problems
      shared_ptr<session> sess = make_shared <session>();
      
      utility::url url("imaps://imap.gmail.com:993");
      sess->getProperties()["store.imaps.auth.username"] = "nope";
      sess->getProperties()["store.imaps.auth.password"] = "nope";
      
      vmime::shared_ptr<vmime::net::store> st = sess->getStore(url);
      st->setCertificateVerifier(vmime::make_shared<TrustingCertificateVerifier>());
      st->connect();
      logger::info("Successfully connected to "+string(url));
      
      return st;
    }
    
    void EmailManager::fetchMessages(){
      
      //TODO: figure out what VMIME throws if there's no internet connectivity at all (this shouldn't be hard), and rethrow it as a favor::networkException
      
      //TODO: catch any OTHER vmime exceptions (all of them, I hope they all inheret from a vmime::exception base) and log their what(), then rethrow them as
      //favor::emailException objects
      
      vmime::shared_ptr<vmime::net::store> st = login();
      
      
      //Dummy data
      vector<std::string> addresses;
      addresses.push_back("nope");
      addresses.push_back("nope");
      
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
      
    
      std::regex sentRegex("sent", std::regex::ECMAScript | std::regex::icase);
      
      for(int i = 0; i < folders.size(); ++i){
	cout << folders[i]->getName().getBuffer() << endl;
	if(regex_search(folders[i]->getName().getBuffer(), sentRegex)){
	  if(!sent) sent = folders[i];
	  else {
	    logger::error("Competing sent folders in "+folderList(folders));
	    throw emailException();
	  }
	}else if(folders[i]->getName().getBuffer() == "INBOX"){
	  if(!inbox) inbox = folders[i];
	  else {
	    logger::error("Competing INBOX folders");
	    throw emailException();
	  }
	}
      }
      
      //TODO: these opens should be moved into their respective methods. We should definitely close one before we try and open the other.
      //The exception/error logging should stay here though; we want to know if we're missing something ASAP
      if(sent){
	logger::info("Found sent folder as \"" + sent->getName().getBuffer() + "\"");
	if(!sent->isOpen()) sent->open(vmime::net::folder::MODE_READ_ONLY); //Not that this (or INBOX) should ever already be open...
      } else{
	logger::error("Could not find sent folder in "+folderList(folders));
	throw emailException();
      }
      
      if(inbox){
	logger::info("Found \"INBOX\"");
	if(!inbox->isOpen()) inbox->open(vmime::net::folder::MODE_READ_ONLY);
      } else{
	logger::error("Could not find INBOX in "+folderList(folders));
	throw emailException();
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