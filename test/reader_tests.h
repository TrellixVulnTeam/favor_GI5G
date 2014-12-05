#include "favor.h"
#include "reader.h"
#include "address.h"
#include "accountmanager.h"
#include "gtest/gtest.h"
#include "testdata.h"

using namespace std;
using namespace favor;

class DatabaseTest : public ::testing::Test {
    protected:

        //TODO: we should also define some constants for types of bad data, like improperly associated addresses and stuff

        const string contactSeed = CONTACT_TEST_DATA;
        const string addressSeed = ADDRESS_TEST_DATA;

        const string accountSeed = ACCOUNT_TEST_DATA;

        const string messagesSeed = MESSAGE_TEST_DATA;

        void populateDb(){
            string sql = "BEGIN IMMEDIATE TRANSACTION;";
            sql += contactSeed;
            sql += addressSeed;
            sql += accountSeed;
            sql += messagesSeed;
            sql += "COMMIT TRANSACTION;";

            worker::exec(sql);
        }

        virtual void SetUp() override {
            ASSERT_EQ(sqlite3_config(SQLITE_CONFIG_URI,1), SQLITE_OK);
            favor::dbName = "file::memory:?cache=shared";
            favor::dbPath = "";
            favor::initialize();
            worker::buildDatabase();
            populateDb();
            reader::refreshAll(); //These values are expected to be correct in other methods, so it tests the refresh methods
        }

        virtual void TearDown() override {
            favor::cleanup();
        }
};

class ReaderDatabase : public DatabaseTest {};

TEST_F(ReaderDatabase, SQLiteSum){
    Contact EmailTest1 CONTACT_EmailTest1_ARGS;
    AccountManager* Account1 = AccountManager::buildManager ACC_account1_at_test_dot_com_ARGS;

    //Also tests sent/true distinction
    long result = reader::sum(Account1, EmailTest1, KEY_CHARCOUNT, -1, -1, true);
    ASSERT_EQ(ACCOUNT1_AT_TEST_DOT_COM_TEST3_AT_TEST_DOT_COM_CHARCOUNT_SENT, result);

    result = reader::sumAll(Account1, KEY_CHARCOUNT, -1, -1, false);
    ASSERT_EQ(ACCOUNT1_AT_TEST_DOT_COM_OVERALL_CHARCOUNT_RECEIVED, result);

    delete Account1;
}


TEST_F(ReaderDatabase, SQLiteAverage){
    Contact LineEmailTest3 CONTACT_LineEmailTest3_ARGS;
    AccountManager* Account2 = AccountManager::buildManager ACC_account2_at_test_dot_com_ARGS;
    AccountManager* Account3 = AccountManager::buildManager ACC_account3_ARGS; //Line

    //Also tests contact-address mapping separation across types
    double result = reader::average(Account2, LineEmailTest3, KEY_CHARCOUNT, -1, -1, true);
    //test4@test.com is the address bound to LineEmailTest
    logger::info(as_string(result)+"=~="+as_string(ACCOUNT2_AT_TEST_DOT_COM_TEST4_AT_TEST_DOT_COM_CHARCOUNT_SENT / ACCOUNT2_AT_TEST_DOT_COM_TEST4_AT_TEST_DOT_COM_MSGCOUNT_SENT));
    ASSERT_EQ(ACCOUNT2_AT_TEST_DOT_COM_TEST4_AT_TEST_DOT_COM_CHARCOUNT_SENT / ACCOUNT2_AT_TEST_DOT_COM_TEST4_AT_TEST_DOT_COM_MSGCOUNT_SENT, (long)result);

    //test2 is another address (line this time) boudn to LineEmailTest
    result = reader::average(Account3, LineEmailTest3, KEY_CHARCOUNT, -1, -1, true);
    logger::info(as_string(result)+"=~="+as_string(ACCOUNT3_TEST2_CHARCOUNT_SENT / ACCOUNT3_TEST2_MSGCOUNT_SENT));
    ASSERT_EQ(ACCOUNT3_TEST2_CHARCOUNT_SENT / ACCOUNT3_TEST2_MSGCOUNT_SENT, (long)result);

    delete Account2;
    delete Account3;
}

