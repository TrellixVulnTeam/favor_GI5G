#include "accountmanager.h"
#include "worker.h"
#include "logger.h"
#include "reader.h"

//Managers
#ifdef FAVOR_EMAIL_MANAGER

#include "emailmanager.h"

#endif

#ifdef FAVOR_SKYPE_MANAGER

#include "skypemanager.h"

#endif

#ifdef FAVOR_LINE_MANAGER

#include "linemanager.h"

#endif

#ifdef ANDROID

#include "androidtextmanager.h"

#endif

using namespace std;
namespace favor {
    namespace worker {
        AccountManager::AccountManager(string accNm, MessageType typ, string detailsJson)
                : type(typ), accountName(accNm) {
            json.Parse(detailsJson.c_str());
            if (json.HasParseError()) {
                logger::error("Parse error on json: \"" + detailsJson + "\". RapidJson says: " + rapidjson::GetParseError_En(json.GetParseError()));
                throw badUserDataException("Failed to parse JSON details for account "+accountName);
            }
        }

        AccountManager::~AccountManager(){}

        void AccountManager::buildTables() {
            //TODO: index if indexing is enabled
            exec("CREATE TABLE IF NOT EXISTS " SENT_TABLE_NAME SENT_TABLE_SCHEMA ";");
            exec("CREATE TABLE IF NOT EXISTS " RECEIVED_TABLE_NAME RECEIVED_TABLE_SCHEMA ";");
        }

        void AccountManager::destroyTables() {
            exec("DROP TABLE IF EXISTS " SENT_TABLE_NAME ";");
            exec("DROP TABLE IF EXISTS " RECEIVED_TABLE_NAME ";");
        }

        void AccountManager::truncateSentTable() {
            exec("DELETE FROM " SENT_TABLE_NAME ";");
        }

        void AccountManager::truncateReceivedTable() {
            exec("DELETE FROM " RECEIVED_TABLE_NAME ";");
        }

        void AccountManager::truncateTables() {
            truncateSentTable();
            truncateReceivedTable();
        }

        void AccountManager::deindexTables() {
            exec("DROP INDEX IF EXISTS " SENT_INDEX_NAME ";");
            exec("DROP INDEX IF EXISTS " RECEIVED_INDEX_NAME ";");
        }

        void AccountManager::indexTables() {
            exec("CREATE INDEX IF NOT EXISTS " RECEIVED_INDEX_NAME " ON " RECEIVED_TABLE_NAME MESSAGE_INDEX_SCHEMA ";");
            exec("CREATE INDEX IF NOT EXISTS " SENT_INDEX_NAME " ON " SENT_TABLE_NAME MESSAGE_INDEX_SCHEMA ";");
        }

        void AccountManager::updateAddresses() {
            fetchAddresses();
            saveHeldAddresses();
        }

        void AccountManager::updateMessages() {
            fetchMessages();
            updateFetchData();
            saveHeldMessages();
            saveFetchData();
        }

        string AccountManager::getJson() const {
            return as_string(json);
        }

        string AccountManager::getTableName(bool sent) const {
            if (sent) return SENT_TABLE_NAME;
            else return RECEIVED_TABLE_NAME;
        }

        bool AccountManager::isWhitespace(uint32_t code) {
            //TODO: consult https://www.cs.tut.fi/~jkorpela/chars/spaces.html and http://www.fileformat.info/info/unicode/category/Zs/list.htm
            //because I'm suddenly easy about the data wikipedia gave me. Should've seen that coming I guess
            if (code >= 9 && code <= 13) return true;
            else if (code >= 8192 && code <= 8202) return true;
            else
                switch (code) {
                    case 32: return true;
                    case 133: return true;
                    case 160: return true;
                    case 5760: return true;
                    case 8232: return true;
                    case 8233: return true;
                    case 8239: return true;
                    case 8287: return true;
                    case 12288: return true; //This is our Japanese space, even though it's not on the unicode wiki
                    default: return false;
                }
        }

        void AccountManager::cleanWhitespace(string &s) {
            //http://en.wikipedia.org/wiki/Whitespace_character
            //TODO: this copies the string into our result array, and then again in the string constructor, which is less than ideal. also the vector stuff is slightly sketch

            //const char result[s.length()] = {0}; //this won't compile on android http://stackoverflow.com/questions/15013077/arrayn-vs-array10-initializing-array-with-variable-vs-real-number
            vector<char> buffer(s.size()); //So we get memory with a vector, which is fortunately gauranteed to be contiguous
            const char* result = &buffer[0];  //and use a pointer to its head, instead of an array
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
        void AccountManager::holdMessage(bool sent, long int id, time_t date, const string& address, bool media, string msg) {
            //Must be UTF8
            string::iterator utf8End = utf8::find_invalid(msg.begin(), msg.end());
            if (utf8End != msg.end()) {
                logger::warning("Message body with invalid formatting detected.");
                //TODO: log the valid/invalid portions separately. also potentially consider discarding this, because in many cases it's likely to be entirely unusable
                string temp;
                utf8::replace_invalid(msg.begin(), msg.end(), std::back_inserter(temp));
                msg = temp;
            }
            cleanWhitespace(msg);
            size_t length = utf8::distance(msg.begin(), msg.end());

            heldMessages.emplace_back(Message(type, sent, id, date, address, media, length, msg));
        }

        void AccountManager::holdMessageFailure(bool sent, long int id, const string& address) {
            heldMessages.emplace_back(Message::createFailure(type, sent, id, address));
        }

        //TODO: untested since minor refactor. should be identical though
        void AccountManager::saveHeldAddresses() {
            worker::recomputeAddressTable(countedAddresses, addressNames, type);
            countedAddresses.clear();
            addressNames.clear();
        }

        void AccountManager::countAddress(const string &address){
            countedAddresses[address]++;
            //unordered_map [] operator creats the value with default (0 for int, empty str for str) if it doesn't exist
        }

        void AccountManager::setCountedAddressName(const string &address, const string &name){
            addressNames[address] = name;
        }

        //TODO: test this after changes
        shared_ptr<vector<Address>> AccountManager::contactAddresses() const {
            shared_ptr<vector<Address>> ret = make_shared<vector<Address>>();
            auto contacts = reader::contactList();
            for (auto it = contacts->begin(); it != contacts->end(); ++it){
                if (it->hasType(type)) {
                    for (int i = 0; i < it->getAddresses().size(); ++i){
                        if (it->getAddresses()[i].type == type) ret->push_back(it->getAddresses()[i]);
                    }
                }
            }
            return ret;
        }


        //AccountManagers are pretty lightweight so they don't need to be on the heap for memory reasons, but we have to use pointers for polymorphism
        AccountManager* AccountManager::buildManager(string accNm, favor::MessageType typ, string detailsJson) {
            switch (typ) {
                #ifdef FAVOR_EMAIL_MANAGER
                case TYPE_EMAIL:
                    return new EmailManager(accNm, detailsJson);
                #endif
                #ifdef ANDROID
                case TYPE_ANDROIDTEXT:
                    return new AndroidTextManager(accNm, detailsJson);
                #endif
                #ifdef FAVOR_SKYPE_MANAGER
                case TYPE_SKYPE:
                    return new SkypeManager(accNm, detailsJson);
                #endif
                #ifdef FAVOR_LINE_MANAGER
                case TYPE_LINE:
                    return new LineManager(accNm, detailsJson);
                #endif
                default:
                    logger::error("Attempt to initialize manager for unsupported type " + as_string(typ));
                    assert(false);
            }

        }

    }
}