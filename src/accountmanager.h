#ifndef favor_accountmanager_include
#define favor_accountmanager_include

#include "favor.h"
#include "message.h"
#include "contact.h"

//I agree processor macros as functions are bad, but it's too convenient to just be able to use these variable's names as strings directly, and
//in this case actually ends up with us being less bug prone
#define setJsonLong(name) if (json.HasMember(#name)) json[#name].SetInt64(name);\
    else {\
        rapidjson::Value val;\
        val.SetInt64(name);\
        json.AddMember(#name, val, json.GetAllocator());\
    }


#define getJsonLong(name, default) name = json.HasMember(#name) ? json[#name].GetInt64() : default;

namespace favor {
    namespace worker {

        class AccountManager {
        public:
            const MessageType type;
            const string accountName;

        protected:
            rapidjson::Document json;

        private:
            std::vector<favor::message *> heldMessages;
            std::unordered_map<std::string, int> countedContacts; //Address : count
            std::unordered_map<std::string, std::string> contactNames; //Address : suggestedName

            void saveHeldMessages();

            void saveHeldContacts();

            void saveMessage(const message* m);

            static bool isWhitespace(uint32_t code);

            static void cleanWhitespace(string &s);

        protected:
            AccountManager(string accNm, MessageType typ, string detailsJson);

            AccountManager(const AccountManager &that) = delete; //AccountManager shouldn't ever be copied.
            void truncateSentTable();

            void truncateReceivedTable();

            void holdMessage(bool sent, long int id, time_t date, string address, bool media, string msg);

            void countContact(const string& address);

            void setCountedContactName(const string& address, const string& name);

            virtual void fetchMessages() = 0;

            virtual void fetchContacts() = 0;

            virtual void updateFetchData() = 0;

            void saveFetchData();

        public:

            //Database
            static AccountManager* addAccount(string name, MessageType type, string detailsJson);

            void destroy();

            void buildTables();

            void destroyTables();

            void truncateTables();

            void indexTables();

            void deindexTables();

            //Work
            void updateMessages();

            void updateContacts();

            //Static methods
            static AccountManager* buildManager(string accNm, MessageType typ, string detailsJson);
        };

    }
}
#endif