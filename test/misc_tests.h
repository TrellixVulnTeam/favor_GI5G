#include "favor.h"
#include "reader.h"
#include "address.h"
#include "accountmanager.h"
#include "gtest/gtest.h"
#include "testdata.h"

using namespace std;
using namespace favor;


/*
Address
 */

TEST(Address, BelongsToContact){
    Address Test1 ADDR_test1_at_test_dot_com_ARGS; //Belongs to no contacts
    Address Test2 ADDR_test3_at_test_dot_com_ARGS; //Belongs to a contact
    ASSERT_FALSE(Test1.belongsToContact());
    ASSERT_TRUE(Test2.belongsToContact());
}

TEST(Address, Equality){
    Address Test1 ADDR_test1_at_test_dot_com_ARGS;
    Address Test1Dup ADDR_test1_at_test_dot_com_ARGS;
    Address Test2 ADDR_test3_at_test_dot_com_ARGS;
    ASSERT_EQ(Test1, Test1Dup);
    ASSERT_EQ(Test1, Test1);
    ASSERT_NE(Test1, Test2);
}

/*
Message
Message(MessageType t, bool s, long int i, std::time_t d, string a, short m, long cc, const string& b);
 */
TEST(Message, Equality){
    Message m1(TYPE_EMAIL, true, 1, 500, "test@test.com", true, 25, "m1body");
    Message m1dup(TYPE_EMAIL, true, 1, 500, "test@test.com", true, 25, "m1body");
    Message m2(TYPE_EMAIL, false, 1, 300, "test@test.com", true, 35, "m2body");

    ASSERT_EQ(m1, m1dup);
    ASSERT_EQ(m1, m1);
    ASSERT_NE(m1, m2);

}

TEST(Message, Failures){
    Message m1 = Message::createFailure(TYPE_EMAIL, true, 1, "failure@fail.com");
    ASSERT_TRUE(m1.failure());
    ASSERT_FALSE(m1.isBodyKnown());
    ASSERT_TRUE(m1.isSentKonwn());
    ASSERT_FALSE(m1.isCharCountKnown());
    ASSERT_FALSE(m1.isDateKnown());
    ASSERT_TRUE(m1.isIdKnown());
    ASSERT_FALSE(m1.isMediaKnown());

    Message m2(TYPE_EMAIL, true, 1, 500, "test@test.com", true, 25);
    Message m3(TYPE_EMAIL, true, 1, 500, "test@test.com", true, 25, "m1body");
    ASSERT_FALSE(m2.failure());
    ASSERT_FALSE(m3.failure());


}

TEST(Message, IsKnown){
    Message m1(TYPE_EMAIL, true, 1, 500, "test@test.com", true, 25, "m1body");
    ASSERT_TRUE(m1.isBodyKnown());
    ASSERT_TRUE(m1.isSentKonwn());
    ASSERT_TRUE(m1.isCharCountKnown());
    ASSERT_TRUE(m1.isDateKnown());
    ASSERT_TRUE(m1.isIdKnown());
    ASSERT_TRUE(m1.isMediaKnown());

    Message m2(TYPE_EMAIL, true, 1, 500, "test@test.com", true, 25);
    ASSERT_FALSE(m2.isBodyKnown());
    ASSERT_TRUE(m2.isSentKonwn());
    ASSERT_TRUE(m2.isCharCountKnown());
    ASSERT_TRUE(m2.isDateKnown());
    ASSERT_TRUE(m2.isIdKnown());
    ASSERT_TRUE(m2.isMediaKnown());

    Message m3(TYPE_EMAIL, true, 1, Message::UNKNOWN_NUMERIC_VALUE, Message::UNKNOWN_ADDRESS_VALUE, Message::UNKNOWN_NUMERIC_VALUE, Message::UNKNOWN_NUMERIC_VALUE);
    ASSERT_FALSE(m3.isBodyKnown());
    ASSERT_TRUE(m3.isSentKonwn());
    ASSERT_FALSE(m3.isCharCountKnown());
    ASSERT_FALSE(m3.isDateKnown());
    ASSERT_TRUE(m3.isIdKnown());
    ASSERT_FALSE(m3.isMediaKnown());
}

/*
Contact
 */

TEST(Contact, HasType){
    Contact EmailTest CONTACT_EmailTest1_ARGS;
    Contact LineTest CONTACT_LineTest2_ARGS;
    Contact LineEmailTest CONTACT_LineEmailTest3_ARGS;

    ASSERT_FALSE(EmailTest.hasType(TYPE_SKYPE));
    ASSERT_FALSE(LineTest.hasType(TYPE_SKYPE));
    ASSERT_FALSE(LineEmailTest.hasType(TYPE_SKYPE));

    ASSERT_TRUE(EmailTest.hasType(TYPE_EMAIL));
    ASSERT_FALSE(EmailTest.hasType(TYPE_LINE));

    ASSERT_TRUE(LineTest.hasType(TYPE_LINE));
    ASSERT_FALSE(LineTest.hasType(TYPE_EMAIL));

    ASSERT_TRUE(LineEmailTest.hasType(TYPE_LINE));
    ASSERT_TRUE(LineEmailTest.hasType(TYPE_EMAIL));
}

TEST(Contact, AddAddress){
    //TODO: this needs a test mostly because it should update the types of addresses the contacts has, I.E. change what the contact considers relevant types
}