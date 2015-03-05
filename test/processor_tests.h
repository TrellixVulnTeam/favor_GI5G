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
        shared_ptr<SentRec<vector<time_t>>> strippedDates(shared_ptr<vector<Message>> messages);
        long percentile(float percent, vector<long>& input);
        long standardDeviationFloor(int deviations, const std::vector<std::pair<long,long>>& input);
        vector<long> denseTimes(const vector<time_t>& input);
        SentRec<long> responseTimeNintiethCompute(shared_ptr<vector<Message>> query);
        SentRec<double> conversationalResponseTimeCompute(shared_ptr<vector<Message>> query);
    }

}


//TODO: test the caching methods

using namespace std;
using namespace favor;

//base+minutes
#define BASE 5*60 //Everything starts from 5 minutes
#define BM(mins) mins*60 + BASE

/*
    Note: A lot of these tests are checking the results against hand-computed algorithm results because there isn't really
    a better way to do it
 */


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


    auto result1 = processor::strippedDates(msgs);
    ASSERT_EQ(0, result1->sent.size());
    ASSERT_EQ(1, result1->received.size());
    ASSERT_EQ((result1->received)[0], 3*60);


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

    ASSERT_EQ(2, result2->sent.size());
    ASSERT_EQ(2, result2->received.size());
    ASSERT_EQ((result2->sent)[0], 4*60);
    ASSERT_EQ((result2->sent)[1], 6*60);
    ASSERT_EQ((result2->received)[0], 8*60);
    ASSERT_EQ((result2->received)[1], 16*60);
}

TEST(Processor, Percentile){
    std::vector<long> nums = {2,4,7,10,11,14};
    ASSERT_EQ(processor::percentile(0.60, nums), 10);
    nums = {-4,-3,-2,-1,0, 1, 2, 3, 4, 5};
    ASSERT_EQ(processor::percentile(0.90, nums), 4);

}

TEST(Processor, StandardDeviationFloor){
    std::vector<std::pair<long,long>> nums = {{0,2},{0,4},{0,4},{0,4},{0,5},{0,5},{0,7},{0,9}};
    ASSERT_EQ(processor::standardDeviationFloor(1, nums), 3);
    ASSERT_EQ(processor::standardDeviationFloor(2, nums), 1);
}

TEST(Processor, DenseTimes){
    std::vector<time_t> times = {BM(3),BM(7),BM(7),BM(10),BM(13),BM(17),BM(20),BM(21),BM(34),BM(55),BM(70)};
    std::vector<time_t> badTimes = {BM(34),BM(55),BM(70)};
    std::vector<time_t> goodTimes = {BM(3),BM(7),BM(7),BM(10),BM(13),BM(17),BM(20),BM(21)};
    ASSERT_EQ(badTimes.size()+goodTimes.size(), times.size());


    std::vector<long> result = processor::denseTimes(times);
    ASSERT_EQ(goodTimes.size(), result.size());
    for (auto it = goodTimes.begin(); it != goodTimes.end(); ++it){
        ASSERT_NE(std::find(result.begin(), result.end(), *it), result.end());
    }
    for (auto it = badTimes.begin(); it != badTimes.end(); ++it){
        ASSERT_EQ(std::find(result.begin(), result.end(), *it), result.end());
    }

}

TEST(Processor, ResponseTimeNintieth){
    //Example has no discards, but we're not testing discarding here - that's in date stripping
    std::shared_ptr<std::vector<Message>> msgs = std::make_shared<std::vector<Message>>();
    std::string addr("test@test.com");
    int id = 0;

    //Inserted in reverse order to match DB sort

    msgs->push_back(Message(TYPE_EMAIL, false, ++id, BM(17), addr, 0, 1));

    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(17), addr, 0, 1));

    msgs->push_back(Message(TYPE_EMAIL, false, ++id, BM(16), addr, 0, 1));

    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(15), addr, 0, 1));

    msgs->push_back(Message(TYPE_EMAIL, false, ++id, BM(14), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, false, ++id, BM(12), addr, 0, 1));

    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(10), addr, 0, 1));

    msgs->push_back(Message(TYPE_EMAIL, false, ++id, BM(5), addr, 0, 1));

    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(3), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(2), addr, 0, 1));

    SentRec<long> times = favor::processor::responseTimeNintiethCompute(msgs);
    //Nintieth here is just largest number with such small arrays
    ASSERT_EQ(times.sent, 5*60);
    ASSERT_EQ(times.received, 3*60);

    msgs->clear();
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(3), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(2), addr, 0, 1));

    times = favor::processor::responseTimeNintiethCompute(msgs);
    ASSERT_EQ(times.sent, 0); //3.33 = 3
    ASSERT_EQ(times.received, 0); //1.5 =2
}

TEST(Processor, AverageConversationalResponseTime){

    std::shared_ptr<std::vector<Message>> msgs = std::make_shared<std::vector<Message>>();
    std::string addr("test@test.com");
    int id = 0;

    msgs->push_back(Message(TYPE_EMAIL, false, ++id, BM(17), addr, 0, 1));

    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(17), addr, 0, 1));

    msgs->push_back(Message(TYPE_EMAIL, false, ++id, BM(16), addr, 0, 1));

    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(15), addr, 0, 1));

    msgs->push_back(Message(TYPE_EMAIL, false, ++id, BM(14), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, false, ++id, BM(12), addr, 0, 1));

    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(10), addr, 0, 1));

    msgs->push_back(Message(TYPE_EMAIL, false, ++id, BM(5), addr, 0, 1));

    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(3), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(2), addr, 0, 1));

    logger::info("test1");
    SentRec<double> times = favor::processor::conversationalResponseTimeCompute(msgs);
    ASSERT_EQ(times.sent, 3*60); //3.33 = 3
    ASSERT_EQ(times.received, 1.5*60); //1.5 =2

    logger::info("test2");
    msgs->clear();
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(3), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, BM(2), addr, 0, 1));

    times = favor::processor::conversationalResponseTimeCompute(msgs);
    ASSERT_EQ(times.sent, 0); //3.33 = 3
    ASSERT_EQ(times.received, 0); //1.5 =2

}