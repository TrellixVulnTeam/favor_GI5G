#include "favor.h"
#include "reader.h"
#include "gtest/gtest.h"
#include "testdata.h"

using namespace std;
using namespace favor;

class DatabaseTest : public ::testing::Test {
    protected:

        //TODO: we should also define some constants for types of bad data, like improperly associated addresses and stuff

        const string contactSeed =
                //Remember that if these type numbers look weird it's because they're using the type flag constants, not the generic type constants
                "INSERT INTO" CONTACT_TABLE "VALUES (1, \"EmailTest1\", 1);" //Email
                "INSERT INTO" CONTACT_TABLE "VALUES (2, \"LineTest2\", 4);" //Line
                "INSERT INTO" CONTACT_TABLE "VALUES (3, \"LineEmailTest3\", 5);" //Line + Email
                "INSERT INTO" CONTACT_TABLE "VALUES (4, \"EmailTest4\", 1);"
                "INSERT INTO" CONTACT_TABLE "VALUES (5, \"EmailTest5\", 1);";

        const string addressSeed =
                "INSERT INTO addresses_" NAME_EMAIL " VALUES(\"test1@test.com\", 1, NULL);"
                "INSERT INTO addresses_" NAME_EMAIL " VALUES(\"test2@test.com\", 2, NULL);"
                "INSERT INTO addresses_" NAME_EMAIL " VALUES(\"test3@test.com\", 1, 1);" //Bound to EmailTeset1
                "INSERT INTO addresses_" NAME_EMAIL " VALUES(\"test4@test.com\", 2, 3);" //Bound to LineEmailTest3
                "INSERT INTO ADDRESSES_" NAME_LINE "VALUES (\"Test1\", 2, 2);" //Bound to LineTest2
                "INSERT INTO ADDRESSES_" NAME_LINE "VALUES (\"Test2\", 1, 3);"; //Bound to LineEmailTest3

        const string accountSeed =
                "INSERT INTO accounts VALUES (\"account1@test.com\", 0, \"{}\");" //0 is email
                "INSERT INTO accounts VALUES (\"account2@test.com\", 0, \"{}\");"
                "INSERT INTO accounts VALUES (\"account3\", 2, \"{}\");" //2 for Line
                "CREATE TABLE \"account1@test.com_" NAME_EMAIL "_sent\"" SENT_TABLE_SCHEMA ";"
                "CREATE TABLE \"account2@test.com_" NAME_EMAIL "_sent\"" SENT_TABLE_SCHEMA ";"
                "CREATE TABLE \"account3_" NAME_LINE "_sent\"" SENT_TABLE_SCHEMA ";"
                "CREATE TABLE \"account1@test.com_" NAME_EMAIL "_received\"" RECEIVED_TABLE_SCHEMA ";"
                "CREATE TABLE \"account2@test.com_" NAME_EMAIL "_received\"" RECEIVED_TABLE_SCHEMA ";"
                "CREATE TABLE \"account3_" NAME_LINE "_received\"" RECEIVED_TABLE_SCHEMA ";";

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
            favor::dbPath = ":memory";
            favor::initialize();
            worker::buildDatabase();
            populateDb();
        }

        virtual void TearDown() override {
            favor::cleanup();
        }
};

class ReaderDatabaseTest : protected DatabaseTest {};

