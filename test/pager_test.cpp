//
// Created by Kylan Chen on 3/2/25.
//

#include <gtest/gtest.h>
#include <fstream>

#include "Pager.hpp"
#include "IOHandler.hpp"
#include "SchemaPage.hpp"
#include "constants.hpp"
#include "TablePage.hpp"
#include "PageCache.hpp"
#include "FreeSpaceMap.hpp"

using namespace backend;

class PagerTest : public testing::Test {
protected:
    const std::string kTestFile = "testfile.db";

    std::unique_ptr<IOHandler> ioHandler;
    std::unique_ptr<PageCache> pageCache;
    std::unique_ptr<FreeSpaceMap> fsm;
    std::unique_ptr<Pager> pager;

    void SetUp() override {
        std::remove(kTestFile.c_str());
        initEnv();
    }

    void TearDown() override {
        resetEnv();
        std::remove(kTestFile.c_str());
    }

    void initEnv() {
        ioHandler = std::make_unique<IOHandler>(kTestFile);
        pageCache = std::make_unique<PageCache>(*ioHandler);
        fsm = std::make_unique<FreeSpaceMap>(*pageCache);
        pager = std::make_unique<Pager>(*fsm, *ioHandler, *pageCache);
    }

    void resetEnv() {
        pager.reset();
        fsm.reset();
        pageCache.reset();
        ioHandler.reset();
    }

    string getDBFileName() {return kTestFile;};
};

TEST_F(PagerTest, IsFreeWorksAfterAllocation) {
    auto& page = pager->createNewPage<FSMPage>();
    auto pageID = page.getPageID();

    ASSERT_FALSE(pager->isFree(pageID));
    pager->freePage(pageID);
    ASSERT_TRUE(pager->isFree(pageID));
}

TEST_F(PagerTest, IsFreeWorksForOutOfBounds) {
    for (int i = 2; i < 1000; i++) {
        ASSERT_TRUE(pager->isFree(i));
    }
}

TEST_F(PagerTest, PagerThrowsWhenNotSchemaPage) {
    auto pgid = pager->createNewPage<FSMPage>().getPageID();
    ASSERT_THROW(pager->getPage<SchemaPage>(pgid), std::bad_cast);
}

TEST_F(PagerTest, PagerDeathWhenNoSchemaPage) {
    ASSERT_DEATH(pager->getPage<SchemaPage>(cts::SCHEMA_ID), "");
}

TEST_F(PagerTest, PagerCreatesSchemaPage) {
    pager->createNewPage<SchemaPage>();
    ASSERT_NO_FATAL_FAILURE(pager->getPage<SchemaPage>(cts::SCHEMA_ID));
    ASSERT_EQ(pager->getPage<SchemaPage>(cts::SCHEMA_ID).getPageID(), cts::SCHEMA_ID);
}

TEST_F(PagerTest, SchemaPageWorksAfterSerializing) {
    u32 schemaPageID = pager->createNewPage<SchemaPage>().getPageID();
    pager->getPage<SchemaPage>(schemaPageID).addTable("MyTable", 3);
    pager->getPage<SchemaPage>(schemaPageID).addTable("AnotherTable", 4);

    resetEnv();
    initEnv();

    ASSERT_EQ(pager->getPage<SchemaPage>(schemaPageID).getNumTables(), 2);
    std::unordered_map<string, u32> expected = {{"MyTable", 3}, {"AnotherTable", 4}};
    ASSERT_EQ(pager->getPage<SchemaPage>(schemaPageID).getTables(), expected);
}

TEST_F(PagerTest, CreateNewPageAllocatesUniquePageID) {
    u32 pageID1 = pager->createNewPage<SchemaPage>().getPageID();
    u32 pageID2 = pager->createNewPage<SchemaPage>().getPageID();

    ASSERT_NE(pageID1, pageID2);
}

TEST_F(PagerTest, SchemaPageTablePersistenceAfterPagerDestruction) {
    u32 pageID = pager->createNewPage<SchemaPage>().getPageID();
    pager->getPage<SchemaPage>(pageID).addTable("Users", 5);
    pager->getPage<SchemaPage>(pageID).addTable("Orders", 10);
    auto expectedTables = pager->getPage<SchemaPage>(pageID).getTables();

    resetEnv();
    initEnv();

    ASSERT_EQ(pager->getPage<SchemaPage>(pageID).getTables(), expectedTables);
}

TEST_F(PagerTest, GetPageReturnsSameDataOnRepeatedCalls) {
    u32 pageID = pager->createNewPage<SchemaPage>().getPageID();
    pager->getPage<SchemaPage>(pageID).addTable("Users", 5);
    pager->getPage<SchemaPage>(pageID).addTable("Orders", 10);
    auto expectedTables = pager->getPage<SchemaPage>(pageID).getTables();

    auto& retrievedPage = pager->getPage<SchemaPage>(pageID);
    ASSERT_EQ(retrievedPage.getTables(), expectedTables);
}

