#include "favor.h"
#include "reader.h"
#include "address.h"
#include "worker.h"
#include "accountmanager.h"
#include "gtest/gtest.h"
#include "testdata.h"
#include "testing.h"

namespace favor{
    namespace processor{
        //Defining the methods here is a cheesy way to get at the methods defined in the processor file but not exposed
        //in the normal header
        shared_ptr<std::pair<vector<time_t>,vector<time_t>>> strippedDates(shared_ptr<vector<Message>> messages);
    }
}


using namespace std;
using namespace favor;

//base+minutes
long bm(long mins){
    long base = 5*60; //Everything starts from 5 minutes
    return mins*60 + base;
}

TEST(Processor, StrippedDates){
    //Messages are sorted descending
    std::shared_ptr<std::vector<Message>> msgs = std::make_shared<std::vector<Message>>();
    std::string addr("test@test.com");
    int id = 0;
    //These are inserted in descending order so they match what we expect from the database; if DB_SORT_ORDER changes
    //then the order here will have to change too
    msgs->push_back(Message(TYPE_EMAIL, false, ++id, bm(22), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, bm(20), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, bm(19), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, bm(12), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, bm(7), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, bm(3), addr, 0, 1));

    /*
        First holds response times for us, second for the other party.
     */

    auto result1 = processor::strippedDates(msgs);
    ASSERT_EQ(0, result1->first.size());
    ASSERT_EQ(1, result1->second.size());
    ASSERT_EQ((result1->second)[0], 3*60);


    msgs->clear();
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, bm(56), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, false, ++id, bm(55), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, false, ++id, bm(50), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, bm(34), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, false, ++id, bm(30), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, false, ++id, bm(12), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, bm(10), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, bm(4), addr, 0, 1));

    auto result2 = processor::strippedDates(msgs);

    ASSERT_EQ(2, result2->first.size());
    ASSERT_EQ(2, result2->second.size());
    ASSERT_EQ((result2->first)[0], 4*60);
    ASSERT_EQ((result2->first)[1], 6*60);
    ASSERT_EQ((result2->second)[0], 8*60);
    ASSERT_EQ((result2->second)[1], 16*60);
}