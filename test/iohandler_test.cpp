//
// Created by Kylan Chen on 2/28/25.
//

#include <gtest/gtest.h>
#include <fstream>

#include "IOHandler.hpp"

using namespace backend;

struct IOHandlerTest : testing::Test {
    std::unique_ptr<IOHandler> ioHandler;

    IOHandlerTest() {
        std::ofstream file("testfile.db", std::ios::trunc);
        file.close();
        ioHandler = std::make_unique<IOHandler>("testfile.db");
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

TEST(IOHandlerPersistenceTest, FilePersistsWithCorrectSizeAfterDestruction) {
    u32 expectedSize;

    {
        IOHandler handler("testfile.db");
        handler.createNewBlock();
        expectedSize = handler.getNumBlocks() * cts::PG_SZ;
    } // handler destructed

    // file should still exist
    std::ifstream file("testfile.db", std::ios::binary);
    ASSERT_TRUE(file.good());

    // file should be 1 block large
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    ASSERT_EQ(fileSize, expectedSize);

    file.close();
}

TEST(IOHandlerPersistenceTest, SingleBlockPersistsAfterDestruction) {
    char writeData[cts::PG_SZ] = "Persistent Block";
    u32 expectedSize;

    {
        IOHandler handler("testfile.db");
        handler.writeBlock(writeData, 0);
        expectedSize = handler.getNumBlocks() * cts::PG_SZ;
    } // IOHandler destructs here, flushing data to disk

    std::ifstream file("testfile.db", std::ios::binary);
    ASSERT_TRUE(file.good());

    char fileData[cts::PG_SZ] = {0};
    file.read(fileData, cts::PG_SZ);
    ASSERT_EQ(memcmp(writeData, fileData, cts::PG_SZ), 0);

    file.seekg(0, std::ios::end);
    ASSERT_EQ(file.tellg(), expectedSize);
}

TEST(IOHandlerPersistenceTest, MultipleBlocksPersistAfterDestruction) {
    char block1[cts::PG_SZ] = "Block One";
    char block2[cts::PG_SZ] = "Block Two";
    u32 expectedSize;

    {
        IOHandler handler("testfile.db");
        handler.createNewBlock();
        handler.createNewBlock();
        handler.writeBlock(block1, 0);
        handler.writeBlock(block2, 1);
        expectedSize = handler.getNumBlocks() * cts::PG_SZ;
    } // IOHandler destructs, ensuring persistence

    std::ifstream file("testfile.db", std::ios::binary);
    ASSERT_TRUE(file.good());

    char fileData[cts::PG_SZ] = {};

    file.read(fileData, cts::PG_SZ);
    ASSERT_EQ(memcmp(block1, fileData, cts::PG_SZ), 0);

    file.read(fileData, cts::PG_SZ);
    ASSERT_EQ(memcmp(block2, fileData, cts::PG_SZ), 0);

    file.seekg(0, std::ios::end);
    ASSERT_EQ(file.tellg(), expectedSize);
}

TEST(IOHandlerPersistenceTest, OverwrittenDataPersistsAfterDestruction) {
    char originalData[cts::PG_SZ] = "Original Block";
    char newData[cts::PG_SZ] = "Updated Block";
    u32 expectedSize;

    {
        IOHandler handler("testfile.db");
        handler.writeBlock(originalData, 0);
        handler.writeBlock(newData, 0);  // Overwrite previous data
        expectedSize = handler.getNumBlocks() * cts::PG_SZ;
    } // IOHandler destructs

    std::ifstream file("testfile.db", std::ios::binary);
    ASSERT_TRUE(file.good());

    char fileData[cts::PG_SZ] = {0};
    file.read(fileData, cts::PG_SZ);
    ASSERT_EQ(memcmp(newData, fileData, cts::PG_SZ), 0);

    file.seekg(0, std::ios::end);
    ASSERT_EQ(file.tellg(), expectedSize);
}

TEST(IOHandlerPersistenceTest, ReadBlockRetrievesWrittenData) {
    char writeData[cts::PG_SZ] = "Persistent Read Test";
    char readData[cts::PG_SZ] = {0};

    {
        IOHandler handler("testfile.db");
        handler.writeBlock(writeData, 0);
    }

    IOHandler handler("testfile.db");
    handler.readBlock(readData, 0);
    ASSERT_EQ(memcmp(writeData, readData, cts::PG_SZ), 0);
}

TEST(IOHandlerPersistenceTest, ReadMultipleBlocksAfterDestruction) {
    char block1[cts::PG_SZ] = "Block One";
    char block2[cts::PG_SZ] = "Block Two";
    char readBuffer[cts::PG_SZ] = {0};

    {
        IOHandler handler("testfile.db");
        handler.createNewBlock();
        handler.createNewBlock();
        handler.writeBlock(block1, 0);
        handler.writeBlock(block2, 1);
    }

    IOHandler handler("testfile.db");
    handler.readBlock(readBuffer, 0);
    ASSERT_EQ(memcmp(block1, readBuffer, cts::PG_SZ), 0);

    handler.readBlock(readBuffer, 1);
    ASSERT_EQ(memcmp(block2, readBuffer, cts::PG_SZ), 0);
}

TEST_F(IOHandlerTest, WriteToNonExistentBlock) {
    char data[cts::PG_SZ] = "Out-of-bounds write";
    ASSERT_THROW(ioHandler->writeBlock(data, 5), std::runtime_error);
}

TEST_F(IOHandlerTest, ReadUninitializedBlock) {
    char readData[cts::PG_SZ] = {0};
    ASSERT_THROW(ioHandler->readBlock(readData, 0), std::runtime_error);
}

TEST(IOHandlerEdgeTest, WriteToReadOnlyFile) {
    std::ofstream file("testfile.db", std::ios::trunc);
    file.close();

    IOHandler handler("testfile.db");
    chmod("testfile.db", 0444);

    char data[cts::PG_SZ] = "Read-only test";
    ASSERT_THROW(handler.writeBlock(data, 0), std::runtime_error);

    chmod("testfile.db", 0644);
}

