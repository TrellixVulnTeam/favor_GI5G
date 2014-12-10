#include "favor.h"
#include "reader.h"
#include "address.h"
#include "accountmanager.h"
#include "gtest/gtest.h"
#include "testdata.h"
#include "testing.h"

using namespace std;
using namespace favor;


class Worker : public DatabaseTest {};

/*
 We trust some reader methods here, specifically to verify database changes. The testing dependencies created aren't ideal,
 but the alternative is basically copy pasting the exact same code into the tests to read the database, which seems worse.
 */

template<class InputIterator>
bool findContact (InputIterator first, InputIterator last, const Contact& val)
{
        //This is just here so we can compare contacts meaningfully for these tests; address counts or vector orders don't matter
        while (first!=last) {
                bool equal = (val.id == first->id && val.displayName == first->displayName && val.typeFlags() == first->typeFlags());
                //logger::info("Compare "+as_string(val)+" to "+as_string(*first));
                for (auto oit = val.getAddresses().begin(); oit != val.getAddresses().end(); ++oit){
                        bool foundMatch = false;
                        for (auto iit = first->getAddresses().begin(); iit != first->getAddresses().end(); ++iit){
                                foundMatch = foundMatch || (oit->type == iit->type && oit->addr == iit->addr && iit->contactId == oit->contactId);
                        }
                        equal = equal && foundMatch;
                }
                if (equal){
                        //logger::info("True");
                        return true;
                } //else logger::info("False");
                ++first;
        }
        return false;
}

TEST_F(Worker, CreateContactWithAddress){
        //Order is important here so IDs match up
        Contact EmailTest1 CONTACT_EmailTest1_ARGS;
        Contact LineTest2 CONTACT_LineTest2_ARGS;

        std::list<Contact> definedContacts;
        definedContacts.push_back(EmailTest1);
        definedContacts.push_back(LineTest2);


        worker::createContactWithAddress(EmailTest1.getAddresses()[0].addr, TYPE_EMAIL, EmailTest1.displayName);
        worker::createContactWithAddress(LineTest2.getAddresses()[0].addr, TYPE_LINE, LineTest2.displayName);


        auto contacts = reader::contactList();
        for (auto it = definedContacts.begin(); it != definedContacts.end();++it){
                ASSERT_TRUE(findContact(contacts->begin(), contacts->end(), *it));
        }


        //What we are now testing is moving existing addresses to new contacts with this method; this should snipe the contact from EmailTest1
        worker::createContactWithAddress(EmailTest1.getAddresses()[0].addr, TYPE_EMAIL, "EmailTest2");

        Contact NewEmailTest1(EmailTest1.id, EmailTest1.displayName, MessageTypeFlags[TYPE_EMAIL]); //No addresses
        Address NewEmailAddress(EmailTest1.getAddresses()[0].addr, EmailTest1.getAddresses()[0].count, 3, TYPE_EMAIL); //Have to create this so we can change the contactId
        Contact EmailTest2(3, "EmailTest2", MessageTypeFlags[TYPE_EMAIL], NewEmailAddress); //Id of 3 since created third, and holding EmailTest1's address that it took

        definedContacts.clear();
        definedContacts.push_back(NewEmailTest1);
        definedContacts.push_back(LineTest2);
        definedContacts.push_back(EmailTest2);

        contacts = reader::contactList();
        for (auto it = definedContacts.begin(); it != definedContacts.end();++it){
                ASSERT_TRUE(findContact(contacts->begin(), contacts->end(), *it));
        }

}

TEST_F(Worker, CreateContactFromAddress){

}

TEST_F(Worker, SaveAddress){

}

TEST_F(Worker, RecomputeAddressTable){

}

TEST_F(Worker, RewriteAddressTable){

}