//
// Created by Kylan Chen on 2/28/25.
//

#include <gtest/gtest.h>

#include "IOHandler.hpp"

using std::string;

struct IOHandlerTest : testing::Test {
    std::unique_ptr<IOHandler> ioHandler;
    IOHandlerTest() {
        ioHandler = std::make_unique<IOHandler>("testfile.db");
    }
};

// Demonstrate some basic assertions.
TEST_F(IOHandlerTest, GetBlocksInitializesToZero) {
    EXPECT_EQ(0, ioHandler->getNumBlocks());
}