#include "favor.h"
#include "accountmanager.h"
#include "gtest/gtest.h"
#include "emailmanager.h"
#include "../test/testing.h"

using namespace std;
using namespace favor;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;


namespace favor {

    class MockEmailManager : public EmailManager {

    public:
        MockEmailManager(string accNm, string detailsJson) : EmailManager(accNm, detailsJson) {}

        MOCK_METHOD2(parseMessage, void(bool sent, favor::shared_ptr<vmime::net::message> m));

        MOCK_METHOD1(findSentRecFolder, SentRec<std::shared_ptr<vmime::net::folder>>(favor::shared_ptr<vmime::net::store> st));

        MOCK_METHOD3(fetchFromFolder, void(favor::shared_ptr<vmime::net::folder> folder, shared_ptr<const vector<Address>> addresses, bool catchUp));

        MOCK_CONST_METHOD0(contactAddresses, shared_ptr<vector<Address>>());

        SentRec<std::shared_ptr<vmime::net::folder>> findSentRecFolderConcrete(favor::shared_ptr<vmime::net::store> st){
            return EmailManager::findSentRecFolder(st);
        }

        shared_ptr<vector<Address>> contactAddressesConcerete(){
            return AccountManager::contactAddresses();
        }

        ~MockEmailManager(){};
    };

    class EmailManagerTest : public DatabaseTest {
    protected:

        MockEmailManager* manager;

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

        void fetchMessages() {
            manager->fetchMessages();
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
            manager = new MockEmailManager(ACC_account1_at_test_dot_com_NAME,  "{\"password\":\"password\", \"url\":\"imap://example.url:0\"}");
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


    TEST_F(EmailManagerTest, FetchMessages){

        shared_ptr<vector<Address>> addresses = make_shared<vector<Address>>();
        addresses->push_back(Address ADDR_test1_at_test_dot_com_ARGS);
        addresses->push_back(Address ADDR_test3_at_test_dot_com_ARGS);

        int addressesSize = addresses->size();

        for (int i = 0; i < addressesSize; ++i){
            getManagedAddresses().insert(addresses->at(i).addr);
        }

        ASSERT_EQ(getManagedAddresses().size(), addressesSize);

        EXPECT_CALL(getMockEmailManager(), contactAddresses()).Times(1).WillOnce(Return(addresses));

        EXPECT_CALL(getMockEmailManager(), findSentRecFolder(_)).Times(1)
                .WillOnce(Invoke(&getMockEmailManager(), &MockEmailManager::findSentRecFolderConcrete));

        EXPECT_CALL(getMockEmailManager(), fetchFromFolder(_,_,false)).Times(2);
        fetchMessages();

        ASSERT_EQ(getManagedAddresses().size(), addressesSize); //Ensure no new addresses have made their way in

    }

    TEST_F(EmailManagerTest, FetchMessagesWithNewAddresses){
        shared_ptr<vector<Address>> addresses = make_shared<vector<Address>>();
        addresses->push_back(Address ADDR_test1_at_test_dot_com_ARGS);
        addresses->push_back(Address ADDR_test3_at_test_dot_com_ARGS);

        int addressesSize = addresses->size();

        for (int i = 0; i < addressesSize; ++i){
            getManagedAddresses().insert(addresses->at(i).addr);
        }

        ASSERT_EQ(getManagedAddresses().size(), addressesSize);

        EXPECT_CALL(getMockEmailManager(), contactAddresses()).Times(1)
                .WillOnce(Invoke(&getMockEmailManager(), &MockEmailManager::contactAddressesConcerete));

        EXPECT_CALL(getMockEmailManager(), findSentRecFolder(_)).Times(1)
                .WillOnce(Invoke(&getMockEmailManager(), &MockEmailManager::findSentRecFolderConcrete));

        EXPECT_CALL(getMockEmailManager(), fetchFromFolder(_,_,true)).Times(2);
        EXPECT_CALL(getMockEmailManager(), fetchFromFolder(_,_,false)).Times(2);
        fetchMessages();

        ASSERT_NE(getManagedAddresses().size(), addressesSize); //Ensure new addresses have made their way in
    }

    TEST_F(EmailManagerTest, fetchFromFolder){
//        EXPECT_CALL(getMockEmailManager(), parseMessage(true, _)).Times(EMAILMANAGER_MAIL_COUNT);
//        EXPECT_CALL(getMockEmailManager(), parseMessage(false, _)).Times(EMAILMANAGER_MAIL_COUNT);
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