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

    double avg = reader::average(reader::accountList()->front(), reader::contactList(TYPE_EMAIL)->front(), CHARCOUNT, 500, -1, true);
    logger::info("Average character count length sent to "+reader::contactList(TYPE_EMAIL)->front().displayName+": "+as_string(avg));


    return 0;
}