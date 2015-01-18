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
            string body("Testing");
            Message m1(TYPE_EMAIL, true, 1, 3500, "test@test.com", 0, 1, body);
            Account1->holdMessage(m1.sent, m1.id, m1.date, m1.address, m1.media(), m1.body()+as_string(m1.id)); //yes
            Account1->holdMessage(m1.sent, m1.id, m1.date, m1.address, m1.media(), m1.body()+as_string(m1.id)); //no
            Account1->holdMessage(m1.sent, m1.id, m1.date, m1.address, m1.media(), m1.body()+as_string(m1.id)); //no
            Account1->holdMessage(m1.sent, m1.id, m1.date, m1.address, m1.media(), m1.body()+as_string(m1.id)); //no

            long result = Account1->saveHeldMessages();
            ASSERT_EQ(result, 1);

            Account1->holdMessage(m1.sent, m1.id, m1.date, m1.address, m1.media(), m1.body()+as_string(m1.id)); //no
            Account1->holdMessage(m1.sent, 2, m1.date, m1.address, m1.media(), m1.body()+as_string(2)); //yes
            Account1->holdMessage(m1.sent, 3, m1.date, m1.address, m1.media(), m1.body()+as_string(3)); //yes
            Account1->holdMessage(m1.sent, 4, m1.date, m1.address, m1.media(), m1.body()+as_string(4)); //yes

            result = Account1->saveHeldMessages();
            ASSERT_EQ(result, 3);

            //all yes
            Account1->holdMessage(m1.sent, 5, m1.date, m1.address, m1.media(), m1.body()+as_string(5));
            Account1->holdMessage(m1.sent, 6, m1.date, m1.address, m1.media(), m1.body()+as_string(6));
            Account1->holdMessage(m1.sent, 7, m1.date, m1.address, m1.media(), m1.body()+as_string(7));
            Account1->holdMessage(m1.sent, 8, m1.date, m1.address, m1.media(), m1.body()+as_string(8));

            result = Account1->saveHeldMessages();
            ASSERT_EQ(result, 4);

            //all no
            Account1->holdMessage(m1.sent, 3, m1.date, m1.address, m1.media(), m1.body());
            Account1->holdMessage(m1.sent, 3, m1.date, m1.address, m1.media(), m1.body());
            Account1->holdMessage(m1.sent, 3, m1.date, m1.address, m1.media(), m1.body());
            Account1->holdMessage(m1.sent, 3, m1.date, m1.address, m1.media(), m1.body());

            std::vector<int> ids = {1,2,3,4,5,6,7,8};

            result = Account1->saveHeldMessages();
            ASSERT_EQ(result, 0);

            Contact c(1, "testCon", Address(m1.address, 1, 1, m1.type));
            auto messages = reader::queryContact(Account1, c, KEY_ALL, -1,-1, m1.sent);
            ASSERT_EQ(messages->size(), 8);

            for (int i = 1; i < ids.size()+1; ++i){
                ASSERT_NE(std::find(messages->begin(), messages->end(), Message(m1.type, m1.sent, i, m1.date, m1.address, m1.media(), body.length()+1, m1.body()+as_string(i))), messages->end());
            }
        }
    }
}
