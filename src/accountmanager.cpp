#include "accountmanager.h"
#include "worker.h"
#include "logger.h"

//Managers
#ifdef FAVOR_EMAIL_MANAGER

#include "emailmanager.h"

#endif

#define SENT_TABLE_NAME "\""+accountName+"_"+MessageTypeName[type]+"_sent\""
#define RECEIVED_TABLE_NAME "\""+accountName+"_"+MessageTypeName[type]+"_received\""

#define SENT_INDEX_NAME "i_" SENT_TABLE_NAME
#define RECEIVED_INDEX_NAME "i_" RECEIVED_TABLE_NAME

using namespace std;
namespace favor {
    AccountManager::AccountManager(string accNm, MessageType typ, string detailsJson) : type(typ), accountName(accNm) {
        json.Parse(detailsJson.c_str());
        if (json.HasParseError()) {
            logger::error("Parse error on json: \"" + detailsJson + "\". RapidJson says: " + rapidjson::GetParseError_En(json.GetParseError()));
            throw badAccountDataException("Failed to parse JSON details");
        }
    }

    void AccountManager::buildTables() {
        buildTablesStatic(accountName, type);
    }

    void AccountManager::destroyTables() {
        destroyTablesStatic(accountName, type);
    }

    void AccountManager::truncateSentTable() {
        worker::exec("DELETE FROM " SENT_TABLE_NAME);
    }

    void AccountManager::truncateReceivedTable() {
        worker::exec("DELETE FROM " RECEIVED_TABLE_NAME);
    }

    void AccountManager::truncateTables() {
        truncateSentTable();
        truncateReceivedTable();
    }

    void AccountManager::deindexTables() {
        worker::exec("DROP INDEX IF EXISTS " SENT_INDEX_NAME ";");
        worker::exec("DROP INDEX IF EXISTS " RECEIVED_INDEX_NAME ";");
    }

    void AccountManager::indexTables() {
        worker::exec("CREATE INDEX IF NOT EXISTS " RECEIVED_INDEX_NAME " ON " RECEIVED_TABLE_NAME MESSAGE_INDEX_SCHEMA ";");
        worker::exec("CREATE INDEX IF NOT EXISTS " SENT_INDEX_NAME " ON " SENT_TABLE_NAME MESSAGE_INDEX_SCHEMA ";");
    }

    void AccountManager::updateContacts() {
        fetchContacts();
        //TODO: process results
    }

    void AccountManager::saveFetchData() {
        worker::updateAccountDetails(accountName, type, as_string(json));
    }


    void AccountManager::saveHeldMessages() {
        //TODO: save the messages we have to the database
    }

    void AccountManager::updateMessages() {
        fetchMessages();
        updateFetchData();
        saveHeldMessages();
        saveFetchData();
    }

    void AccountManager::cleanWhitespace(string &s) {
        //http://en.wikipedia.org/wiki/Whitespace_character
        bool prevWhitespace = false;
        //utf8::next
        //TODO: this function
    }

    //TODO: &msg should really be const. There's just no reason for it not to be. Either solve the root issue here (which is the utf8 library
    //not liking a const iterator) or if we end up having to copy the string - say to remove whitespace efficiently - we should just do the UTF8
    //checks afterwards
    void AccountManager::holdMessage(bool sent, long int id, time_t date, string address, bool media, string msg) {
        //Must be UTF8
        string::iterator utf8End = utf8::find_invalid(msg.begin(), msg.end());
        if (utf8End != msg.end()){
            logger::warning("Message body with invalid formatting detected.");
            //TODO: log the valid/invalid portions separately
            string temp;
            utf8::replace_invalid(msg.begin(), msg.end(), std::back_inserter(temp));
            msg = temp;
        }
        size_t length = utf8::distance(msg.begin(), msg.end());

        message* export = new message(type, sent, id, date, address, media, msg, length);
        heldMessages.push_back(export);

        //TODO: strip whitespace

        //TODO: export (save to vector, likely of pointers), and give it a type based on the type of this manager.
        cout << "---------------------------------------------------------" << endl;
        cout << "Message held with - sent: " << sent << ", id: " << id << ", date: " << date << ", address: " << address << ", media: " << media << endl << "...and Body:|" << msg << "|" << endl;
        cout << "Body　Length: " << length << endl;
        cout << "---------------------------------------------------------" << endl << endl;

    }

    //Static methods


    shared_ptr<AccountManager> AccountManager::buildManager(string accNm, favor::MessageType typ, string detailsJson) {
        switch (typ) {
            #ifdef FAVOR_EMAIL_MANAGER
            case TYPE_EMAIL:
                return make_shared<EmailManager>(accNm, detailsJson);
            #endif
            case TYPE_ANDROIDTEXT:
                break;
            default:
                logger::error("Attempt to initialize manager for unsupported type " + as_string(typ));
                assert(false);
        }

    }

    void AccountManager::buildTablesStatic(string accountName, MessageType type) {
        //TODO: index if indexing is enabled
        worker::exec("CREATE TABLE IF NOT EXISTS " SENT_TABLE_NAME SENT_TABLE_SCHEMA ";");
        worker::exec("CREATE TABLE IF NOT EXISTS " RECEIVED_TABLE_NAME RECEIVED_TABLE_SCHEMA ";");
    }

    void AccountManager::destroyTablesStatic(string accountName, MessageType type) {
        worker::exec("DROP TABLE IF EXISTS " SENT_TABLE_NAME ";");
        worker::exec("DROP TABLE IF EXISTS " RECEIVED_TABLE_NAME ";");
    }


}