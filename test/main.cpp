#include <iostream>
#include "../src/favor.h"
#include "../src/worker.h"
#include "../src/reader.h"
//TODO: this should be replaced by a real testing framework ASAP

int main(int argc, char **argv) {
    favor::initialize();
    favor::worker::buildDatabase();
    favor::worker::addAccount("testemail@gmail.com", favor::TYPE_EMAIL, "{}");
    //favor::worker::removeAccount("testemail@gmail.com", favor::TYPE_EMAIL);
    favor::reader::refreshAll();
    cout << favor::reader::accountList().front().type << endl;
    cout << favor::reader::accountList().front().accountName << endl;
    return 0;
}
