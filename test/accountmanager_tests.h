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
        class AccountManagerCore : public DatabaseTest {};

        TEST_F(AccountManagerCore, SaveMessage){
            AccountManager* Account1 = AccountManager::buildManager ACC_account1_at_test_dot_com_ARGS;
            Account1->saveMessage(Message(TYPE_EMAIL, true, 2, 3500, "test@test.com", 0, 1), NULL);
        }
    }
}
