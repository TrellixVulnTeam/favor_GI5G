#include <iostream>
#include "../src/favor.h"
#include "../src/worker.h"
#include "../src/reader.h"
//TODO: this should be replaced by a real testing framework ASAP

int main(int argc, char **argv) {
    favor::initialize();
    favor::reader::refreshAll(); //TODO: these methods are strangely (see: stupidly) circularly reliant on each other in a way that doesn't make any sense
    //building the database shouldn't involve reading from the database. that's retarded.
    favor::worker::buildDatabase(); //reliant on refreshAll. We might want some worker methods to be able to know if they need to run an initial refresh...
    favor::worker::addAccount("testemail@gmail.com", favor::TYPE_EMAIL, "{}");
    cout << favor::reader::accountList().front().type << endl;
    cout << favor::reader::accountList().front().accountName << endl;
    return 0;
}
