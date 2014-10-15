#include <iostream>
#include "../src/favor.h"
#include "../src/worker.h"
#include "../src/reader.h"
#include "../src/logger.h"
//TODO: this should be replaced by a real testing framework ASAP

int main(int argc, char **argv) {
    favor::initialize();
    favor::worker::buildDatabase();
    favor::reader::refreshAll();
    favor::logger::info(favor::as_string(favor::reader::accountList().front()->type));
    favor::logger::info(favor::reader::accountList().front()->accountName);
    favor::reader::accountList().front()->updateMessages();
    //favor::reader::accountList().front()->updateContacts()

    /*
    This seem to be generally good, except it's hard to know because it seems like one of the emails I'm running tests on, after being processed for whitespace,
    starts including a character that's causing printing via cout << to terminate early? It's hard to know, but we'll be able to know for certain ocne we start
    saving results to the database.
     */

    return 0;
}