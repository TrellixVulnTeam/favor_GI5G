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
        long percentile(float percent, const vector<long>& input);
        long standardDeviationFloor(int deviations, const vector<long>& input);
        vector<long> denseTimes(const vector<time_t>& input);
    }
}


using namespace std;
using namespace favor;

//base+minutes
#define BASE 5*60 //Everything starts from 5 minutes
#define BM(mins) mins*60 + BASE

TEST(Processor, StrippedDates){
    //Messages are sorted descending
    std::shared_ptr<std::vector<Message>> msgs = std::make_shared<std::vector<Message>>();
    std::string addr("test@test.com");
    int id = 0;
    //These are inserted in descending order so they match what we expect from the database; if DB_SORT_ORDER changes
    //then the order here will have to change too
    msgs->push_back(Message(TYPE_EMAIL, false, ++id, BM(22), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(20), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(19), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(12), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(7), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(3), addr, 0, 1));

    /*
        First holds response times for us, second for the other party.
     */

    auto result1 = processor::strippedDates(msgs);
    ASSERT_EQ(0, result1->first.size());
    ASSERT_EQ(1, result1->second.size());
    ASSERT_EQ((result1->second)[0], 3*60);


    msgs->clear();
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(56), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, false, ++id, BM(55), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, false, ++id, BM(50), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(34), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, false, ++id, BM(30), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, false, ++id, BM(12), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(10), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(4), addr, 0, 1));

    auto result2 = processor::strippedDates(msgs);

    ASSERT_EQ(2, result2->first.size());
    ASSERT_EQ(2, result2->second.size());
    ASSERT_EQ((result2->first)[0], 4*60);
    ASSERT_EQ((result2->first)[1], 6*60);
    ASSERT_EQ((result2->second)[0], 8*60);
    ASSERT_EQ((result2->second)[1], 16*60);
}

TEST(Processor, Percentile){
    std::vector<long> nums = {2,4,7,10,11,14};
    ASSERT_EQ(processor::percentile(0.60, nums), 10);
    nums = {-4,-3,-2,-1,0, 1, 2, 3, 4, 5};
    ASSERT_EQ(processor::percentile(0.90, nums), 4);

}

TEST(Processor, StandardDeviationFloor){
    std::vector<long> nums = {2,4,4,4,5,5,7,9};
    ASSERT_EQ(processor::standardDeviationFloor(1, nums), 3);
    ASSERT_EQ(processor::standardDeviationFloor(2, nums), 1);
}

TEST(Processor, DenseTimes){
    std::vector<time_t> times = {BM(3),BM(7),BM(7),BM(10),BM(13),BM(17),BM(20),BM(21),BM(34),BM(55),BM(70)};
    processor::denseTimes(times);
}