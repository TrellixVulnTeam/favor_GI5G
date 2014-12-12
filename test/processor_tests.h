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
        shared_ptr<vector<time_t>> strippedDates(shared_ptr<vector<Message>> messages);
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
    //TODO: much broken

    //Messages are sorted descending
    std::shared_ptr<std::vector<Message>> msgs = std::make_shared<std::vector<Message>>();
    std::string addr("test@test.com");
    int id = 0;
    //Message(MessageType t, bool s, long int i, std::time_t d, string a, short m, long cc, const string& b);
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, bm(3), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, bm(7), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, bm(12), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, bm(17), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, true, ++id, bm(20), addr, 0, 1));
    msgs->push_back(Message(TYPE_EMAIL, false, ++id, bm(22), addr, 0, 1));

    auto result1 = processor::strippedDates(msgs);
    for (auto it = result1->begin(); it != result1->end(); ++it) logger::info(as_string(*it));
    ASSERT_EQ(1, result1->size());
    ASSERT_TRUE((*result1)[0]==5*60);

}