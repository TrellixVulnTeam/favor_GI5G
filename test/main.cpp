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
    //favor::reader::accountList().front()->updateContacts();
    return 0;
}