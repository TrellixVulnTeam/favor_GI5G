#include "favor_tests.h"
#include "reader_tests.h"
#include "misc_tests.h"
#include "worker_tests.h"
#include "accountmanager_tests.h"
#include "processor_tests.h"
#include "managers/emailmanager_tests.h"
#include "managers/skypemanager_tests.h"


#include "gtest/gtest.h"

using namespace std;
using namespace favor;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}