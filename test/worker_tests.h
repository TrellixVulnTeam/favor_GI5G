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
                       // logger::info("True");
                        return true;
                } //else logger::info("False");
                ++first;
        }
        return false;
}

TEST_F(Worker, CreateContactWithAddress){
        Contact EmailTest1 CONTACT_EmailTest1_ARGS;
        Contact LineTest2 CONTACT_LineTest2_ARGS;

        std::list<Contact> definedContacts;
        definedContacts.push_back(EmailTest1);
        definedContacts.push_back(LineTest2);

        //Order is important here so predefined IDs match up
        worker::createContactWithAddress(EmailTest1.getAddresses()[0].addr, TYPE_EMAIL, EmailTest1.displayName);
        worker::createContactWithAddress(LineTest2.getAddresses()[0].addr, TYPE_LINE, LineTest2.displayName);


        auto contacts = reader::contactList();
        for (auto it = definedContacts.begin(); it != definedContacts.end();++it){
                ASSERT_TRUE(findContact(contacts->begin(), contacts->end(), *it));
        }


        //What we are now testing is moving existing addresses to new contacts with this method; this should snipe the contact from EmailTest1
        worker::createContactWithAddress(EmailTest1.getAddresses()[0].addr, TYPE_EMAIL, "EmailTest2");

        Contact NewEmailTest1(EmailTest1.id, EmailTest1.displayName); //No addresses
        Address NewEmailAddress(EmailTest1.getAddresses()[0].addr, EmailTest1.getAddresses()[0].count, 3, TYPE_EMAIL); //Have to create this so we can change the contactId
        Contact EmailTest2(3, "EmailTest2", NewEmailAddress); //Id of 3 since created third, and holding EmailTest1's address that it took

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
        worker::exec(contactSeed); //Have to seed contacts first so the foreign key constraint works on addresses
        worker::exec(addressSeed);

        Contact EmailTest1Old CONTACT_EmailTest1_ARGS;
        Contact LineTest2Old CONTACT_LineTest2_ARGS;

        worker::createContactFromAddress(EmailTest1Old.getAddresses()[0], EmailTest1Old.displayName);
        worker::createContactFromAddress(LineTest2Old.getAddresses()[0], LineTest2Old.displayName);
        //TODO: it might be worthwhile to create a contact with a non-database-existant address and make sure we catch (log) that, but
        //I don't see a good way to detect whether something has been logged or not, for example

        std::vector<Address> EmailAddrs;
        for (auto it = EmailTest1Old.getAddresses().begin(); it != EmailTest1Old.getAddresses().end(); ++it){
                EmailAddrs.push_back(Address(it->addr, it->count, CONTACT_COUNT+1, it->type));
        }

        std::vector<Address> LineAddrs;
        for (auto it = LineTest2Old.getAddresses().begin(); it != LineTest2Old.getAddresses().end(); ++it) {
                LineAddrs.push_back(Address(it->addr, it->count, CONTACT_COUNT+2, it->type));
        }
        //Ids here(above and below) have to match the order of contact creations
        Contact CustomEmailTest(CONTACT_COUNT+1, EmailTest1Old.displayName, EmailAddrs);
        Contact CustomLineTest(CONTACT_COUNT+2, LineTest2Old.displayName, LineAddrs);

        //These are the original email/line test contacts without their addresses now that those addresses have moved to the above "customX" contacts
        Contact EmptyEmailTest(EmailTest1Old.id, EmailTest1Old.displayName);
        Contact EmptyLineTest(LineTest2Old.id, LineTest2Old.displayName);

        std::list<Contact> definedContacts;
        definedContacts.push_back(CustomEmailTest);
        definedContacts.push_back(CustomLineTest);
        definedContacts.push_back(EmptyEmailTest);
        definedContacts.push_back(EmptyLineTest);


        auto contacts = reader::contactList();
        for (auto it = definedContacts.begin(); it != definedContacts.end();++it){
                ASSERT_TRUE(findContact(contacts->begin(), contacts->end(), *it));
        }

}

TEST_F(Worker, SaveAddress){

}

TEST_F(Worker, RecomputeAddressTable){

}

TEST_F(Worker, RewriteAddressTable){

}