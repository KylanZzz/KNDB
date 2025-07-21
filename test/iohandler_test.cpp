//
// Created by Kylan Chen on 2/28/25.
//

#include <gtest/gtest.h>
#include <fstream>

#include "IOHandler.hpp"

using namespace backend;

struct IOHandlerTest : testing::Test {
    std::unique_ptr<IOHandler> ioHandler;

    const std::string kTestFile = "testfile.db";

    void SetUp() override {
        std::remove(kTestFile.c_str());
        ioHandler = std::make_unique<IOHandler>(kTestFile);
    }

    void TearDown() override {
        ioHandler.reset();
        std::remove(kTestFile.c_str());
    }
};

TEST_F(IOHandlerTest, GetBlocksInitializesToZero) {
    ASSERT_EQ(0, ioHandler->getNumBlocks());
}

TEST_F(IOHandlerTest, CreateOneBlockWorks) {
    u32 pageid = ioHandler->createNewBlock();
    ASSERT_EQ(0, pageid);
    ASSERT_EQ(1, ioHandler->getNumBlocks());
}

TEST_F(IOHandlerTest, CreateMultipleBlocksWorks) {
    u32 pageid1 = ioHandler->createNewBlock();
    ASSERT_EQ(1, ioHandler->getNumBlocks());
    u32 pageid2 = ioHandler->createNewBlock();
    ASSERT_EQ(2, ioHandler->getNumBlocks());
    u32 pageid3 = ioHandler->createNewBlock();
    ASSERT_EQ(3, ioHandler->getNumBlocks());
    u32 pageid4 = ioHandler->createNewBlock();
    ASSERT_EQ(4, ioHandler->getNumBlocks());
    ASSERT_EQ(0, pageid1);
    ASSERT_EQ(1, pageid2);
    ASSERT_EQ(2, pageid3);
    ASSERT_EQ(3, pageid4);
}

TEST_F(IOHandlerTest, FilePersistsWithCorrectSizeAfterDestruction) {
    u32 expectedSize;

    ioHandler->createNewBlock();
    expectedSize = ioHandler->getNumBlocks() * cts::PG_SZ;
    ioHandler.reset();

    // file should still exist
    std::ifstream file(kTestFile, std::ios::binary);
    ASSERT_TRUE(file.good());

    // file should be 1 block large
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    ASSERT_EQ(fileSize, expectedSize);

    file.close();
}

TEST_F(IOHandlerTest, SingleBlockPersistsAfterDestruction) {
    char writeData[cts::PG_SZ] = "Persistent Block";
    u32 expectedSize;

    ioHandler->createNewBlock();
    ioHandler->writeBlock(writeData, 0);
    expectedSize = cts::PG_SZ;
    ioHandler.reset();

    std::ifstream file(kTestFile, std::ios::binary);
    ASSERT_TRUE(file.good());

    char fileData[cts::PG_SZ] = {0};
    file.read(fileData, cts::PG_SZ);
    ASSERT_EQ(memcmp(writeData, fileData, cts::PG_SZ), 0);

    file.seekg(0, std::ios::end);
    ASSERT_EQ(file.tellg(), expectedSize);
}

TEST_F(IOHandlerTest, MultipleBlocksPersistAfterDestruction) {
    char block1[cts::PG_SZ] = "Block One";
    char block2[cts::PG_SZ] = "Block Two";
    u32 expectedSize;

    ioHandler->createNewBlock();
    ioHandler->createNewBlock();
    ioHandler->writeBlock(block1, 0);
    ioHandler->writeBlock(block2, 1);
    expectedSize = cts::PG_SZ * 2;
    ioHandler.reset();

    std::ifstream file(kTestFile, std::ios::binary);
    ASSERT_TRUE(file.good());

    char fileData[cts::PG_SZ] = {};

    file.read(fileData, cts::PG_SZ);
    ASSERT_EQ(memcmp(block1, fileData, cts::PG_SZ), 0);

    file.read(fileData, cts::PG_SZ);
    ASSERT_EQ(memcmp(block2, fileData, cts::PG_SZ), 0);

    file.seekg(0, std::ios::end);
    ASSERT_EQ(file.tellg(), expectedSize);
}

