#include "accountmanager.h"
#include "emailmanager.h"
#include "logger.h"

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
namespace favor {
    namespace email { 
      
      string stripXML(const pugi::xml_document& doc){
	string withoutHTML;
	pugi::xpath_node_set ns = doc.select_nodes("//text()");
	if (ns.type() != pugi::xpath_node_set::type_sorted) ns.sort();
	for (size_t i = 0; i < ns.size(); ++i) {
	  withoutHTML += ns[i].node().value();
	  if (i < (ns.size()-1)) withoutHTML += " ";
	}
	return withoutHTML;
      }
      
      bool toXML(stringstream& ss){
	TidyBuffer output = {0};
	TidyBuffer errbuf = {0};
	int rc = -1;

	TidyDoc tdoc = tidyCreate();                     // Initialize "document";

	tidyOptSetBool( tdoc, TidyXhtmlOut, yes );  // Tell tidy to convert to xtml
	tidyOptSetBool( tdoc, TidyDropFontTags, yes); //Slight efficiency
	tidyOptSetBool( tdoc, TidyHideComments, yes);
	tidyOptSetValue(tdoc, TidyCharEncoding, "UTF8");
	tidyOptSetBool(tdoc, TidyQuoteNbsp, no);
	rc = tidySetErrorBuffer( tdoc, &errbuf );      // This just sets up someplace for error messages
	if ( rc >= 0 ) rc = tidyParseString( tdoc, ss.str().c_str() );           // Parse the input string
	if ( rc >= 0 ) rc = tidyCleanAndRepair( tdoc );               // Execute the actual cleaning and repairing after configuring and parsing
	if ( rc >= 0 ) rc = tidyRunDiagnostics( tdoc );               // Kvetch
	if ( rc > 1 ) rc = ( tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1 );                            // If error, force output.
	if ( rc >= 0 ) rc = tidySaveBuffer( tdoc, &output );          // Pretty Print

	if ( rc >= 0 )
	{
	  if ( rc > 0 ) logger::info("Diagnostics for converted HTML: "+string(reinterpret_cast<const char*>(errbuf.bp)));
	}
	else {
	  logger::warning("Unable to parse HTML, TidyLib has errno "+as_string(rc)+" and says: \""+string(reinterpret_cast<const char*>(errbuf.bp))+"\"" );
	  return false;
	}
	ss.str(reinterpret_cast<const char*>(output.bp));
	tidyBufFree( &output );
	tidyBufFree( &errbuf );
	tidyRelease( tdoc );
      }
      
      
      const unordered_map<string, string> imapServers({{"gmail.com", "imaps://imap.gmail.com:993"}});

      
      class InfoTracer : public vmime::net::tracer
      {
      public:
	void traceSend(const string& line) override {
	  logger::info("["+service->getProtocolName()+"] Sent: "+line);
	}
	
	void traceReceive(const string& line) override {
	  logger::info("["+service->getProtocolName()+"] Received: "+line);
	}
	
	InfoTracer(vmime::shared_ptr <vmime::net::service> serv, const int id) : service(serv), connectionId(id) {}
      private:
	vmime::shared_ptr <vmime::net::service> service;
	const int connectionId;
      };
      
      class InfoTracerFactory : public vmime::net::tracerFactory {
      public: 
	shared_ptr <vmime::net::tracer> create (shared_ptr <vmime::net::service> serv, const int connectionId) override{
	  return make_shared<InfoTracer>(serv, connectionId);
	}
	
      };
      
      
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
    }

