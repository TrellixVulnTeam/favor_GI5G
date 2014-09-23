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
    
    std::time_t EmailManager::toTime(const vmime::datetime input){
      struct tm timeinfo;
      const vmime::datetime time = vmime::utility::datetimeUtils::toUniversalTime(input);
      timeinfo.tm_sec = time.getSecond();
      timeinfo.tm_min = time.getMinute();
      timeinfo.tm_hour = time.getHour();
      timeinfo.tm_mday = time.getDay();
      timeinfo.tm_mon = time.getMonth() - 1; //tm specification is "Months since January"
      timeinfo.tm_year = time.getYear() - 1900; //tm specification is "Years since 1900"
      timeinfo.tm_isdst = 0; //No daylight savings in universal time, I believe
      time_t ret = mktime(&timeinfo);
      return ret;
    }
      

    bool EmailManager::hasMedia(shared_ptr<vmime::net::messageStructure> structure){
      for(int i = 0; i < structure->getPartCount(); ++i){
	shared_ptr<vmime::net::messagePart> part = structure->getPartAt(i);
	if (part->getPartCount()){
	  if (hasMedia(part->getStructure())) return true;
	}
	else {
	  string partType = part->getType().getType();
	  if (partType!=vmime::mediaTypes::TEXT && partType!=vmime::mediaTypes::MULTIPART) return true;
	}
      }
      return false;
    }

    void EmailManager::parseMessage(bool sent, shared_ptr<vmime::net::message> m){
      
      shared_ptr<vmime::message> parsedMessage = m->getParsedMessage();
      vmime::messageParser mp(parsedMessage);
      shared_ptr<vmime::net::messageStructure> structure = m->getStructure();
      std::stringstream body;
      vmime::utility::outputStreamAdapter os(body);
      
      /*Strangely, if we get the date like this:
      * shared_ptr<const vmime::datetime> rawdate = m->getHeader()->Date()->getValue<vmime::datetime>();
      * it reports a one second difference (earlier) than if we get it the way we're doing below.
      * Probably just a rounding quirk, but something word recording */
      
      
      const time_t date = toTime(mp.getDate());
 
      const long uid = stoi(m->getUID()); //stoi function may not be implemented on Android but this mail client is dekstop-only anyway
      
      const bool media = hasMedia(structure);
      
      for (int i = 0; i < mp.getTextPartCount(); ++i){
	vmime::shared_ptr<const vmime::textPart> tp = mp.getTextPartAt(i);
	if (tp->getType().getSubType() == vmime::mediaTypes::TEXT_HTML){
	  vmime::shared_ptr<const vmime::htmlTextPart> htp = dynamic_pointer_cast<const vmime::htmlTextPart>(tp);
	  
	  //If this HTML part has a plain text equivalent, use that
	  if (!htp->getPlainText()->isEmpty()){
	    htp->getPlainText()->extract(os);
	  }
	  else {
	    htp->getText()->extract(os);
	    //TODO: strip the HTML, and more importantly, convert HTML-encoded characters into appropriate unicode
	  }
	  //TODO: handle encoding, at least insofar as figuring out which of our constants we pass down
	  cout << "encoding: " << htp->getText()->getEncoding().getName() << endl;
	  //vmime::encodingTypes::
	}
	else if (tp->getType().getSubType() == vmime::mediaTypes::TEXT_PLAIN){
	  cout << "encoding: " << tp->getText()->getEncoding().getName() << endl;
	  tp->getText()->extract(os);
	}
      }    
      
      
      //TODO: test exporting, especially for sent messages with multiple recipients
      if (sent){
	shared_ptr<const vmime::address> firstAddr = mp.getRecipients().getAddressAt(0);
	shared_ptr<const vmime::mailbox> resultAddr;
	if(firstAddr->isGroup()){
	  shared_ptr<const vmime::mailboxGroup> grp = dynamic_pointer_cast<const vmime::mailboxGroup>(firstAddr);
	  for (int i = 0; i < grp->getMailboxCount(); ++i){
	    holdMessage(sent, uid, date, grp->getMailboxAt(i)->getEmail().toString(), media, body.str());
	  }
	}
	else{
	  resultAddr = dynamic_pointer_cast<const vmime::mailbox>(firstAddr);
	  holdMessage(sent, uid, date, resultAddr->getEmail().toString(), media, body.str());
	}
      }
      else {
	holdMessage(sent, uid, date, mp.getExpeditor().getEmail().toString(), media, body.str());
      }
    }
      
    
    string EmailManager::folderList(vector<shared_ptr<vmime::net::folder>> folders){
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
      addresses.push_back("favor.t@null.net");
      
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
	    throw emailException("Unable to determine sent folder");
	  }
	} else if(folders[i]->getName().getBuffer() == "INBOX"){
	  if (!inbox) inbox = folders[i];
	  else {
	    logger::error("Competing INBOX folders");
	    throw emailException("Unable to determine inbox folder");
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
      //TODO: full header fetch may be redundant here, depending on how getting parsed messages works (need trace)
      vector<shared_ptr<vmime::net::message>> messages = inbox->getAndFetchMessages(wantedMessages, 
	  vmime::net::fetchAttributes::STRUCTURE | vmime::net::fetchAttributes::FULL_HEADER | vmime::net::fetchAttributes::UID);
            cout << "size: " << messages.size() << endl;
      for(unsigned int i = 0; i < messages.size(); ++i){
	parseMessage(false, messages[i]);
      }    
             
    }
    
    void EmailManager::fetchContacts(){
      
    }
}