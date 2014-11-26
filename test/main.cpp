#include <iostream>
#include "../src/favor.h"
#include "../src/worker.h"
#include "../src/reader.h"
#include "../src/logger.h"
//TODO: this should be replaced by a real testing framework ASAP

using namespace std;
using namespace favor;

int main(int argc, char **argv) {
    initialize();
    worker::buildDatabase();
    reader::refreshAll();

    double avg = reader::average(reader::accountList()->front(), reader::contactList()->front(), KEY_CHARCOUNT, 500, -1, true);
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