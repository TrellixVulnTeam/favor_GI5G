#ifndef favor_accountmanager_include
#define favor_accountmanager_include

#include "favor.h"
#include "message.h"
#include "address.h"
#include "contact.h"

#ifdef ANDROID
#include <jni.h>
namespace favor{
 namespace jni{
 void _saveMessages(JNIEnv* env, jobject callingObj, jint type, jstring name, jbooleanArray sent, jlongArray ids, jlongArray dates, jobjectArray addresses, jbooleanArray media, jobjectArray bodies);
  }
}
#endif

//I agree processor macros as functions are bad, but it's too convenient to just be able to use these variable's names as strings directly, and
//in this case actually ends up with us being less bug prone
#define setJsonLong(name) if (json.HasMember(#name)) json[#name].SetInt64(name);\
    else {\
        rapidjson::Value addrsVal;\
        addrsVal.SetInt64(name);\
        json.AddMember(#name, addrsVal, json.GetAllocator());\
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
            std::vector<favor::Message> heldMessages;
            std::unordered_map<std::string, int> countedAddresses; //Address : count
            std::unordered_map<std::string, std::string> addressNames; //Address : suggestedName

            void saveHeldMessages();

            void saveHeldAddresses();

            void saveMessage(const Message& m, sqlite3_stmt* stmt);

            static bool isWhitespace(uint32_t code);

            static void cleanWhitespace(string &s);

        protected:
            AccountManager(string accNm, MessageType typ, string detailsJson);

            NONCOPYABLE(AccountManager)
            NONMOVEABLE(AccountManager)

            void truncateSentTable();

            void truncateReceivedTable();

            void holdMessage(bool sent, long int id, time_t date, string address, bool media, string msg);

            void countAddress(const string &address);

            void setCountedAddressName(const string &address, const string &name);

            virtual void fetchMessages() = 0;

            virtual void fetchAddresses() = 0;

            virtual void updateFetchData() = 0;

            void saveFetchData();

            shared_ptr<list<Address>> contactAddresses();

        public:
            ~AccountManager();

            //Database
            static AccountManager* addAccount(string name, MessageType type, string detailsJson);

            void destroy();

            void buildTables();

            void destroyTables();

            void truncateTables();

            void indexTables();

            void deindexTables();

            string getJson() const;

            //Work
            void updateMessages();

            void updateAddresses();

            //Static methods
            static AccountManager* buildManager(string accNm, MessageType typ, string detailsJson);

            //Friend methods
            #ifdef ANDROID
            friend void ::favor::jni::_saveMessages(JNIEnv* env, jobject callingObj, jint type, jstring name, jbooleanArray sent, jlongArray ids, jlongArray dates, jobjectArray addresses, jbooleanArray media, jobjectArray bodies);
            #endif
        };

    }
}
#endif