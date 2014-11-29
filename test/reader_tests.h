#include "favor.h"
#include "reader.h"
#include "gtest/gtest.h"

using namespace std;
using namespace favor;

class DatabaseReadingTest : public ::testing::Test {
    protected:
        virtual void SetUp() override {
            //TODO: a lot of work. we basically have to generate a dummy database here. this sucks, but we should really only have to write it once
            //we can eventually extract and refactor a lot of the same work into worker/etc. database tests too
        }

        virtual void TearDown() override { //TODO: necessary?

        }
};