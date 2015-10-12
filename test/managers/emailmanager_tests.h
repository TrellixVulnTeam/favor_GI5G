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

namespace favor {

    class mockMessageStructure : public vmime::net::messageStructure {
//        MOCK_METHOD0(getPartCount, size_t());
//        MOCK_METHOD1(getPartAt, mockMessagePart(size_t pos));
    };


    class EmailManagerTest : public ::testing::Test {
    protected:

        EmailManager* manager;

        std::time_t toTime(const vmime::datetime input) {
            return EmailManager::toTime(input);
        }

        void consultJsonInitial(){
            manager->consultJson(true);
        }

        string getPassword(){
            return manager->password;
        }

        string getURL(){
            return manager->serverURL;
        }

        void setJson(string json){
            manager->json.Parse(json.c_str());
        }

        void constructNewManager(string username, string json){
            delete manager;
            manager = new EmailManager(username, json);
        }

        bool toXml(stringstream &ss){
            return manager->toXML(ss);
        }


        virtual void SetUp() override {
            manager =  new EmailManager("emailtest@email.com",  "{\"password\":\"password\", \"url\":\"imap://example.url:0\"}");

        }

        virtual void TearDown() override {
            delete manager;
        }
    };


/*
Address
 */


//TEST(EmailManager, General){
//    //TODO: this is a mess, and should at least inherit from something to use the in-memory database
//    if (EMAIL_PASSWORD == ""){
//        logger::warning("Email test skipped; no login info provided on this run");
//        return;
//    }
//    initialize();
//    worker::buildDatabase();
//    //reader::refreshAll();
//
//    AccountManager::addAccount(EMAIL_LOGIN, TYPE_EMAIL, "{\"password\":\"" EMAIL_PASSWORD "\"}");
//    worker::createContactWithAddress(JP_EMAIL, TYPE_EMAIL, "JP_TEST");
//    worker::createContactWithAddress(ALT_EMAIL, TYPE_EMAIL, "OTHER_TEST");
//    worker::createContactWithAddress("no-reply@accounts.google.com", TYPE_EMAIL, "HTML TEST");
//    logger::info("-----------------------------------------------------------------");
//    logger::info(favor::as_string(favor::reader::accountList()->front()->type));
//    logger::info(favor::reader::accountList()->front()->accountName);
//    reader::accountList()->front()->updateMessages();
//}

    TEST_F(EmailManagerTest, ConsultJsonWithNoPassword) {
        setJson("{\"url\":\"imap://example.url:0\"}");
        ASSERT_THROW(consultJsonInitial(), badUserDataException);

    }

    TEST_F(EmailManagerTest, ConsultJsonWithBadEmail) {
        ASSERT_THROW(constructNewManager("NO_EMAILING_ALLOWED",  "{\"password\":\"password\", \"url\":\"imap://example.url:0\"}"), badUserDataException);
        SetUp(); //just so we don't delete man empty manager pointer in teardown, since the attempt to construct a new one here will except
    }

    TEST_F(EmailManagerTest, ConsultJsonForURl) {
        //No url
        setJson("{\"password\":\"password\"}");
        ASSERT_THROW(consultJsonInitial(), badUserDataException);

        //Bad url
        setJson("{\"password\":\"password\", \"url\":\"NO_EMAILING_ALLOWED\"}");
        ASSERT_THROW(consultJsonInitial(), badUserDataException);

    }

    TEST_F(EmailManagerTest, ConsultJson) {
        string password = "password";
        string url = "imap://example.url";
        string port = ":0";

        //We split up URL and port because vmime handles ports so all we can get back is the url by itself
        constructNewManager("email@emailtest.com",  "{\"password\":\""+password+"\", \"url\":\""+url+port+"\"}");
        ASSERT_EQ(getPassword(), password);
        ASSERT_EQ(getURL(), url);
    }

    TEST_F(EmailManagerTest, AddressValid){
        ASSERT_FALSE(manager->addressValid("NO_EMAILING_ALLOWED"));
        ASSERT_TRUE(manager->addressValid("email@emailtest.com"));
    }


    TEST_F(EmailManagerTest, ToTime) {
        time_t unixTime1 = 1444506541;
        time_t unixTime2 = 747706393;
        vmime::datetime time1(unixTime1, vmime::datetime::TimeZones::GMT);
        vmime::datetime time2(unixTime2, vmime::datetime::TimeZones::GMT);


        ASSERT_EQ(toTime(time1), unixTime1);
        ASSERT_EQ(toTime(time2), unixTime2);

    }

    TEST_F(EmailManagerTest, ToXMLInvalid){
        string content("NOT <ANY> XML <HERE> AT ALL!");
        stringstream ss(content);
        ASSERT_FALSE(toXml(ss));
        ASSERT_EQ(ss.str(), content);
    }

    TEST_F(EmailManagerTest, ToXML){
        string html("<!DOCTYPE html>\n"
                            "<html>\n"
                            "<body>\n"
                            "\n"
                            "<h1>My First Heading</h1>\n"
                            "\n"
                            "<p>My first paragraph.</p>\n"
                            "\n"
                            "</body>\n"
                            "</html>");
        stringstream ss(html);
        ASSERT_TRUE(toXml(ss));
        ASSERT_NE(ss.str(), html);

        //We don't make predictions about what this would actually look like because
        //that would just be testing tidyhtml5. It also doesn't matter as long as it's
        //valid, strippable XML
    }

//TEST(EmailManager, FetchAsddresses){
//    initialize();
//    worker::buildDatabase();
//    //reader::refreshAll();
//
//    AccountManager::addAccount(EMAIL_LOGIN, TYPE_EMAIL, "{\"password\":\"" EMAIL_PASSWORD "\"}");
//    reader::accountList()->front()->updateAddresses();
//}
}