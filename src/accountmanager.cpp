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
    namespace worker {
        namespace {
            extern sqlite3 *db; //TODO: I strongly suspect this is not actually working
        }
        AccountManager::AccountManager(string accNm, MessageType typ, string detailsJson)
                : type(typ), accountName(accNm) {
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
            bool sent;
            if (heldMessages.size() == 0) return;
            else sent = heldMessages[0]->sent;
            //id INTEGER, address TEXT NOT NULL, date INTEGER NOT NULL, charcount INTEGER NOT NULL, media INTEGER NOT NULL
            //TODO: yeah, it's silly that we generate a string here but we only need to make it once. I'm starting to think
            //we should just move the AccountManager into the worker namespace; this would solve a lot of issues both with
            //encapsulation and with presentation because the accountmanager does a lot of writing anyway
            string sql = "INSERT INTO " + (sent ? SENT_TABLE_NAME : RECEIVED_TABLE_NAME) + " VALUES(?,?,?,?,?)";
            for (int i = 0; i < heldMessages.size(); ++i) {
                //TODO: untested
                worker::saveMessage(heldMessages[i], sql);
                cout << "Current measured body size raw: " << heldMessages[i]->body.length() << endl;
                cout << heldMessages[i]->logString() << endl;
                delete heldMessages[i];
            }
            heldMessages.clear();
        }

        void AccountManager::updateMessages() {
            fetchMessages();
            updateFetchData();
            saveHeldMessages();
            saveFetchData();
        }

        bool AccountManager::isWhitespace(uint32_t code) {
            if (code >= 9 && code <= 13) return true;
            else if (code >= 8192 && code <= 8202) return true;
            else
                switch (code) {
                    case 32:
                        return true;
                    case 133:
                        return true;
                    case 160:
                        return true;
                    case 5760:
                        return true;
                    case 8232:
                        return true;
                    case 8233:
                        return true;
                    case 8239:
                        return true;
                    case 8287:
                        return true;
                    default:
                        return false;
                }
        }

        void AccountManager::cleanWhitespace(string &s) {
            //http://en.wikipedia.org/wiki/Whitespace_character
            //TODO: this copies the string into our result array, and then again in the string constructor, which is less than ideal

            //TODO: also remove whitespace if it's the last character
            const char result[s.length()] = {0};
            char *current = (char *) result;

            char *start = (char *) s.c_str();
            char *end = start + s.length();

            bool prevWhitespace = true; //Sneaky way of removing whitespace at the beginning of the string
            while (start != end) {
                uint32_t code = utf8::next(start, end);
                if (isWhitespace(code)) {
                    if (!prevWhitespace && start != end) current = utf8::append(code, current); //Append only if no prev whitespace, and not last iteration
                    prevWhitespace = true;
                } else {
                    current = utf8::append(code, current);
                    prevWhitespace = false;
                }
            }
            s = std::string(result, (const char *) current); //Very important we only copy up to the current iterator, as it's easy to accidentally grab garbage
        }

        //TODO: &msg should really be const. There's just no reason for it not to be. Either solve the root issue here (which is the utf8 library
        //not liking a const iterator) or if we end up having to copy the string - say to remove whitespace efficiently - we should just do the UTF8
        //checks afterwards
        void AccountManager::holdMessage(bool sent, long int id, time_t date, string address, bool media, string msg) {
            //Must be UTF8
            string::iterator utf8End = utf8::find_invalid(msg.begin(), msg.end());
            if (utf8End != msg.end()) {
                logger::warning("Message body with invalid formatting detected.");
                //TODO: log the valid/invalid portions separately
                string temp;
                utf8::replace_invalid(msg.begin(), msg.end(), std::back_inserter(temp));
                msg = temp;
            }
            cleanWhitespace(msg);
            size_t length = utf8::distance(msg.begin(), msg.end());

            message *ex = new message(type, sent, id, date, address, media, msg, length);
            heldMessages.push_back(ex);
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
}