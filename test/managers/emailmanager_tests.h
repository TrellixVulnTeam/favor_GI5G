#include "favor.h"
#include "accountmanager.h"
#include "gtest/gtest.h"
#include "emailmanager.h"
#include "../test/testing.h"

using namespace std;
using namespace favor;
using ::testing::_;


namespace favor {

    class MockEmailManager : public EmailManager {

        MOCK_METHOD2(parseMessage, void(bool sent, favor::shared_ptr<vmime::net::message> m));

        MOCK_METHOD1(findSentRecFolder, SentRec<std::shared_ptr<vmime::net::folder>>(favor::shared_ptr<vmime::net::store> st));


    public:
        MockEmailManager(string accNm, string detailsJson) : EmailManager(accNm, detailsJson) {}
    };

    class EmailManagerTest : public DatabaseTest {
    protected:

        EmailManager* manager;

        MockEmailManager& getMockEmailManager(){
            return *(dynamic_cast<MockEmailManager*>(getManager()));
        }

        std::time_t toTime(const vmime::datetime input) {
            return EmailManager::toTime(input);
        }

        void consultJsonInitial(){
            manager->consultJson(true);
        }

        EmailManager* getManager(){
            return manager;
        }

        string getPassword(){
            return manager->password;
        }

        string getURL(){
            return manager->serverURL;
        }

        set<string>& getManagedAddresses(){
            return manager->managedAddresses;
        }

        void setJson(string json){
            manager->json.Parse(json.c_str());
        }

        void constructNewManager(string username, string json){
            delete manager;
            manager = new MockEmailManager(username, json);
        }

        bool toXml(stringstream &ss){
            return manager->toXML(ss);
        }

        void populateDb(){
            string sql = "BEGIN IMMEDIATE TRANSACTION;";
            sql += contactSeed;
            sql += addressSeed;
            sql += accountSeed;
            sql += "COMMIT TRANSACTION;";

            worker::exec(sql);
        }

        void newManager(){
            constructNewManager(ACC_account1_at_test_dot_com_NAME,  "{\"password\":\"password\", \"url\":\"imap://example.url:0\"}");
        }


        virtual void SetUp() override {
            DatabaseTest::SetUp();
            populateDb();
            reader::refreshAll(); //These values are expected to be correct in other methods, so it tests the refresh methods
            newManager();
        }

        virtual void TearDown() override {
            DatabaseTest::TearDown();
            delete manager;
        }
    };


/*
Address
 */


    TEST_F(EmailManagerTest, UpdateMessages){
        getManagedAddresses().insert("Test Address 1");
        EXPECT_CALL(getMockEmailManager(), findSentRecFolder(_)).times(1);
        getManager()->updateMessages();

        //Methods should be called,
    }

    TEST_F(EmailManagerTest, UpdateMessagesWithNewAddresses){
        getManagedAddresses().insert("Test Address 1");
        getManager()->updateMessages();

        //same as above, but with the extra catch-up calls (catch up beings et to true) for fetchFromFolder
    }

    TEST_F(EmailManagerTest, ConsultJsonWithNoPassword) {
        setJson("{\"url\":\"imap://example.url:0\"}");
        ASSERT_THROW(consultJsonInitial(), badUserDataException);

    }

    TEST_F(EmailManagerTest, ConsultJsonWithBadEmail) {
        ASSERT_THROW(constructNewManager("NO_EMAILING_ALLOWED",  "{\"password\":\"password\", \"url\":\"imap://example.url:0\"}"), badUserDataException);
        newManager(); //This is to avoid segfaults in TearDown() when we delete the manager
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

}