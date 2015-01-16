#include "favor.h"
#include "reader.h"
#include "address.h"
#include "worker.h"
#include "accountmanager.h"
#include "gtest/gtest.h"
#include "testdata.h"
#include "testing.h"

using namespace std;
using namespace favor;


namespace favor{
    namespace worker{
        //These have to be in namespace for FRIEND_TEST to work
        class AccountManagerCore : public DatabaseTest {
            void populateDb(){
                string sql = "BEGIN IMMEDIATE TRANSACTION;";
                sql += accountSeed;
                sql += "COMMIT TRANSACTION;";

                worker::exec(sql);
            }

            virtual void SetUp() override {
                DatabaseTest::SetUp();
                populateDb();
                reader::refreshAll(); //These values are expected to be correct in other methods, so it tests the refresh methods
            }
        };

        TEST_F(AccountManagerCore, HoldMessage){
            //TODO:
        }

        TEST_F(AccountManagerCore, SaveMessage){
            AccountManager* Account1 = AccountManager::buildManager ACC_account1_at_test_dot_com_ARGS;
            Message m1(TYPE_EMAIL, true, 1, 3500, "test@test.com", 0, 1, "Testing");
            Account1->holdMessage(m1.sent, m1.id, m1.date, m1.address, m1.media(), m1.body());
            Account1->holdMessage(m1.sent, m1.id, m1.date, m1.address, m1.media(), m1.body());
            Account1->holdMessage(m1.sent, m1.id, m1.date, m1.address, m1.media(), m1.body());
            Account1->holdMessage(m1.sent, m1.id, m1.date, m1.address, m1.media(), m1.body());

            long result = Account1->saveHeldMessages();
            ASSERT_EQ(result, 1);

            Account1->holdMessage(m1.sent, m1.id, m1.date, m1.address, m1.media(), m1.body());
            Account1->holdMessage(m1.sent, 2, m1.date, m1.address, m1.media(), m1.body());
            Account1->holdMessage(m1.sent, 3, m1.date, m1.address, m1.media(), m1.body());
            Account1->holdMessage(m1.sent, 4, m1.date, m1.address, m1.media(), m1.body());

            result = Account1->saveHeldMessages();
            ASSERT_EQ(result, 3);

            Account1->holdMessage(m1.sent, 5, m1.date, m1.address, m1.media(), m1.body());
            Account1->holdMessage(m1.sent, 6, m1.date, m1.address, m1.media(), m1.body());
            Account1->holdMessage(m1.sent, 7, m1.date, m1.address, m1.media(), m1.body());
            Account1->holdMessage(m1.sent, 8, m1.date, m1.address, m1.media(), m1.body());

            result = Account1->saveHeldMessages();
            ASSERT_EQ(result, 4);

            Account1->holdMessage(m1.sent, 3, m1.date, m1.address, m1.media(), m1.body());
            Account1->holdMessage(m1.sent, 3, m1.date, m1.address, m1.media(), m1.body());
            Account1->holdMessage(m1.sent, 3, m1.date, m1.address, m1.media(), m1.body());
            Account1->holdMessage(m1.sent, 3, m1.date, m1.address, m1.media(), m1.body());

            result = Account1->saveHeldMessages();
            ASSERT_EQ(result, 0);

            //TODO: check that the messages were actually properly written to the database

        }
    }
}
