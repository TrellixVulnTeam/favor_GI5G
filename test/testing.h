#ifndef DATABASE_TEST
#define DATABASE_TEST

#include "favor.h"
#include "reader.h"
#include "address.h"
#include "worker.h"
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

    virtual void SetUp() override {
        ASSERT_EQ(sqlite3_config(SQLITE_CONFIG_URI,1), SQLITE_OK);
        favor::dbName = "file::memory:?cache=shared";
        favor::dbPath = "";
        favor::initialize();
        worker::buildDatabase();
    }

    virtual void TearDown() override {
        favor::cleanup();
    }
};


//So that Google Test knows how to print favor objects
namespace favor{
    void PrintTo(const Message& m, ::std::ostream* os){
        *os << as_string(m);
    }

    void PrintTo(const Address& a, ::std::ostream* os){
        *os << as_string(a);
    }

    void PrintTo(const Contact& c, ::std::ostream* os){
        *os << as_string(c);
    }

    void PrintTo(const AccountManager& a, ::std::ostream* os){
        *os << as_string(a);
    }

}

#endif