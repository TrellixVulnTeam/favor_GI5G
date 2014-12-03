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
            //favor::dbPath = ":memory:";
            //We skip Favor's init because it will fail some assertions that we don't need for testing anyway
            worker::initialize();
            reader::initialize();
            worker::buildDatabase();
            populateDb();
        }

        virtual void TearDown() override {
            favor::cleanup();
        }
};

class ReaderDatabase : public DatabaseTest {};


TEST_F(ReaderDatabase, SQLiteSum){
    Contact EmailTest1 CONTACT_EmailTest1_ARGS;
    AccountManager* Account1 = AccountManager::buildManager ACC_account1_at_test_dot_com_ARGS;
    long result = reader::sum(Account1, EmailTest1, KEY_CHARCOUNT, -1, -1, true);
    delete Account1;

    ASSERT_EQ(result, ACCOUNT1_AT_TEST_DOT_COM_TEST3_AT_TEST_DOT_COM_CHARCOUNT_SENT);
}