    //TODO: test this with bad URLS, and an unrecognized email+custom URL
    EmailManager::EmailManager(string accNm, string detailsJson) : AccountManager(accNm, TYPE_EMAIL, detailsJson), serverURL("imap://bad.url:0")
    {
      if (json.HasMember("password")) password = json["password"].GetString();
      else throw badAccountDataException("EmailManager missing password");
      
      //http://www.regular-expressions.info/email.html
      std::regex emailRegex("[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}", std::regex::ECMAScript | std::regex::icase);
      if (!regex_match( accountName, emailRegex)) logger::warning("Account name "+accountName+" for email manager does not match email regex");
      
      size_t atSign = accountName.find_first_of("@");
      if (atSign==string::npos){
	logger::error("Could not find \"@\" in \""+accountName+"\"");
	throw badAccountDataException("EmailManager initialized with bad email address");
      }
      
      string domain = accountName.substr(atSign+1);
      unordered_map<string, string>::const_iterator it = email::imapServers.find(domain);
      if (it != email::imapServers.end()) {
	serverURL = vmime::utility::url(it->second);
      }
      else {
	if (json.HasMember("url")) {
	  try {
	  serverURL = vmime::utility::url(json["url"].GetString());
	  }
	  catch (vmime::exceptions::malformed_url &e) {
	    logger::error("Attempted to use provided url \""+string(json["url"].GetString())+"\" but it was malformed");
	    throw badAccountDataException("Provided url was malformed");
	  }
	}
	else {
	  logger::error("Could not determine url for email "+accountName+" and no alternative URL was provided");
	  throw badAccountDataException("Unable to determine URL for "+accountName);
	}
      }
      
      
      
      lastReceivedUid = json.HasMember("lastReceivedUid") ? json["lastReceivedUid"].GetInt64() : 1; //Uids start from 1, always
      lastSentUid = json.HasMember("lastSentUid") ? json["lastSentUid"].GetInt64() : 1;
      
      lastReceivedUidValidity = json.HasMember("lastReceivedUidValidity") ? json["lastReceivedUidValidity"].GetInt64() : -1; // <0 if we don't know
      lastSentUidValidity = json.HasMember("lastSentUidValidity") ? json["lastSentUidValidity"].GetInt64() : -1;
    }

    
    std::time_t EmailManager::toTime(const vmime::datetime input){
      //TODO: test this to verify this works properly
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
    
    void EmailManager::handleHTML(vmime::utility::outputStream* out, stringstream& ss, shared_ptr<const vmime::htmlTextPart> part){
	//If this HTML part has a plain text equivalent, use that
	if (!part->getPlainText()->isEmpty()){
	  part->getPlainText()->extract(*out);
	  out->flush();
	}
	else {
	  part->getText()->extract(*out);
	  out->flush();
	  bool xmlSuccess = email::toXML(ss);
	  pugi::xml_document doc;
	  pugi::xml_parse_result res = doc.load(ss);
	  if (res) ss.str(email::stripXML(doc));
	  else {
	    if (xmlSuccess) logger::error("PugiXML could not parse HTML despite tidying and transformation to XML");
	    else logger::error("HTML could not be tidied or parsed. Likely malformed");
	    throw badMessageDataException("Unable to process HTML in email message");
	  }
	}
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
      
      std::regex utf8regex("utf-8", std::regex::ECMAScript | std::regex::icase);
      
      
      for (int i = 0; i < mp.getTextPartCount(); ++i){
	vmime::shared_ptr<const vmime::textPart> tp = mp.getTextPartAt(i);
	if (tp->getType().getSubType() == vmime::mediaTypes::TEXT_HTML){
	  //TODO: handleHTMl can fail, and except. how do we handle that?
	  shared_ptr<const vmime::htmlTextPart> htp = dynamic_pointer_cast<const vmime::htmlTextPart>(tp);
	  if (!regex_match(htp->getCharset().getName(), utf8regex)){
	    //Encoding is supposedly handled by VMIME, but we'll need to look at charset
	    //TODO: test this converting from a non utf-8 charset. Everybody seems to send everything already in UTF-8 these days
	    logger::info("Converting mail with UID "+string(m->getUID())+" and charset "+htp->getCharset().getName()+" to utf-8");
	    shared_ptr<vmime::charsetConverter> conv = vmime::charsetConverter::create(htp->getCharset(), vmime::charset("utf-8"));
	    shared_ptr<vmime::utility::charsetFilteredOutputStream> out = conv->getFilteredOutputStream(os);
	    handleHTML(&(*out), body, htp);
	  }
	  else {
	    handleHTML(&os, body, htp);
	  }
	}
	else if (tp->getType().getSubType() == vmime::mediaTypes::TEXT_PLAIN){
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
      shared_ptr<session> sess = make_shared <session>();

      //TODO: test this on a normal IMAP server (if any don't use IMAPS anymore, anyway)
      if(serverURL.getProtocol() == "imaps"){
	sess->getProperties()["store.imaps.auth.username"] = accountName;
	sess->getProperties()["store.imaps.auth.password"] = password;
      }
      else {
	sess->getProperties()["store.imap.auth.username"] = accountName;
	sess->getProperties()["store.imap.auth.username"] = password;
      }

      vmime::shared_ptr<vmime::net::store> st = sess->getStore(serverURL);
      
      try {
      st->setCertificateVerifier(vmime::make_shared<email::TrustingCertificateVerifier>());
      st->setTracerFactory(make_shared<email::InfoTracerFactory>());
      st->connect();
      logger::info("Successfully connected to "+string(serverURL)+"as "+accountName);
      } 
      catch (vmime::exceptions::authentication_error& e){
	logger::error("Could not authenticate to "+string(serverURL)+" as "+accountName+" with the credentials provided");
	throw authenticationException();
      }
      //I've only ever seen authentication_error, but we'll catch the similarly named exception here just in case...
      catch (vmime::exceptions::authentication_exception& e){
	logger::error("Could not authenticate to "+string(serverURL)+" as "+accountName+" with the credentials provided");
	throw authenticationException();
      } 
      catch (vmime::exception& e){
 	logger::error("Error authenticating to email server: "+string(e.what()));
 	throw emailException();
      }
      
      
      
      return st;
    }
    
    string EmailManager::searchCommand(bool sent, const vector<string>& addresses, long uid){
      string cmd("");
      string addressField = sent ? "TO" : "FROM";
      for (int i = 1; i < addresses.size(); ++i){cmd+="OR ";} //One less "OR " than the number of addresses is important, so we start from 1 here
      for (int i = 0; i < addresses.size(); ++i){cmd+=(addressField+" \""+addresses[i]+"\" ");}
      cmd+=("UID "+as_string(uid)+":*");
      return cmd;
    }
    
    pair<shared_ptr<vmime::net::folder>, shared_ptr<vmime::net::folder>> EmailManager::findSentRecFolder(shared_ptr<vmime::net::store> st){
      vector<shared_ptr<vmime::net::folder>> folders = st->getRootFolder()->getFolders(true);
      std::regex sentRegex("sent", std::regex::ECMAScript | std::regex::icase);
      shared_ptr<vmime::net::folder> sent = null;
      shared_ptr<vmime::net::folder> inbox = null;
      
      for (int i = 0; i < folders.size(); ++i){
	if (regex_search(folders[i]->getName().getBuffer(), sentRegex)){
	  if (!sent) sent = folders[i];
	  else {
	    logger::error("Competing sent folders in "+folderList(folders));
	    throw emailException("Unable to determine sent folder");
	  }
	} else if (folders[i]->getName().getBuffer() == "INBOX"){
	  if (!inbox) inbox = folders[i];
	  else {
	    logger::error("Competing INBOX folders");
	    throw emailException("Unable to determine inbox folder");
	  }
	}
      }
      
      if (sent) logger::info("Found sent folder as \"" + sent->getName().getBuffer() + "\"");
      else{
	logger::error("Could not find sent folder in "+folderList(folders));
	throw emailException("Could not find sent folder");
      }
      
      if (inbox) logger::info("Found \"INBOX\"");
      else{
	logger::error("Could not find INBOX in "+folderList(folders));
	throw emailException("Could not find INBOX");
      }
      
      return pair<shared_ptr<vmime::net::folder>, shared_ptr<vmime::net::folder>>(sent, inbox);
     
    }
    
    void EmailManager::fetchFromFolder(shared_ptr<vmime::net::folder> folder, const vector<string>& addresses){
      bool sent = (folder->getName().getBuffer() != "INBOX");
      long& lastUid = sent ? lastSentUid : lastReceivedUid;
      long& lastUidValidity = sent? lastSentUidValidity : lastSentUidValidity;
      
      //TODO: test uidvalidity change stuff
      shared_ptr<vmime::net::imap::IMAPFolderStatus> status = dynamic_pointer_cast<vmime::net::imap::IMAPFolderStatus>(folder->getStatus());
      long uidValidity = status->getUIDValidity();
      if (lastUidValidity == -1) lastUidValidity = uidValidity;
      if (uidValidity == 0) logger::warning("Server does not support UID validity");
      else if (lastUidValidity != uidValidity) {
	logger::warning("UID Validity in folder "+folder->getName().getBuffer()+" has changed from "+as_string(lastUidValidity)+" to "+as_string(uidValidity));
	//Drop the database, and reset our last UID as well because it's moot now. Eventual todo: pun about base dropping
	if (sent) truncateSentTable();
	else truncateReceivedTable();
	lastUid = 1; //UIDs always from 1
      }
      string cmd = searchCommand(sent, addresses, lastUid);
      if (!folder->isOpen()) folder->open(vmime::net::folder::MODE_READ_ONLY);
      else logger::warning("Folder \""+folder->getName().getBuffer()+"\" already open before fetch. This should not happen.");
      
      vmime::net::messageSet wantedMessages(folder->UIDSearch(cmd));
      vector<shared_ptr<vmime::net::message>> messages = folder->getAndFetchMessages(wantedMessages, 
	  vmime::net::fetchAttributes::STRUCTURE | vmime::net::fetchAttributes::FULL_HEADER | vmime::net::fetchAttributes::UID);
      if (messages.size()==0) return;
      else for (unsigned int i = 0; i < messages.size(); ++i) parseMessage(false, messages[i]);  
    }
    
    void EmailManager::updateFetchData(){
      //TODO: write our updated UIDS and last UID validities to the JSON doc
    }
    
    void EmailManager::fetchMessages(){
      
      //TODO: get actual data from the database
      vector<string> addresses;
      addresses.push_back("favor.t@null.net");
      
      if (addresses.size() == 0) return;
      
      try{
	vmime::shared_ptr<vmime::net::store> st = login();
	
	pair<shared_ptr<vmime::net::folder>, shared_ptr<vmime::net::folder>> sentRecFolders = findSentRecFolder(st);
	fetchFromFolder(sentRecFolders.first, addresses); //Sent folder
	fetchFromFolder(sentRecFolders.second, addresses); //Receied folder
      }
      catch (vmime::exceptions::connection_error& e){
 	logger::error("Error connecting to "+serverURL.getHost()+". This can mean a bad host or no internet connectivity");
 	throw networkConnectionException("Connectivity error or bad host");
      }
      catch (vmime::exception& e){
	logger::error("Unhandled VMIME exception, says: "+string(e.what()));
	throw emailException();
      }
      updateFetchData();
   
    }
    
    void EmailManager::fetchContacts(){
      //TODO: this
    }
    
}