/*
Copyright (C) 2015  Joshua Tanner (mindful.jt@gmail.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "reader.h"
#include "accountmanager.h"
#include "emailmanager.h"
#include "logger.h"

using namespace vmime;
using namespace vmime::net;

using namespace std;
namespace favor {
    //A poorly expanded version of this: http://www.regular-expressions.info/email.html
    //Added a-z to things because case insensitive compilation wasn't handling character rangers properly (I.E., A-Z -> a-z wasn't happening)
    //also added insulations to avoid consecutive periods and make sure the email doesn't start with one. This made it much longer, and I'm sure
    //could be done much better by someone more experienced with regular expressions than I. The aaddresses given
    //here: http://stackoverflow.com/questions/297420/list-of-email-addresses-that-can-be-used-to-test-a-javascript-validation-script are a good resource
    std::regex EmailManager::emailRegex = std::regex("[a-zA-Z0-9_%+-]+[a-zA-Z0-9._%+-]+[a-zA-Z0-9_%+-]+@[a-zA-Z0-9.-]+[a-zA-Z0-9-]+\\.[a-zA-Z]{2,4}",
                                                     std::regex::ECMAScript | std::regex::icase);
    std::regex EmailManager::utf8regex = std::regex("utf-8", std::regex::ECMAScript | std::regex::icase);

    //TODO: this needs to be updated with more imap servers, somehwat obviously
    const unordered_map<string, string> EmailManager::imapServers({{"gmail.com", "imaps://imap.gmail.com:993"}});


    //TODO: test this with bad URLS, and an unrecognized email+custom URL
    EmailManager::EmailManager(string accNm, string detailsJson)
            : AccountManager(accNm, TYPE_EMAIL, detailsJson), serverURL("imap://bad.url:0") {
        if (!addressValid(accountName)) logger::warning("Account name " + accountName + " for email manager does not match email regex");
        consultJson(true);
    }

    void EmailManager::consultJson(bool initial) {
        if (initial){
            if (json.HasMember("password")) password = json["password"].GetString();
            else throw badUserDataException("EmailManager missing password");

            size_t atSignPosition = accountName.find_first_of("@");
            if (atSignPosition == string::npos) {
                logger::error("Could not find \"@\" in \"" + accountName + "\"");
                throw badUserDataException("EmailManager initialized with bad email address");
            }

            string domain = accountName.substr(atSignPosition + 1);
            unordered_map<string, string>::const_iterator it = imapServers.find(domain);
            if (it != imapServers.end()) {
                serverURL = vmime::utility::url(it->second);
            }
            else {
                if (json.HasMember("url")) {
                    try {
                        serverURL = vmime::utility::url(json["url"].GetString());
                    }
                    catch (vmime::exceptions::malformed_url &e) {
                        logger::error("Attempted to use provided url \"" + string(json["url"].GetString()) + "\" but it was malformed");
                        throw badUserDataException("Provided url was malformed");
                    }
                }
                else {
                    logger::error("Could not determine url for email " + accountName + " and no alternative URL was provided");
                    throw badUserDataException("Unable to determine URL for " + accountName);
                }
            }
        }

        getJsonLong(lastReceivedUid, 1); //Uids start from 1, always
        getJsonLong(lastSentUid, 1);
        getJsonLong(lastReceivedUidValidity, -1); // <0 if we don't know
        getJsonLong(lastSentUidValidity, -1);
    }

    bool EmailManager::addressValid(const string &address) {
        return regex_match(address, emailRegex);
    }

    bool tidyNoErrors(int rc){
        return rc >= 0 && rc < 2;
    }

    bool EmailManager::toXML(stringstream &ss) {
        TidyBuffer output = {0};
        TidyBuffer errbuf = {0};
        int rc;

        TidyDoc tdoc = tidyCreate();                     // Initialize "document";

        tidyOptSetBool(tdoc, TidyXhtmlOut, yes);  // Tell tidy to convert to xtml
        tidyOptSetBool(tdoc, TidyDropFontTags, yes); //Slight efficiency
        tidyOptSetBool(tdoc, TidyHideComments, yes);
        tidyOptSetValue(tdoc, TidyCharEncoding, "UTF8");
        tidyOptSetBool(tdoc, TidyQuoteNbsp, no);
        rc = tidySetErrorBuffer(tdoc, &errbuf);      // This just sets up someplace for error messages
        if (tidyNoErrors(rc)) rc = tidyParseString(tdoc, ss.str().c_str());            // Parse the input string
        if (tidyNoErrors(rc)) rc = tidyCleanAndRepair(tdoc);              // Execute the actual cleaning and repairing after configuring and parsing
        if (tidyNoErrors(rc)) rc = tidyRunDiagnostics(tdoc);              // Kvetch
        //if (rc > 1) rc = (tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1);     //Here in case we need it, but currently, do not force output
        if (tidyNoErrors(rc)) rc = tidySaveBuffer(tdoc, &output);           // Pretty Print

        if (tidyNoErrors(rc)) {
            logger::info("Diagnostics for converted HTML: " + string(reinterpret_cast<const char *>(errbuf.bp)));
        }
        else {
            logger::warning("Unable to parse HTML, TidyLib has errno " + as_string(rc) + " and says: \"" + string(reinterpret_cast<const char *>(errbuf.bp)) + "\"");
            return false;
        }
        ss.str(reinterpret_cast<const char *>(output.bp));
        tidyBufFree(&output);
        tidyBufFree(&errbuf);
        tidyRelease(tdoc);
        return true;
    }


    std::time_t EmailManager::toTime(const vmime::datetime input) {
        struct tm timeinfo;
        const vmime::datetime time = vmime::utility::datetimeUtils::toUniversalTime(input);
        timeinfo.tm_sec = time.getSecond(); DLOG("Seconds:"+as_string(time.getSecond()))
        timeinfo.tm_min = time.getMinute(); DLOG("Minutes:"+as_string(time.getMinute()))
        timeinfo.tm_hour = time.getHour(); DLOG("Hours:"+as_string(time.getHour()))
        timeinfo.tm_mday = time.getDay(); DLOG("Day:"+as_string(time.getDay()))
        timeinfo.tm_mon = time.getMonth() - 1; //tm specification is "Months since January"
        timeinfo.tm_year = time.getYear() - 1900; //tm specification is "Years since 1900"
        timeinfo.tm_isdst = 0; //No daylight savings in universal time, I believe
        time_t ret = to_time_t_utc(&timeinfo);
        return ret;
    }


    bool EmailManager::hasMedia(shared_ptr<vmime::net::messageStructure> structure) {
        for (int i = 0; i < structure->getPartCount(); ++i) {
            shared_ptr<vmime::net::messagePart> part = structure->getPartAt(i);
            if (part->getPartCount()) {
                if (hasMedia(part->getStructure())) return true;
            }
            else {
                string partType = part->getType().getType();
                if (partType != vmime::mediaTypes::TEXT && partType != vmime::mediaTypes::MULTIPART) return true;
            }
        }
        return false;
    }

    //Worth noting that out is hooked into SS here, so when we extract into it it ends up writing to SS
    void EmailManager::handleHTMLandEncoding(vmime::utility::outputStream *out, stringstream &ss,
                                             shared_ptr<const vmime::htmlTextPart> part, string &output) {
        //If vmime can't find a plaintext equivalent part, getPlainText() will be empty
        std::regex utf8regex("utf-8", std::regex::ECMAScript | std::regex::icase);

        if (part->getPlainText()->isEmpty()) {
            part->getText()->extract(*out);
            out->flush();
            if (!regex_match(part->getCharset().getName(), utf8regex)){
                DLOG("Converting HTML mail with charset " + part->getCharset().getName() + " to utf-8");
                string preConvert = ss.str();
                ss.clear();
                ss << to_utf8(preConvert, part->getCharset().getName());
            }

            bool xmlSuccess = toXML(ss);
            pugi::xml_document doc;
            pugi::xml_parse_result res = doc.load(ss);
            if (res) ss.str(stripXML(doc));
            else {
                if (xmlSuccess) logger::error("PugiXML could not parse HTML despite tidying and transformation to XML");
                else logger::error("HTML could not be tidied or parsed. Likely malformed");
                throw badMessageDataException("Unable to process HTML in email message");
            }
            output = ss.str();
        }
        //If there is a plaintext part, we have either already gotten it or will get it later in the iteration and this method doesn't
        //need to do anything.
    }

    bool EmailManager::parseMessageParts(long uid, const vmime::messageParser& mp, string& bodyFinal){

        std::stringstream bodyStream;
        vmime::utility::outputStreamAdapter os(bodyStream);

        for (int i = 0; i < mp.getTextPartCount(); ++i) {
            vmime::shared_ptr<const vmime::textPart> tp = mp.getTextPartAt(i);
            if (tp->getType().getSubType() == vmime::mediaTypes::TEXT_HTML) {
                shared_ptr<const vmime::htmlTextPart> htp = dynamic_pointer_cast<const vmime::htmlTextPart>(tp);
                try{
                    DLOG("Handling html for mail with UID "+as_string(uid));
                    handleHTMLandEncoding(&(os), bodyStream, htp, bodyFinal);
                } catch (badMessageDataException& e){
                    logger::warning("Failed to parse html text part of message with UID "+as_string(uid));
                    return false;
                }
            }
            else if (tp->getType().getSubType() == vmime::mediaTypes::TEXT_PLAIN) {
                tp->getText()->extract(os);
                os.flush(); //the stringstream "body" is what actually gets written to here
                if (!regex_match(tp->getCharset().getName(), utf8regex)) {
                    DLOG("Converting plaintext mail with UID " + as_string(uid) + " and charset " + tp->getCharset().getName() + " to utf-8");
                    bodyFinal = to_utf8(bodyStream.str(), tp->getCharset().getName());
                } else bodyFinal = bodyStream.str();
            }
            else {
                logger::warning("Failed to parse message with unfamiliar text part of type: "+tp->getType().getType()+" with UID "+as_string(uid));
                return false;
            }
        }

        return true;

    }

    void EmailManager::holdParsedMessage(bool failure, const vmime::messageParser &mp, bool sent, long uid, time_t date,
                                         bool media, string bodyFinal){
        if (sent) {
            const vmime::addressList addrList = mp.getRecipients();
            for (int i = 0; i < addrList.getAddressCount(); ++i) {
                shared_ptr<const vmime::address> addr = addrList.getAddressAt(i);
                shared_ptr<const vmime::mailbox> resultAddr;
                if (addr->isGroup()) {
                    //I don't think we'll actually ever get a group here given that multiple recipients seems to result in a longer addrList, but it's better safe than sorry
                    shared_ptr<const vmime::mailboxGroup> grp = dynamic_pointer_cast<const vmime::mailboxGroup>(addr);
                    //Remember email addresses should be converted to lowercase first
                    for (int i = 0; i < grp->getMailboxCount(); ++i) {
                        if (failure) holdMessageFailure(sent, uid, lowercase(grp->getMailboxAt(i)->getEmail().toString()));
                        else holdMessage(sent, uid, date, lowercase(grp->getMailboxAt(i)->getEmail().toString()), media, bodyFinal);
                    }
                }
                else {
                    resultAddr = dynamic_pointer_cast<const vmime::mailbox>(addr);
                    if (failure) holdMessageFailure(sent, uid, lowercase(resultAddr->getEmail().toString()));
                    else holdMessage(sent, uid, date, lowercase(resultAddr->getEmail().toString()), media, bodyFinal);
                }
            }
        }
        else {
            holdMessage(sent, uid, date, lowercase(mp.getExpeditor().getEmail().toString()), media, bodyFinal);
        }
    }


    void EmailManager::parseMessage(bool sent, shared_ptr<vmime::net::message> m) {
        long &lastUid = sent ? lastSentUid : lastReceivedUid;

        shared_ptr<vmime::message> parsedMessage = m->getParsedMessage();
        vmime::messageParser mp(parsedMessage);

        string bodyFinal;

        /*Strangely, if we get the date like this:
        * shared_ptr<const vmime::datetime> rawdate = m->getHeader()->Date()->getValue<vmime::datetime>();
        * it reports a one second difference (earlier) than if we get it the way we're doing below.
        * Probably just a rounding quirk, but something word recording */

        const time_t date = toTime(mp.getDate());

        const long uid = stoi(m->getUID()); //stoi function may not be implemented on Android but this mail client is dekstop-only anyway
        lastUid = (uid > lastUid) ? uid : lastUid; //Update our last UID to this new one if it's larger (which it almost always should be)
        const bool media = hasMedia(m->getStructure());


        bool failure = !parseMessageParts(uid, mp, bodyFinal);

        DLOG("Finalize message with body:"+bodyFinal);

        holdParsedMessage(failure, mp, sent, uid, date, media, bodyFinal);
    }


    string EmailManager::folderList(vector<shared_ptr<vmime::net::folder>> folders) {
        string out("[");
        for (int i = 0; i < folders.size(); ++i) {
            out += "\"" + folders[i]->getName().getBuffer() + "\"";
            if (i != (folders.size() - 1)) out += ", ";
        }
        out += "]";
        return out;
    }

    shared_ptr<vmime::net::store> EmailManager::login() {
        shared_ptr<session> sess = make_shared<session>();

        //TODO: test this on a normal IMAP server (if any don't use IMAPS anymore, anyway)
        if (serverURL.getProtocol() == "imaps") {
            sess->getProperties()["store.imaps.auth.username"] = accountName;
            sess->getProperties()["store.imaps.auth.password"] = password;
        }
        else {
            sess->getProperties()["store.imap.auth.username"] = accountName;
            sess->getProperties()["store.imap.auth.username"] = password;
        }

        vmime::shared_ptr<vmime::net::store> st = sess->getStore(serverURL);

        try {
            st->setCertificateVerifier(vmime::make_shared<TrustingCertificateVerifier>());
            st->setTracerFactory(make_shared<InfoTracerFactory>());
            st->connect();
            logger::info("Successfully connected to " + string(serverURL) + "as " + accountName);
        }
        catch (vmime::exceptions::authentication_error &e) {
            logger::error("Could not authenticate to " + string(serverURL) + " as " + accountName + " with the credentials provided");
            throw authenticationException();
        }
            //I've only ever seen authentication_error, but we'll catch the similarly named exception here just in case...
        catch (vmime::exceptions::authentication_exception &e) {
            logger::error("Could not authenticate to " + string(serverURL) + " as " + accountName + " with the credentials provided");
            throw authenticationException();
        }
        catch (vmime::exception &e) {
            logger::error("Error authenticating to email server: " + string(e.what()));
            throw emailException();
        }


        return st;
    }

    string EmailManager::searchCommand(bool sent, shared_ptr<const vector<Address>> addresses, long startUid, long endUid = -1) {
        string cmd("");
        string addressField = sent ? "TO" : "FROM";
        for (int i = 1; i < addresses->size(); ++i) {cmd += "OR ";} //One less "OR " than the number of addresses is important, so we start from 1 here
        for (auto it = addresses->begin(); it != addresses->end(); ++it) {cmd += (addressField + " \"" + it->addr + "\" ");}
        if (endUid != -1) cmd += ("UID " + as_string(startUid + 1) + ":" + as_string(endUid));
        else cmd += ("UID " + as_string(startUid) + ":*"); //In the case where we have no endUid, it is possible we will pick up a duplicate message.
        return cmd;
    }


    SentRec<shared_ptr<vmime::net::folder>> EmailManager::findSentRecFolder(shared_ptr<vmime::net::store> st) {
        vector<shared_ptr<vmime::net::folder>> folders = st->getRootFolder()->getFolders(true);
        std::regex sentRegex("sent", std::regex::ECMAScript | std::regex::icase);
        shared_ptr<vmime::net::folder> sent = null;
        shared_ptr<vmime::net::folder> inbox = null;

        for (int i = 0; i < folders.size(); ++i) {
            if (regex_search(folders[i]->getName().getBuffer(), sentRegex)) {
                if (!sent) sent = folders[i];
                else {
                    logger::error("Competing sent folders in " + folderList(folders));
                    throw emailException("Unable to determine sent folder");
                }
            } else if (folders[i]->getName().getBuffer() == "INBOX") {
                if (!inbox) inbox = folders[i];
                else {
                    logger::error("Competing INBOX folders");
                    throw emailException("Unable to determine inbox folder");
                }
            }
        }

        if (sent) logger::info("Found sent folder as \"" + sent->getName().getBuffer() + "\"");
        else {
            logger::error("Could not find sent folder in " + folderList(folders));
            throw emailException("Could not find sent folder");
        }

        if (inbox) logger::info("Found \"INBOX\"");
        else {
            logger::error("Could not find INBOX in " + folderList(folders));
            throw emailException("Could not find INBOX");
        }

        return SentRec<shared_ptr<vmime::net::folder>>(sent, inbox);

    }

    void EmailManager::fetchFromFolder(shared_ptr<vmime::net::folder> folder, shared_ptr<const vector<Address>> addresses, bool catchUp = false) {
        bool sent = (folder->getName().getBuffer() != "INBOX");
        long &lastUid = sent ? lastSentUid : lastReceivedUid;
        long &lastUidValidity = sent ? lastSentUidValidity : lastReceivedUidValidity;

        //TODO: test uidvalidity change stuff
        shared_ptr<vmime::net::imap::IMAPFolderStatus> status = dynamic_pointer_cast<vmime::net::imap::IMAPFolderStatus>(folder->getStatus());

        long uidValidity = status->getUIDValidity();
        if (lastUidValidity < 0) lastUidValidity = uidValidity;
        if (uidValidity == 0) logger::warning("Server does not support UID validity");
        else if (lastUidValidity != uidValidity) {
            logger::warning("UID Validity in folder " + folder->getName().getBuffer() + " has changed from " + as_string(lastUidValidity) + " to " + as_string(uidValidity));
            //Drop the database, and reset our last UID as well because it's moot now.
            if (sent) truncateSentTable();
            else truncateReceivedTable();
            lastUid = 1; //UIDs always from 1
            lastUidValidity = uidValidity;
        }

        long nextUid = status->getUIDNext();
        string cmd;
        if (catchUp){
            if (lastUid == 1) return; //There's no need for a catch up fetch if the normal one is going to get everything anyway
            cmd = searchCommand(sent, addresses, 1, lastUid); //From the first message to the last one we're ignoring
        }
        else {
            if (nextUid != 0) {
                //Have to add one to startUid here or we'll keep getting the last message we fetched
                cmd = searchCommand(sent, addresses, lastUid+1, nextUid);
            }
            else {
                //This represents a case where we're likely to get the last item in the mailbox whether we like it or not,
                //because we don't know what the largest UID is so we have to use *, meaning we are gauranteed to fetch at least the last message which may be a duplicate
                logger::warning("Server would not provide next UID");
                cmd = searchCommand(sent, addresses, lastUid+1);
            }
        }

        if (!folder->isOpen()) folder->open(vmime::net::folder::MODE_READ_ONLY);
        else logger::warning("Folder \"" + folder->getName().getBuffer() + "\" already open before fetch. This should not happen.");

        vmime::net::messageSet wantedMessages(folder->UIDSearch(cmd));
        vector<shared_ptr<vmime::net::message>> messages = folder->getAndFetchMessages(wantedMessages,
                vmime::net::fetchAttributes::STRUCTURE | vmime::net::fetchAttributes::FULL_HEADER | vmime::net::fetchAttributes::UID);
        if (messages.size() == 0) return;
        else for (unsigned int i = 0; i < messages.size(); ++i) parseMessage(sent, messages[i]);

        folder->close(false);
    }

    void EmailManager::updateJson() {
        setJsonLong(lastReceivedUid);
        setJsonLong(lastSentUid);
        setJsonLong(lastReceivedUidValidity);
        setJsonLong(lastSentUidValidity);
        rapidjson::Value addrsVal;
        addrsVal.SetArray();
        for (auto it = managedAddresses.begin(); it != managedAddresses.end(); ++it){
            addrsVal.PushBack(rapidjson::Value(it->c_str(), json.GetAllocator()).Move(), json.GetAllocator());
        }
        json[managedAddrListName] = addrsVal;
    }

    void EmailManager::fetchMessages() {

        shared_ptr<vector<Address>> addresses  = contactAddresses();
        shared_ptr<vector<Address>> newAddresses  = make_shared<vector<Address>>();


        if (addresses->size() == 0){
            logger::info("Account "+accountName+" fetchMessages returned because no addresses found");
            return;
        }

        try {
            vmime::shared_ptr<vmime::net::store> st = login();

            SentRec<shared_ptr<vmime::net::folder>> sentRecFolders = findSentRecFolder(st);
            for (auto it = addresses->begin(); it != addresses->end(); ++it){
                if (!managedAddresses.count(it->addr)){
                    logger::info("New email address "+it->addr+" detected");
                    newAddresses->push_back(*it);
                }
            }
            if (newAddresses->size()){
                //If there are new addresses, we run catchup fetches before the new normal fetch
                fetchFromFolder(sentRecFolders.sent, newAddresses, true);
                fetchFromFolder(sentRecFolders.received, newAddresses, true);
            }
            fetchFromFolder(sentRecFolders.sent, addresses); //Sent folder
            fetchFromFolder(sentRecFolders.received, addresses); //Receied folder
            //st->disconnect(); //It seems like we'd want to call this, but the disconnect command is issued fine without it, and VMIME actually starts spitting out threading
            //errors if we use it, so it's exempted from this and fetchAddresses(). I suspect the reason is that something along these lines happens automatically when it falls
            //out of scope
        }
        catch (vmime::exceptions::connection_error &e) {
            logger::error("Error connecting to " + serverURL.getHost() + ". This can mean a bad host or no internet connectivity");
            throw networkConnectionException("Connectivity error or bad host");
        }
        catch (vmime::exception &e) {
            logger::error("Unhandled VMIME exception, says: " + string(e.what()));
            throw emailException();
        }

        //Many of these inserts will be redundant, but it's just our way of updating the fetch data
        for (auto it = addresses->begin(); it != addresses->end(); ++it) managedAddresses.insert(it->addr);
    }


    void EmailManager::processFetchedAddresses(vector<shared_ptr<vmime::net::message>> fetchedAddresses){

        std::unordered_map<string, std::unordered_map<string, int>> nameOccurenceCount;

        for (int i = 0; i < fetchedAddresses.size(); ++i) {
            shared_ptr<const vmime::addressList> addrList = fetchedAddresses[i]->getHeader()->To()->getValue<vmime::addressList>();


            for (int i = 0; i < addrList->getAddressCount(); ++i) {
                shared_ptr<const vmime::address> vmimeAddress = addrList->getAddressAt(i); //vmime "addresses" can be multiple addresses
                shared_ptr<const vmime::mailbox> mailbox; //Mailbox is vmime's somehwhat confusing term for an actual address...
                if (vmimeAddress->isGroup()) mailbox = dynamic_pointer_cast<const vmime::mailboxGroup>(vmimeAddress)->getMailboxAt(0);
                else mailbox = dynamic_pointer_cast<const vmime::mailbox>(vmimeAddress);

                string addressString = lowercase(mailbox->getEmail().toString()); //Have to lowercase email addresses

//                //While most other mediums are going to have a clean 1:1 mapping, different email clients can send
//                //differet names from the same address, so we just pick the most common one
//
//                auto words = mailbox->getName().getWordList();
//                if (words.size() != 0){
//                    nameOccurenceCount[addressString][words[0]->getConvertedText(vmime::charsets::UTF_8)]+= 1;
//                }
//
//                for (auto it = nameOccurenceCount.begin(); it != nameOccurenceCount.end(); ++it){
//                    int localMaxNameCount = 0;
//                    const string* mostCommonName;
//                    for (auto innerIt = it->second.begin(); innerIt != it->second.end(); ++innerIt){
//                        if (innerIt->second > localMaxNameCount){
//                            localMaxNameCount = innerIt->second;
//                            mostCommonName = &(it->first);
//                        }
//                    }
//                    setCountedAddressName(it->first, *mostCommonName);
//                }

                countAddress(addressString);
            }
        }
    }


    void EmailManager::fetchAddresses() {
        try {
            vmime::shared_ptr<vmime::net::store> st = login();
            SentRec<shared_ptr<vmime::net::folder>> sentRecFolders = findSentRecFolder(st);
            shared_ptr<vmime::net::folder> sent = sentRecFolders.sent;
            sent->open(vmime::net::folder::MODE_READ_ONLY);

            long nextUid = dynamic_pointer_cast<vmime::net::imap::IMAPFolderStatus>(sent->getStatus())->getUIDNext();
            if (nextUid == 0) {
                if (lastSentUid > 1) {
                    logger::warning("Could not retrieve next UID from server, so using last fetched sent UID to determine recent messages for address fetching");
                    nextUid = lastSentUid;
                } else {
                    logger::error("Could not retrieve next UID and no previous UID fetches on this server, so could not determine recent messages");
                    throw emailException("Server failed to provide next UID");
                }
            }

            nextUid--; //To account for the fact that this UID doesn't actually exist yet
            long minUid = nextUid - ADDRESS_CHECK_MESSAGE_COUNT;
            if (minUid < 1) minUid = 1;
            vmime::net::messageSet wantedMessages(vmime::net::messageSet::byUID(minUid, nextUid));
            vmime::net::fetchAttributes attribs;
            attribs.add("To");
            vector<shared_ptr<vmime::net::message>> fetchedAddresses = sent->getAndFetchMessages(wantedMessages, attribs);
            sent->close(false);

            processFetchedAddresses(fetchedAddresses);
        }
        catch (vmime::exceptions::connection_error &e) {
            logger::error("Error connecting to " + serverURL.getHost() + ". This can mean a bad host or no internet connectivity");
            throw networkConnectionException("Connectivity error or bad host");
        }
        catch (vmime::exception &e) {
            logger::error("Unhandled VMIME exception, says: " + string(e.what()));
            throw emailException();
        }

    }

}