TEST_F(IOHandlerTest, OverwrittenDataPersistsAfterDestruction) {
    char originalData[cts::PG_SZ] = "Original Block";
    char newData[cts::PG_SZ] = "Updated Block";

    ioHandler->createNewBlock();
    ioHandler->writeBlock(originalData, 0);
    ioHandler->writeBlock(newData, 0);
    ioHandler.reset();
    size_t expectedSize = cts::PG_SZ;

    std::ifstream file(kTestFile, std::ios::binary);
    ASSERT_TRUE(file.good());

    char fileData[cts::PG_SZ] = {0};
    file.read(fileData, cts::PG_SZ);
    ASSERT_EQ(memcmp(newData, fileData, cts::PG_SZ), 0);

    file.seekg(0, std::ios::end);
    ASSERT_EQ(file.tellg(), expectedSize);
}

TEST_F(IOHandlerTest, ReadBlockRetrievesWrittenData) {
    char writeData[cts::PG_SZ] = "Persistent Read Test";
    char readData[cts::PG_SZ] = {0};

    ioHandler->createNewBlock();
    ioHandler->writeBlock(writeData, 0);
    ioHandler.reset();

    IOHandler handler(kTestFile);
    handler.readBlock(readData, 0);
    ASSERT_EQ(memcmp(writeData, readData, cts::PG_SZ), 0);
}

TEST_F(IOHandlerTest, ReadMultipleBlocksAfterDestruction) {
    char block1[cts::PG_SZ] = "Block One";
    char block2[cts::PG_SZ] = "Block Two";
    char readBuffer[cts::PG_SZ] = {0};

    ioHandler->createNewBlock();
    ioHandler->createNewBlock();
    ioHandler->writeBlock(block1, 0);
    ioHandler->writeBlock(block2, 1);
    ioHandler.reset();

    IOHandler handler(kTestFile);
    handler.readBlock(readBuffer, 0);
    ASSERT_EQ(memcmp(block1, readBuffer, cts::PG_SZ), 0);

    handler.readBlock(readBuffer, 1);
    ASSERT_EQ(memcmp(block2, readBuffer, cts::PG_SZ), 0);
}

TEST_F(IOHandlerTest, WriteToNonExistentBlock) {
    char data[cts::PG_SZ] = "Out-of-bounds write";
    ASSERT_THROW(ioHandler->writeBlock(data, 0), std::runtime_error);
    ASSERT_THROW(ioHandler->writeBlock(data, 1), std::runtime_error);
    ASSERT_THROW(ioHandler->writeBlock(data, 5), std::runtime_error);
    ASSERT_THROW(ioHandler->writeBlock(data, 10000), std::runtime_error);
}

TEST_F(IOHandlerTest, ReadUninitializedBlock) {
    char readData[cts::PG_SZ] = {0};
    ASSERT_THROW(ioHandler->readBlock(readData, 0), std::runtime_error);
    ASSERT_THROW(ioHandler->readBlock(readData, 1), std::runtime_error);
    ASSERT_THROW(ioHandler->readBlock(readData, 5), std::runtime_error);
}

TEST_F(IOHandlerTest, CreateMultipleBlocksAllocatesCorrectly) {
    u32 startBlock = ioHandler->createMultipleBlocks(3);
    ASSERT_EQ(startBlock, 0);
    ASSERT_EQ(ioHandler->getNumBlocks(), 3);
}

TEST_F(IOHandlerTest, CreateMultipleBlocksSequentiallyIncreasesCount) {
    ioHandler->createMultipleBlocks(2);
    u32 nextStart = ioHandler->createMultipleBlocks(4);
    ASSERT_EQ(nextStart, 2);
    ASSERT_EQ(ioHandler->getNumBlocks(), 6);
}

TEST_F(IOHandlerTest, CreateMultipleBlocksPersistsAfterDestruction) {
    u32 blockCount = 5;
    ioHandler->createMultipleBlocks(blockCount);
    ioHandler.reset();

    std::ifstream file(kTestFile, std::ios::binary);
    ASSERT_TRUE(file.good());

    file.seekg(0, std::ios::end);
    ASSERT_EQ(file.tellg(), blockCount * cts::PG_SZ);
}

TEST_F(IOHandlerTest, WriteAndReadToMultipleBlocks) {
    const int blockCount = 4;
    ioHandler->createMultipleBlocks(blockCount);

    char data[cts::PG_SZ] = "Block Data";
    char buffer[cts::PG_SZ] = {0};

    for (int i = 0; i < blockCount; ++i) {
        ioHandler->writeBlock(data, i);
    }

    ioHandler.reset();
    ioHandler = std::make_unique<IOHandler>(kTestFile);

    for (int i = 0; i < blockCount; ++i) {
        ioHandler->readBlock(buffer, i);
        ASSERT_EQ(memcmp(data, buffer, cts::PG_SZ), 0);
    }
}