#include "favor.h"
#include "reader.h"
#include "address.h"
#include "accountmanager.h"
#include "gtest/gtest.h"
#include "testdata.h"
#include "testing.h"

using namespace std;
using namespace favor;

class Reader : public DatabaseTest {
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
        DatabaseTest::SetUp();
        populateDb();
        reader::refreshAll(); //These values are expected to be correct in other methods, so it tests the refresh methods
    }



};

TEST_F(Reader, SQLiteSum){
    Contact EmailTest1 CONTACT_EmailTest1_ARGS;
    AccountManager* Account1 = AccountManager::buildManager ACC_account1_at_test_dot_com_ARGS;

    //Also tests sent/true distinction
    long result = reader::sum(Account1, EmailTest1, KEY_CHARCOUNT, -1, -1, true);
    ASSERT_EQ(ACCOUNT1_AT_TEST_DOT_COM_TEST3_AT_TEST_DOT_COM_CHARCOUNT_SENT, result);

    result = reader::sumAll(Account1, KEY_CHARCOUNT, -1, -1, false);
    ASSERT_EQ(ACCOUNT1_AT_TEST_DOT_COM_OVERALL_CHARCOUNT_RECEIVED, result);

    delete Account1;
}


TEST_F(Reader, SQLiteAverage){
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

TEST_F(Reader, SQliteCount){
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

TEST_F(Reader, AccountList){
    auto result = reader::accountList();
    std::list<AccountManager*> definedAccounts;
    definedAccounts.push_back(AccountManager::buildManager ACC_account1_at_test_dot_com_ARGS);
    definedAccounts.push_back(AccountManager::buildManager ACC_account2_at_test_dot_com_ARGS);
    definedAccounts.push_back(AccountManager::buildManager ACC_account3_ARGS);

    ASSERT_EQ(definedAccounts.size(), result->size());
    for (auto it = result->begin(); it != result->end(); ++it){
        //Can't use std::find here because it'll compare pointers, so we remove one when we find a match
        for (auto iit = definedAccounts.begin(); iit != definedAccounts.end(); ++iit){
            if (*(*it) == *(*iit)){
                definedAccounts.remove(*iit);
                iit = definedAccounts.end();
            }
        }
    }
    ASSERT_EQ(0, definedAccounts.size()); //And there should be none left
}


TEST_F(Reader, ContactList){
    auto result = reader::contactList();
    std::list<Contact> definedContacts;
    definedContacts.push_back(Contact CONTACT_EmailTest1_ARGS);
    definedContacts.push_back(Contact CONTACT_LineTest2_ARGS);
    definedContacts.push_back(Contact CONTACT_LineEmailTest3_ARGS);
    definedContacts.push_back(Contact CONTACT_TwoEmailTest4_ARGS);

    ASSERT_EQ(definedContacts.size(), result->size());
    for (auto it = result->begin(); it != result->end(); ++it){
        ASSERT_NE(std::find(definedContacts.begin(), definedContacts.end(), *it), definedContacts.end());
    }
}

TEST_F(Reader, Addresses){
    std::vector<Address> definedAddresses;
    definedAddresses.push_back(Address ADDR_Test1_ARGS);
    definedAddresses.push_back(Address ADDR_Test2_ARGS);
    definedAddresses.push_back(Address ADDR_test3_at_test_dot_com_ARGS);
    definedAddresses.push_back(Address ADDR_test1_at_test_dot_com_ARGS);
    definedAddresses.push_back(Address ADDR_test4_at_test_dot_com_ARGS);
    definedAddresses.push_back(Address ADDR_dubtest1_at_test_dot_com_ARGS);
    definedAddresses.push_back(Address ADDR_dubtest2_at_test_dot_com_ARGS);



    auto emailResult = reader::addresses(TYPE_EMAIL, false);
    auto lineResult = reader::addresses(TYPE_LINE, false);
    auto emailLineResult = reader::addresses(MessageTypeFlags[TYPE_EMAIL] | MessageTypeFlags[TYPE_LINE], false);


    ASSERT_EQ(emailResult->size()+lineResult->size(), emailLineResult->size());
    ASSERT_EQ(definedAddresses.size(), emailLineResult->size());
    for (auto it = emailLineResult->begin(); it != emailLineResult->end(); ++it){
        if (it->type == TYPE_EMAIL){
            ASSERT_NE(std::find(emailResult->begin(), emailResult->end(), *it), emailResult->end());
            ASSERT_EQ(std::find(lineResult->begin(), lineResult->end(), *it), lineResult->end());
        }
        else if (it->type == TYPE_LINE){
            ASSERT_NE(std::find(lineResult->begin(), lineResult->end(), *it), lineResult->end());
            ASSERT_EQ(std::find(emailResult->begin(), emailResult->end(), *it), emailResult->end());
        }
    }

    auto emailLineResultSmaller = reader::addresses(MessageTypeFlags[TYPE_EMAIL] | MessageTypeFlags[TYPE_LINE], true);

    for (auto it = definedAddresses.begin(); it != definedAddresses.end(); ++it){
        if (it->type == TYPE_EMAIL){
            ASSERT_NE(std::find(emailResult->begin(), emailResult->end(), *it), emailResult->end());
        } else if (it->type == TYPE_LINE){
            ASSERT_NE(std::find(lineResult->begin(), lineResult->end(), *it), lineResult->end());
        }
    }
    ASSERT_LT(emailLineResultSmaller->size(), emailLineResult->size());
}

TEST_F(Reader, AddressExists){
    Address addr1 ADDR_test1_at_test_dot_com_ARGS;
    Address addrFake("no@nope.no", 1, -1, TYPE_EMAIL);
    ASSERT_TRUE(reader::addressExists(addr1.addr, addr1.type));
    ASSERT_FALSE(reader::addressExists(addrFake.addr, addrFake.type));
}

TEST_F(Reader, QueryContact){
    Contact EmailTest1 CONTACT_EmailTest1_ARGS;
    Contact DoubleEmailTest4 CONTACT_TwoEmailTest4_ARGS;
    Contact LineTest2 CONTACT_LineTest2_ARGS;
    AccountManager* Account1 = AccountManager::buildManager ACC_account1_at_test_dot_com_ARGS;
    auto result = reader::queryContact(Account1, EmailTest1, KEY_DATE | KEY_CHARCOUNT, -1, -1, false);
    //test3@test.com is the address bound to EmailTest1

    std::vector<long> recDates = ACCOUNT1_AT_TEST_DOT_COM_TEST3_AT_TEST_DOT_COM_RECEIVED_DATELIST_ARG;
    std::vector<long> recCharCounts = ACCOUNT1_AT_TEST_DOT_COM_TEST3_AT_TEST_DOT_COM_RECEIVED_CHARCOUNTLIST_ARG;

    //We have to go backwards in the vector because it's sorted normally, but database output should be descending
    int counter = recDates.size()-1;
    auto bit = result->begin();
    for (auto it = result->begin(); it != result->end(); ++it){
        ASSERT_EQ(recDates[counter], it->date);
        ASSERT_EQ(recCharCounts[counter], it->charCount);
        ASSERT_FALSE(it->isBodyKnown());
        ASSERT_FALSE(it->isAddressKnown());
        ASSERT_GE(bit->date, it->date); //Verify sort order
        bit = it;
        counter--;
    }

    auto halfResult = reader::queryContact(Account1, EmailTest1, KEY_ALL,
            ACCOUNT1_AT_TEST_DOT_COM_TEST3_AT_TEST_DOT_COM_RECEIVED_MIDDATE, -1, false);

    ASSERT_EQ(result->size(), halfResult->size()*2 - 1); //Includes the middle value so we pull one off

    counter = recDates.size()-1;
    bit = halfResult->begin();
    for (auto it = halfResult->begin(); it != halfResult->end(); ++it){
        ASSERT_EQ(recDates[counter], it->date);
        ASSERT_EQ(recCharCounts[counter], it->charCount);
        ASSERT_TRUE(it->isBodyKnown());
        ASSERT_TRUE(it->isAddressKnown());
        ASSERT_TRUE(it->isMediaKnown());
        ASSERT_GE(bit->date, it->date); //Verify sort order
        bit = it;
        counter--;
    }

    auto doubleResult = reader::queryContact(Account1, DoubleEmailTest4, KEY_CHARCOUNT, -1, -1, true);
    ASSERT_EQ(ACCOUNT1_AT_TEST_DOT_COM_DUBTEST1_AT_TEST_DOT_COM_MSGCOUNT_SENT+ACCOUNT1_AT_TEST_DOT_COM_DUBTEST2_AT_TEST_DOT_COM_MSGCOUNT_SENT,
            doubleResult->size());

    bit = halfResult->begin();
    for (auto it = doubleResult->begin(); it != doubleResult->end(); ++it){
        ASSERT_GE(bit->date, it->date); //Verify sort order
        bit = it;
    }

    auto emptyResult = reader::queryContact(Account1, LineTest2, KEY_ID, -1, -1, true);
    ASSERT_EQ(0, emptyResult->size());

    delete Account1;
}

TEST_F(Reader, QueryAll){
    AccountManager* Account1 = AccountManager::buildManager ACC_account1_at_test_dot_com_ARGS;
    AccountManager* Account3 = AccountManager::buildManager ACC_account3_ARGS;

    auto smallerResult = reader::queryAll(Account1, KEY_ALL, ACCOUNT1_AT_TEST_DOT_COM_TEST3_AT_TEST_DOT_COM_RECEIVED_MIDDATE, -1, false);
    auto result = reader::queryAll(Account1, KEY_CHARCOUNT, -1, -1, false);

    ASSERT_LT(smallerResult->size(), result->size()); //Using a somewhat arbitrary date for the smaller result, so establishing relative size is enough
    ASSERT_NE(0, smallerResult->size());

    auto bit = smallerResult->begin();
    for (auto it = smallerResult->begin(); it!=smallerResult->end(); ++it){
        ASSERT_GE(bit->date, it->date);
        bit = it;
    }

    long long sum = 0;
    for (auto it = result->begin(); it!=result->end(); ++it){
        sum += it->charCount;
        ASSERT_FALSE(it->isBodyKnown());
        ASSERT_FALSE(it->isAddressKnown());
        ASSERT_FALSE(it->isDateKnown());
    }
    ASSERT_EQ(ACCOUNT1_AT_TEST_DOT_COM_OVERALL_CHARCOUNT_RECEIVED, sum);
    ASSERT_EQ(ACCOUNT1_AT_TEST_DOT_COM_OVERALL_MSGCOUNT_RECEIVED, result->size());


    auto result2 = reader::queryAll(Account3, KEY_CHARCOUNT, -1, -1, true);
    ASSERT_NE(*result, *result2); //Though this should be a given, really

    sum = 0;
    bit = result2->begin();
    for (auto it = result2->begin(); it!=result2->end(); ++it){
        sum += it->charCount;
        ASSERT_GE(bit->date, it->date);
        bit = it;
    }
    ASSERT_EQ(ACCOUNT3_OVERALL_CHARCOUNT_SENT, sum);
    ASSERT_EQ(ACCOUNT3_OVERALL_MSGCOUNT_SENT, result2->size());

    delete Account1;
    delete Account3;
}

TEST_F(Reader, QueryConversation){
    AccountManager* Account1 = AccountManager::buildManager ACC_account1_at_test_dot_com_ARGS;
    AccountManager* Account3 = AccountManager::buildManager ACC_account3_ARGS;

    Contact EmailTest1 CONTACT_EmailTest1_ARGS;
    Contact DoubleEmailTest4 CONTACT_TwoEmailTest4_ARGS;
    Contact LineTest2 CONTACT_LineTest2_ARGS;

    auto emptyResult = reader::queryConversation(Account1, LineTest2, KEY_ID, -1, -1);
    ASSERT_EQ(0, emptyResult->size());

    auto smallerResult = reader::queryConversation(Account3, LineTest2, KEY_DATE, ACCOUNT3_TEST1_RECEIVED_MIDDATE, -1);
    auto result1 = reader::queryConversation(Account1, EmailTest1, KEY_DATE | KEY_CHARCOUNT, -1, -1);
    ASSERT_LT(smallerResult->size(), result1->size());
    ASSERT_NE(0, smallerResult->size());

    auto bit = smallerResult->begin();
    for (auto it = smallerResult->begin(); it!= smallerResult->end(); ++it){
        ASSERT_GE(bit->date, it->date);
        bit = it;
        ASSERT_FALSE(it->isCharCountKnown());
        ASSERT_FALSE(it->isBodyKnown());
        ASSERT_FALSE(it->isAddressKnown());
        ASSERT_FALSE(it->isMediaKnown());
    }

    //Using dates because they're unique when generated with our test script
    std::vector<long> result1DatesReceived = ACCOUNT1_AT_TEST_DOT_COM_TEST3_AT_TEST_DOT_COM_RECEIVED_DATELIST_ARG;
    std::vector<long> result1DatesSent = ACCOUNT1_AT_TEST_DOT_COM_TEST3_AT_TEST_DOT_COM_SENT_DATELIST_ARG;

    long long sum = 0;
    bit = result1->begin();
    for (auto it = result1->begin(); it != result1->end(); ++it){
        ASSERT_GE(bit->date, it->date);
        ASSERT_FALSE(it->isBodyKnown());
        ASSERT_FALSE(it->isAddressKnown());
        ASSERT_FALSE(it->isMediaKnown());
        if (it->sent) ASSERT_NE(std::find(result1DatesSent.begin(), result1DatesSent.end(), it->date), result1DatesSent.end());
        else ASSERT_NE(std::find(result1DatesReceived.begin(), result1DatesReceived.end(), it->date), result1DatesReceived.end());
        bit = it;
        sum += it->charCount;
    }
    ASSERT_EQ(ACCOUNT1_AT_TEST_DOT_COM_TEST3_AT_TEST_DOT_COM_CHARCOUNT_RECEIVED+ACCOUNT1_AT_TEST_DOT_COM_TEST3_AT_TEST_DOT_COM_CHARCOUNT_SENT, sum);


    long maxDate = std::max(ACCOUNT1_AT_TEST_DOT_COM_DUBTEST1_AT_TEST_DOT_COM_RECEIVED_MAXDATE, ACCOUNT1_AT_TEST_DOT_COM_DUBTEST1_AT_TEST_DOT_COM_SENT_MAXDATE);
    maxDate = std::max(maxDate, (long) ACCOUNT1_AT_TEST_DOT_COM_DUBTEST2_AT_TEST_DOT_COM_RECEIVED_MAXDATE);
    maxDate = std::max(maxDate, (long) ACCOUNT1_AT_TEST_DOT_COM_DUBTEST2_AT_TEST_DOT_COM_SENT_MAXDATE);
    //The chosen date should be >= to all relevant dates here so it shouldn't effect the outcome, we just want to make sure selection setup works properly
    auto doubleResult = reader::queryConversation(Account1, DoubleEmailTest4, KEY_ALL, -1, maxDate);
    bit = doubleResult->begin();
    for (auto it = doubleResult->begin(); it != doubleResult->end(); ++it){
        ASSERT_GE(bit->date, it->date);
        bit = it;
    }
    ASSERT_EQ(ACCOUNT1_AT_TEST_DOT_COM_DUBTEST1_AT_TEST_DOT_COM_MSGCOUNT_RECEIVED+ACCOUNT1_AT_TEST_DOT_COM_DUBTEST1_AT_TEST_DOT_COM_MSGCOUNT_SENT+
            ACCOUNT1_AT_TEST_DOT_COM_DUBTEST2_AT_TEST_DOT_COM_MSGCOUNT_RECEIVED+ACCOUNT1_AT_TEST_DOT_COM_DUBTEST2_AT_TEST_DOT_COM_MSGCOUNT_SENT,
            doubleResult->size());

    delete Account1;
    delete Account3;

}
