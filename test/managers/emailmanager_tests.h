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
#define ALT_EMAIL ""


/*
Address
 */


TEST(EmailManager, General){
    if (EMAIL_PASSWORD == ""){
        logger::warning("Email test skipped; no login info provided on this run");
        return;
    }
    initialize();
    worker::buildDatabase();
    //reader::refreshAll();

    AccountManager::addAccount(EMAIL_LOGIN, TYPE_EMAIL, "{\"password\":\"" EMAIL_PASSWORD "\"}");
    worker::createContactWithAddress(JP_EMAIL, TYPE_EMAIL, "JP_TEST");
    worker::createContactWithAddress(ALT_EMAIL, TYPE_EMAIL, "OTHER_TEST");
    worker::createContactWithAddress("no-reply@accounts.google.com", TYPE_EMAIL, "HTML TEST");
    logger::info("-----------------------------------------------------------------");
    logger::info(favor::as_string(favor::reader::accountList()->front()->type));
    logger::info(favor::reader::accountList()->front()->accountName);
    reader::accountList()->front()->updateMessages();
}

TEST(EmailManager, CharsetConversions){

}