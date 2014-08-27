#include <iostream>
#include "../src/favor.h"
#include "../src/worker.h"
//TODO: this should be replaced by a real testing framework ASAP

int main(int argc, char **argv) {
    favor::initialize();
    //favor::worker::buildDatabase();
    //favor::worker::add_account("testemail@gmail.com", favor::TYPE_EMAIL, "");
    return 0;
}