TEST_F(ReaderDatabase, SQliteCount){
    Contact LineEmailTest3 CONTACT_LineEmailTest3_ARGS;
    AccountManager* Account2 = AccountManager::buildManager ACC_account2_at_test_dot_com_ARGS;

    //Also tests dates on reader selections for the basic SQL queries
    long result = reader::count(Account2, LineEmailTest3, 0, ACCOUNT2_AT_TEST_DOT_COM_TEST4_AT_TEST_DOT_COM_SENT_MIDDATE, true);
    //test4@test.com is the address bound to LineEmailTest
    ASSERT_EQ((ACCOUNT2_AT_TEST_DOT_COM_TEST1_AT_TEST_DOT_COM_MSGCOUNT_SENT)/2 + 1, result); //>= and <= will pick up the middle term

    //Should be identical
    result = reader::count(Account2, LineEmailTest3, -1, ACCOUNT2_AT_TEST_DOT_COM_TEST4_AT_TEST_DOT_COM_SENT_MIDDATE, true);
    ASSERT_EQ((ACCOUNT2_AT_TEST_DOT_COM_TEST1_AT_TEST_DOT_COM_MSGCOUNT_SENT)/2 + 1, result);



    result = reader::count(Account2, LineEmailTest3, ACCOUNT2_AT_TEST_DOT_COM_TEST4_AT_TEST_DOT_COM_SENT_MIDDATE, -1, true);
    ASSERT_EQ((ACCOUNT2_AT_TEST_DOT_COM_TEST1_AT_TEST_DOT_COM_MSGCOUNT_SENT)/2 + 1, result);

    result = reader::count(Account2, LineEmailTest3, ACCOUNT2_AT_TEST_DOT_COM_TEST4_AT_TEST_DOT_COM_SENT_MIDDATE, ACCOUNT2_AT_TEST_DOT_COM_TEST4_AT_TEST_DOT_COM_SENT_MAXDATE, true);
    //Again should be identical because the <= will pick up the last term
    ASSERT_EQ((ACCOUNT2_AT_TEST_DOT_COM_TEST1_AT_TEST_DOT_COM_MSGCOUNT_SENT)/2 + 1, result);

    long result_all = reader::count(Account2, LineEmailTest3, 0, -1, true);
    ASSERT_EQ((result*2)-1, result_all);

    delete Account2;
}

TEST_F(ReaderDatabase, AccountList){
    auto result = reader::accountList();
    std::list<AccountManager*> definedAccounts;
    definedAccounts.push_back(AccountManager::buildManager ACC_account1_at_test_dot_com_ARGS);
    definedAccounts.push_back(AccountManager::buildManager ACC_account2_at_test_dot_com_ARGS);
    definedAccounts.push_back(AccountManager::buildManager ACC_account3_ARGS);

    ASSERT_EQ(definedAccounts.size(), result->size());
    for (auto it = result->begin(); it != result->end(); ++it){
        //Remove them from defined contacts if we find a match
        for (auto inner_it = definedAccounts.begin(); inner_it != definedAccounts.end(); ++inner_it){
            if (*(*it) == *(*inner_it)){
                definedAccounts.remove(*inner_it);
                inner_it = definedAccounts.end();
            }
        }
    }
    ASSERT_EQ(0, definedAccounts.size()); //There should be nothing left, every one should match

}


TEST_F(ReaderDatabase, ContactList){
    auto result = reader::contactList();
    std::list<Contact> definedContacts;
    definedContacts.push_back(Contact CONTACT_EmailTest1_ARGS);
    definedContacts.push_back(Contact CONTACT_LineTest2_ARGS);
    definedContacts.push_back(Contact CONTACT_LineEmailTest3_ARGS);

    ASSERT_EQ(definedContacts.size(), result->size());
    for (auto it = result->begin(); it != result->end(); ++it){
        for (auto inner_it = definedContacts.begin(); inner_it != definedContacts.end(); ++inner_it){
            //Remove them from defined contacts if we find a match
            if (*it == *inner_it){
                definedContacts.remove(*inner_it);
                inner_it = definedContacts.end();
            }
        }
    }
    ASSERT_EQ(0, definedContacts.size()); //There should be nothing left, every one should match
}

TEST_F(ReaderDatabase, Addresses){
    auto emailResult = reader::addresses(TYPE_EMAIL, false);
    auto lineResult = reader::addresses(TYPE_LINE, false);
    auto emailLineResult = reader::addresses(MessageTypeFlags[TYPE_EMAIL] | MessageTypeFlags[TYPE_LINE], false);

    ASSERT_EQ(emailResult->size()+lineResult->size(), emailLineResult->size());
    //TODO: finish this method. also, we need some contact irrelevant addresses to exist so that we can test the contact relevant flag
}

TEST_F(ReaderDatabase, AddressExists){

}

TEST_F(ReaderDatabase, QueryContact){

}

TEST_F(ReaderDatabase, QueryConversation){

}

TEST_F(ReaderDatabase, QueryAll){

}
