#include "favor.h"
#include "accountmanager.h"
#include "gtest/gtest.h"
#include "skypemanager.h"
#include "skype_testdata.h"


namespace favor {

class SkypeManagerTest : public DatabaseTest {
protected:

    void populateDb(){
        string sql = "BEGIN IMMEDIATE TRANSACTION;";
        sql += SKYPE_SQLITE_SCHEMA;
        sql += "COMMIT TRANSACTION;";


        sqlite3 *db;
        sqlv(sqlite3_open(SKYPE_MEMORY_DB, &db));
        sqlv(sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL));
    }


    virtual void SetUp() override {
        DatabaseTest::SetUp();
        populateDb();
        reader::refreshAll(); //These values are expected to be correct in other methods, so it tests the refresh methods
    }

    virtual void TearDown() override {
        DatabaseTest::TearDown();
    }
};


TEST_F(SkypeManagerTest, General){

    AccountManager::addAccount(SKYPE_ACCOUNT_NAME, TYPE_SKYPE, "{\"skypeDatabaseLocation\":\"" SKYPE_MEMORY_DB "\"}");
    worker::createContactWithAddress(SKYPE_CONTACT_1,TYPE_SKYPE, "SKYPE_TEST");
    reader::accountList()->front()->updateMessages();
    DLOG("Test sum:"+as_string(reader::sum(reader::accountList()->front(),reader::contactList()->front(), KEY_CHARCOUNT, -1, -1, true)));
    worker::createContactWithAddress(SKYPE_CONTACT_2, TYPE_SKYPE, "SKYPE_TEST2");
    reader::accountList()->front()->updateMessages();
    DLOG("Test sum:"+as_string(reader::sum(reader::accountList()->front(),reader::contactList()->front(), KEY_CHARCOUNT, -1, -1, true)));

    reader::accountList()->front()->updateAddresses();

    //worker::createContactWithAddress("GIGGLE", TYPE_LINE, "GIGGLE_WIGGLE");

    worker::backupDatabase();


}


}