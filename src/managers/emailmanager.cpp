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

using namespace std;
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
    
    
      /** Print the MIME structure of a message on the standard output.
  *
  * @param s structure object
  * @param level current depth
  */
static void printStructure(vmime::shared_ptr <const vmime::net::messageStructure> s, const int level = 0)
{
	for (int i = 0 ; i < s->getPartCount() ; ++i)
	{
		vmime::shared_ptr <const vmime::net::messagePart> part = s->getPartAt(i);

		for (int j = 0 ; j < level * 2 ; ++j)
			std::cout << " ";

		std::cout << (part->getNumber() + 1) << ". "
				<< part->getType().generate()
				<< " [" << part->getSize() << " byte(s)]"
				<< std::endl;

		printStructure(part->getStructure(), level + 1);
	}
}   


    void processPart(shared_ptr<vmime::net::messagePart> part, shared_ptr<vmime::net::message> m,  vmime::utility::outputStreamAdapter& bodystream, bool& media){
      vmime::mediaType type = part->getType();
      if (type.getType()==vmime::mediaTypes::TEXT){
	if (type.getSubType()==vmime::mediaTypes::TEXT_PLAIN){
	  //We can add to the body directly
	  //TODO: is this launching a fetch to get this info? If not, we already have it, and I'm scared that we already have images/attachments too - which we don't want to download in full
	  //I assume it is though, given the option for peeking (the final "true"). Unfortunately, it's also picking up the header right now
	  m->extractPart(part, bodystream, 0, 0, -1, true);
	}
	else if (type.getSubType()==vmime::mediaTypes::TEXT_HTML){
	  //Strip the HTMl, add it to the body
	}
	else if (type.getSubType()==vmime::mediaTypes::TEXT_ENRICHED || type.getSubType()==vmime::mediaTypes::TEXT_RICHTEXT){
	  logger::warning("Skipped parsing "+type.getType()+" portion of message with UID "+string(m->getUID()));
	}
      }
    }
    
    void EmailManager::parseMessage(bool sent, shared_ptr<vmime::net::message> m){
      shared_ptr<const vmime::header> head = m->getHeader();
      
      shared_ptr<const vmime::datetime> date = head->Date()->getValue<vmime::datetime>();
     
      long uid = stoi(m->getUID()); //stoi function may not be implemented on Android but this mail client is dekstop-only anyway
      
      string addr;
      if (sent){
	//Yes, this is a pretty wretched mess. Unfortunately, I'm somewhat at a loss for how to make this better with the 
	//current VMIME implementation. Getting an addressList back sucks.
	shared_ptr<const vmime::address> firstAddr = head->To()->getValue<vmime::addressList>()->getAddressAt(0);
	shared_ptr<const vmime::mailbox> resultAddr;
	if(firstAddr->isGroup()){
	  shared_ptr<const vmime::mailboxGroup> grp = dynamic_pointer_cast<const vmime::mailboxGroup>(firstAddr);
	  resultAddr = grp->getMailboxAt(0);
	}
	else{
	  resultAddr = dynamic_pointer_cast<const vmime::mailbox>(firstAddr);
	}
	addr = resultAddr->getEmail().toString();
      }
      else {
	addr = head->From()->getValue<vmime::mailbox>()->getEmail().toString();
      }

      //Easy header stuff is done, now we move onto parsing
      bool media = false;
      std::stringstream body;
      vmime::utility::outputStreamAdapter bodystream(cout);
      
      shared_ptr<vmime::net::messageStructure> structure = m->getStructure();
      for(int i = 0; i < structure->getPartCount(); ++i){
	processPart(structure->getPartAt(i), m, bodystream, media);
      };
      
      cout << date->getSecond() << " : " << addr << " : " << uid << endl;
      printStructure(structure);
      cout << body.str() << endl;
    }
      
    
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
      
      try {
      st->setCertificateVerifier(vmime::make_shared<TrustingCertificateVerifier>());
      st->connect();
      logger::info("Successfully connected to "+string(url)+"as ");
      } 
      //TODO: update these exceptions to contain more useful information since they can take string arguments now
      catch (vmime::exceptions::authentication_error& e){
	logger::error("Could not authenticate to "+string(url)+" as " " with the credentials provided");
	throw authenticationException();
      }
      //I've only ever seen authentication_error, but we'll catch the similarly named exception here just in case...
      catch (vmime::exceptions::authentication_exception& e){
	logger::error("Could not authenticate to "+string(url)+" as " " with the credentials provided");
	throw authenticationException();
      } 
      catch (vmime::exception& e){
 	logger::error("Error authenticating to email server: "+string(e.what()));
 	throw emailException();
      }
      
      
      
      return st;
    }
    
    void EmailManager::fetchMessages(){
      
      
      //TODO: cattch connection_error here
//             catch(vmime::exceptions::connection_error){
// 	logger::error("Error connecting to "+string(url)+"; this can mean a bad host or no internet connectivity");
// 	throw networkConnectionException();
//       }
      
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
	if (regex_search(folders[i]->getName().getBuffer(), sentRegex)){
	  if (!sent) sent = folders[i];
	  else {
	    logger::error("Competing sent folders in "+folderList(folders));
	    throw emailException();
	  }
	} else if(folders[i]->getName().getBuffer() == "INBOX"){
	  if (!inbox) inbox = folders[i];
	  else {
	    logger::error("Competing INBOX folders");
	    throw emailException();
	  }
	}
      }
      
      //TODO: these opens should be moved into their respective methods. We should definitely close one before we try and open the other.
      //The exception/error logging should stay here though; we want to know if we're missing something ASAP
//       if(sent){
// 	logger::info("Found sent folder as \"" + sent->getName().getBuffer() + "\"");
// 	if(!sent->isOpen()) sent->open(vmime::net::folder::MODE_READ_ONLY); //Not that this (or INBOX) should ever already be open...
//       } else{
// 	logger::error("Could not find sent folder in "+folderList(folders));
// 	throw emailException();
//       }
      
      if(inbox){
	logger::info("Found \"INBOX\"");
	if(!inbox->isOpen()) inbox->open(vmime::net::folder::MODE_READ_ONLY);
      } else{
	logger::error("Could not find INBOX in "+folderList(folders));
	throw emailException();
      }
     
      
      vmime::net::messageSet wantedMessages(inbox->UIDSearch(cmd));
      vector<shared_ptr<vmime::net::message>> messages = inbox->getAndFetchMessages(wantedMessages, 
	  vmime::net::fetchAttributes::STRUCTURE | vmime::net::fetchAttributes::FULL_HEADER | vmime::net::fetchAttributes::UID);
            cout << "size: " << messages.size() << endl;
      for(unsigned int i = 0; i < messages.size(); ++i){
	parseMessage(false, messages[i]);
      }    
      
//       vmime::net::message m;
//       m.getHeader()->Date()
//       
    }
    
    void EmailManager::fetchContacts(){
      
    }
}