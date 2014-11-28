#include <iostream>
#include <processor.h>
#include "../src/favor.h"
#include "../src/worker.h"
#include "../src/reader.h"
#include "../src/logger.h"

#include <chrono>
//TODO: this should be replaced by a real testing framework ASAP

using namespace std;
using namespace favor;

int main(int argc, char **argv) {
    initialize();
    worker::buildDatabase();
    reader::refreshAll();
    logger::info("A");


//    reader::accountList()->front()->updateAddresses();
//    reader::accountList()->front()->updateMessages();

    double avg;// = processor::averageCharcount(reader::accountList()->front(), reader::contactList()->front(), 500, -1, true);

    auto start = chrono::steady_clock::now();
    for (long l = 0; l < 10; l++){
        avg = processor::averageCharcount(reader::accountList()->front(), reader::contactList()->front(), 500, -1, true);
    }
    auto end = chrono::steady_clock::now();

    cout << "Duration: " << chrono::duration<double, milli>(end-start).count() << " ns" << endl;
    logger::info("Average character count length sent to "+reader::contactList()->front().displayName+": "+as_string(avg));
    logger::info("List messages from"+reader::contactList()->back().displayName);
    auto result = reader::queryContact(reader::accountList()->front(), reader::contactList()->back(), KEY_ALL, 500, 1411419510, false);
    for (auto it = result->begin(); it != result->end(); ++it){
        logger::info(as_string(*it));
    }

    logger::info("List conversations including"+reader::contactList()->back().displayName);
    auto result2 = reader::queryConversation(reader::accountList()->front(), reader::contactList()->back(), KEY_ALL, 500, -1);
    for (auto it = result2->begin(); it != result2->end(); ++it){
        logger::info(as_string(*it));
    }



    return 0;
}