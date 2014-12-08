#include <iostream>
#include <processor.h>
#include "favor_tests.h"
#include "worker.h"
#include "reader_tests.h"
#include "misc_tests.h"
#include "logger.h"

#include <chrono>
//TODO: this should be replaced by a real testing framework ASAP


#include "gtest/gtest.h"

using namespace std;
using namespace favor;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}