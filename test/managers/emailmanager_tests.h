#include "favor.h"
#include "accountmanager.h"
#include "gtest/gtest.h"
#include "emailmanager.h"

using namespace std;
using namespace favor;

/*
 * Some of these tests use an actual account and hit the wire because it's just the easiest way to run them.
 */
#define EMAIL_LOGIN ""
#define EMAIL_PASSWORD ""
#define JP_EMAIL ""


/*
Address
 */


TEST(EmailManager, General){
    initialize();
    worker::buildDatabase();
    //reader::refreshAll();

    AccountManager::addAccount(EMAIL_LOGIN, TYPE_EMAIL, "{\"password\":\"" EMAIL_PASSWORD "\"}");
    worker::createContactWithAddress(JP_EMAIL, TYPE_EMAIL, "JP_TEST");
    logger::info("-----------------------------------------------------------------");
    logger::info(favor::as_string(favor::reader::accountList()->front()->type));
    logger::info(favor::reader::accountList()->front()->accountName);
    reader::accountList()->front()->updateMessages();
}

TEST(EmailManager, CharsetConversions){

}