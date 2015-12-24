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



#ifndef favor_accountmanager_include
#define favor_accountmanager_include

#ifdef DEBUG
#include "gtest/gtest_prod.h"
#endif

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
            std::set<string> managedAddresses;
            static const char* managedAddrListName;

        private:
            std::vector<favor::Message> heldMessages;
            std::unordered_map<std::string, int> countedAddresses; //Address : count
            std::unordered_map<std::string, std::string> addressNames; //Address : suggestedName

            long saveHeldMessages();

            void saveHeldAddresses();

            bool saveMessage(const Message& m, sqlite3_stmt* stmt);

            static bool isWhitespace(uint32_t code);

            static void cleanWhitespace(string &s);

            void recordFailure(bool value);
            int previousFailures();

        protected:
            AccountManager(string accNm, MessageType typ, string detailsJson);

            NONCOPYABLE(AccountManager)
            NONMOVEABLE(AccountManager)

            void truncateSentTable();

            void truncateReceivedTable();

            void holdMessage(bool sent, long int id, time_t date, const string& address, bool media, string msg);

            void holdMessageFailure(bool sent, long int id, const string& address);

            void countAddress(const string &address);

            void setAddressCount(const string& address, int count);

            void setCountedAddressName(const string &address, const string &name);

            void updateManagedAddresses();



            string stripXML(const pugi::xml_document &doc);

            virtual void fetchMessages() = 0;

            virtual void fetchAddresses() = 0;

            void saveJson(); //Save the JSON we have
            virtual void updateJson() = 0; //Update the JSON to reflect new variables in the AccountManager
            virtual void consultJson(bool initial = false) = 0; //Update the variables in the AccountManager to reflect the JSON

        public:
            ~AccountManager();

            bool operator==(const AccountManager& rhs) const;

            virtual bool addressValid(const string& address);

            //Database
            static AccountManager* addAccount(string name, MessageType type, string detailsJson);

            void destroy();

            void buildTables();

            void destroyTables();

            void truncateTables();

            void indexTables();

            void deindexTables();

            //Getters

            string getJson() const;

            string getTableName(bool sent) const;

            shared_ptr<vector<Address>> contactAddresses() const;

            //Work
            void updateMessages();

            void updateAddresses();

            //Static methods
            static AccountManager* buildManager(string accNm, MessageType typ, string detailsJson);

            //Friend methods
            #ifdef ANDROID
            friend void ::favor::jni::_saveMessages(JNIEnv* env, jobject callingObj, jint type, jstring name, jbooleanArray sent, jlongArray ids, jlongArray dates, jobjectArray addresses, jbooleanArray media, jobjectArray bodies);
            #endif

            //TESTS
            #ifdef DEBUG
                FRIEND_TEST(AccountManagerCore, SaveMessage);
            #endif
        };

    }
}
#endif