TEST_F(PagerTest, FreePageAllowsReallocation) {
    u32 freedPageID = pager->createNewPage<SchemaPage>().getPageID();
    pager->freePage(freedPageID);
    u32 reallocatedPageID = pager->createNewPage<SchemaPage>().getPageID();
    ASSERT_EQ(freedPageID, reallocatedPageID);
}

TEST_F(PagerTest, GetPageDeathForFreedPage) {
    u32 pageID = pager->createNewPage<SchemaPage>().getPageID();
    pager->freePage(pageID);
    ASSERT_DEATH(pager->getPage<SchemaPage>(pageID), "");
}

TEST_F(PagerTest, ThrowsIfExceedsMaxBlocks) {
    std::vector<u32> pageIDs;

    // Subtract one page per FSMPage for bitmap page
    u32 numPagesToAlloc = (FSMPage::getBlocksInPage() - 1) * cts::MAX_FSMPAGES;
    for (u32 i = 0; i < numPagesToAlloc; ++i) {
        auto& schemaPage = pager->createNewPage<SchemaPage>();
        pageIDs.push_back(schemaPage.getPageID());
    }

    // Verify all pageIDs are unique
    std::sort(pageIDs.begin(), pageIDs.end());
    for (int i = 1; i < pageIDs.size(); ++i)
        ASSERT_NE(pageIDs[i], pageIDs[i - 1]);

    ASSERT_THROW(pager->createNewPage<SchemaPage>(), std::runtime_error);
}


TEST_F(PagerTest, RepeatedFreeAndReallocateSamePage) {
    for (int i = 0; i < (FSMPage::getBlocksInPage() * 2) * cts::MAX_FSMPAGES; ++i) {
        auto& page = pager->createNewPage<SchemaPage>();
        u32 pageID = page.getPageID();
        pager->freePage(pageID);
    }
}

TEST_F(PagerTest, FreeAllPagesAndReallocate) {
    std::vector<u32> pageIDs;

    for (int i = 0; i < (FSMPage::getBlocksInPage() - 1) * cts::MAX_FSMPAGES; ++i) {
        auto& schemaPage = pager->createNewPage<SchemaPage>();
        pageIDs.push_back(schemaPage.getPageID());
    }

    for (int pageID : pageIDs) {
        pager->freePage(pageID);
    }

    for (int i = 0; i < (FSMPage::getBlocksInPage() - 1) * cts::MAX_FSMPAGES; ++i) {
        ASSERT_NO_THROW(pager->createNewPage<SchemaPage>());
    }
    ASSERT_THROW(pager->createNewPage<SchemaPage>(), std::runtime_error);
}

TEST_F(PagerTest, GetPageExitsForUnallocatedPage) {
    ASSERT_DEATH(pager->getPage<SchemaPage>(0), "");
    ASSERT_DEATH(pager->getPage<SchemaPage>(1), "");
    ASSERT_DEATH(pager->getPage<SchemaPage>(100), "");
}


TEST_F(PagerTest, FreePagesInReverseOrder) {
    std::vector<u32> pageIDs;
    for (int i = 0; i < 10; ++i) {
        auto& schemaPage = pager->createNewPage<SchemaPage>();
        pageIDs.push_back(schemaPage.getPageID());
    }
    for (auto it = pageIDs.rbegin(); it != pageIDs.rend(); ++it) {
        pager->freePage(*it);
    }
}

TEST_F(PagerTest, SchemaPageNumTablesPersistsAfterDestruction) {
    auto& schemaPage = pager->createNewPage<SchemaPage>();
    u32 pageID = schemaPage.getPageID();
    schemaPage.addTable("Users", 5);
    schemaPage.addTable("Orders", 10);
    u8 expectedTableCount = schemaPage.getNumTables();

    resetEnv();
    initEnv();

    auto& reloadedSchemaPage = pager->getPage<SchemaPage>(pageID);
    ASSERT_EQ(reloadedSchemaPage.getNumTables(), expectedTableCount);
}

TEST_F(PagerTest, SchemaPageRemainsEmptyAfterRemovingAllTables) {
    auto& schemaPage = pager->createNewPage<SchemaPage>();
    u32 pageID = schemaPage.getPageID();
    schemaPage.addTable("Users", 5);
    schemaPage.addTable("Orders", 10);
    schemaPage.removeTable("Users");
    schemaPage.removeTable("Orders");
    ASSERT_EQ(schemaPage.getNumTables(), 0);

    resetEnv();
    initEnv();

    auto& reloadedSchemaPage = pager->getPage<SchemaPage>(pageID);
    ASSERT_EQ(reloadedSchemaPage.getNumTables(), 0);
    ASSERT_TRUE(reloadedSchemaPage.getTables().empty());
}