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

using namespace backend;

struct PagerTest : testing::Test {
    void SetUp() override {
        std::ofstream file("testfile.db", std::ios::trunc);
        file.close();
    }
};

TEST_F(PagerTest, PagerThrowsWhenNotSchemaPage) {
    IOHandler ioHandler("testfile.db");
    Pager pager(ioHandler);
    pager.createNewPage<FSMPage>();
    ASSERT_THROW(pager.getPage<SchemaPage>(cts::pgid::SCHEMA_ID), std::bad_cast);
}

TEST_F(PagerTest, PagerThrowsWhenNoSchemaPage) {
    IOHandler ioHandler("testfile.db");
    Pager pager(ioHandler);
    ASSERT_THROW(pager.getPage<SchemaPage>(cts::pgid::SCHEMA_ID), std::invalid_argument);
}

TEST_F(PagerTest, PagerCreatesSchemaPage) {
    IOHandler ioHandler("testfile.db");
    Pager pager(ioHandler);
    pager.createNewPage<SchemaPage>();
    ASSERT_NO_THROW(pager.getPage<SchemaPage>(cts::pgid::SCHEMA_ID));
    ASSERT_EQ(pager.getPage<SchemaPage>(cts::pgid::SCHEMA_ID).getNumTables(), 0);
    std::unordered_map<string, u32> mp;
    ASSERT_EQ(pager.getPage<SchemaPage>(cts::pgid::SCHEMA_ID).getTables(), mp);
    ASSERT_EQ(pager.getPage<SchemaPage>(cts::pgid::SCHEMA_ID).getPageID(), cts::pgid::SCHEMA_ID);
}

TEST_F(PagerTest, SchemaPageWorksAfterSerializing) {
    u32 schemaPageID;
    {
        IOHandler ioHandler("testfile.db");
        Pager pager(ioHandler);
        schemaPageID = pager.createNewPage<SchemaPage>().getPageID();
        pager.getPage<SchemaPage>(schemaPageID).addTable("MyTable", 3);
        pager.getPage<SchemaPage>(schemaPageID).addTable("AnotherTable", 4);
    }

    IOHandler ioHandler("testfile.db");
    Pager pager(ioHandler);
    ASSERT_EQ(pager.getPage<SchemaPage>(schemaPageID).getNumTables(), 2);
    auto tables = pager.getPage<SchemaPage>(schemaPageID).getTables();
    std::unordered_map<string, u32> expected = {{"MyTable", 3}, {"AnotherTable", 4}};
    ASSERT_EQ(tables, expected);
}

TEST_F(PagerTest, CreateNewPageAllocatesUniquePageID) {
    u32 pageID1;
    u32 pageID2;

    {
        IOHandler ioHandler("testfile.db");
        Pager pager(ioHandler);
        auto& page1 = pager.createNewPage<SchemaPage>();
        auto& page2 = pager.createNewPage<SchemaPage>();

        ASSERT_NE(page1.getPageID(), page2.getPageID());
        pageID1 = page1.getPageID();
        pageID2 = page2.getPageID();
    }  // Destructing `Pager` forces a flush to disk

    // Verify that the page was correctly written
    std::ifstream file("testfile.db", std::ios::binary);
    ASSERT_TRUE(file.good());

    file.seekg(pageID1 * cts::PG_SZ, std::ios::beg);
    u8 storedPageType;
    file.read(reinterpret_cast<char*>(&storedPageType), sizeof(u8));

    ASSERT_EQ(storedPageType, cts::pg_type_id::SCHEMA_PAGE);

    file.seekg(pageID2 * cts::PG_SZ, std::ios::beg);
    file.read(reinterpret_cast<char*>(&storedPageType), sizeof(u8));

    ASSERT_EQ(storedPageType, cts::pg_type_id::SCHEMA_PAGE);
}

TEST_F(PagerTest, SchemaPageTablePersistenceAfterPagerDestruction) {
    IOHandler ioHandler("testfile.db");

    u32 pageID;
    std::unordered_map<std::string, u32> expectedTables;

    {
        Pager pager(ioHandler);
        auto& schemaPage = pager.createNewPage<SchemaPage>();
        pageID = schemaPage.getPageID();

        // Add tables to SchemaPage
        schemaPage.addTable("Users", 5);
        schemaPage.addTable("Orders", 10);

        // Store expected table mapping
        expectedTables = schemaPage.getTables();
    } // Pager destructed, writes should be flushed

    // Reconstruct pager and fetch the same SchemaPage
    Pager newPager(ioHandler);
    auto& reloadedSchemaPage = newPager.getPage<SchemaPage>(pageID);

    // Verify table data is correctly restored
    ASSERT_EQ(reloadedSchemaPage.getTables(), expectedTables);
}

TEST_F(PagerTest, GetPageReturnsSameDataOnRepeatedCalls) {
    IOHandler ioHandler("testfile.db");

    u32 pageID;
    std::unordered_map<string, u32> expectedTables;

    {
        Pager pager(ioHandler);
        auto& schemaPage = pager.createNewPage<SchemaPage>();
        pageID = schemaPage.getPageID();

        schemaPage.addTable("Users", 5);
        schemaPage.addTable("Orders", 10);

        expectedTables = schemaPage.getTables();

        // Get the page again and ensure it's still consistent
        auto& retrievedPage = pager.getPage<SchemaPage>(pageID);
        ASSERT_EQ(retrievedPage.getTables(), expectedTables);
    }
}

TEST_F(PagerTest, FreePageAllowsReallocation) {
    {
        IOHandler ioHandler("testfile.db");
        Pager pager(ioHandler);
        auto& schemaPage = pager.createNewPage<SchemaPage>();
        u32 freedPageID = schemaPage.getPageID();

        pager.freePage<SchemaPage>(freedPageID);

        // Allocate a new page, seeing if it will reuse the freed page
        auto& newSchemaPage = pager.createNewPage<SchemaPage>();
        u32 reallocatedPageID = newSchemaPage.getPageID();

        ASSERT_EQ(freedPageID, reallocatedPageID);
    }
}

TEST_F(PagerTest, GetPageThrowsForFreedPage) {
    IOHandler ioHandler("testfile.db");

    u32 pageID;

    {
        Pager pager(ioHandler);
        auto& schemaPage = pager.createNewPage<SchemaPage>();
        pageID = schemaPage.getPageID();

        pager.freePage<SchemaPage>(pageID);

        ASSERT_THROW(pager.getPage<SchemaPage>(pageID), std::invalid_argument);
    }
}

TEST_F(PagerTest, ThrowsIfExceedsMaxBlocks) {
    IOHandler ioHandler("testfile.db");
    Vec<u32> pageIDs;

    {
        Pager pager(ioHandler);

        // subtract a hundred or so pages for bitmaps
        for (int i = 0; i < cts::MAX_BLOCKS - (cts::MAX_BLOCKS / FSMPage::getBlocksInPage()) - 1; ++i) {
            auto& schemaPage = pager.createNewPage<SchemaPage>();
            pageIDs.push_back(schemaPage.getPageID());
        }

        // Ensure all pages have unique IDs
        std::sort(pageIDs.begin(), pageIDs.end());
        auto last = std::unique(pageIDs.begin(), pageIDs.end());
        ASSERT_EQ(last, pageIDs.end());

        ASSERT_THROW(pager.createNewPage<SchemaPage>(), std::runtime_error);
    }
}

TEST_F(PagerTest, RepeatedFreeAndReallocateSamePage) {
    IOHandler ioHandler("testfile.db");

    u32 pageID;

    {
        Pager pager(ioHandler);

        for (int i = 0; i < cts::MAX_BLOCKS * 3; ++i) {
            auto& schemaPage = pager.createNewPage<SchemaPage>();
            pageID = schemaPage.getPageID();

            pager.freePage<SchemaPage>(pageID);
        }
    }
}

TEST_F(PagerTest, FreeEveryOtherPageThenReallocate) {
    IOHandler ioHandler("testfile.db");
    Vec<u32> allocatedPages;

    {
        Pager pager(ioHandler);

        for (int i = 0; i < 50; ++i) {
            auto& schemaPage = pager.createNewPage<SchemaPage>();
            allocatedPages.push_back(schemaPage.getPageID());
        }

        // Free every other page
        for (int i = 0; i < allocatedPages.size(); i += 2) {
            pager.freePage<SchemaPage>(allocatedPages[i]);
        }

        // Allocate new pages and check if freed IDs are reused
        for (int i = 0; i < allocatedPages.size() / 2; ++i) {
            auto& newSchemaPage = pager.createNewPage<SchemaPage>();
            ASSERT_NE(std::find(allocatedPages.begin(), allocatedPages.end(), newSchemaPage.getPageID()), allocatedPages.end());
        }
    }
}

TEST_F(PagerTest, FreeAllPagesAndReallocate) {
    IOHandler ioHandler("testfile.db");
    Vec<u32> firstBatch, secondBatch;

    {
        Pager pager(ioHandler);

        // Allocate a bunch of pages
        for (int i = 0; i < std::min(FSMPage::getBlocksInPage() + 500, cts::MAX_BLOCKS - 100); ++i) {
            auto& schemaPage = pager.createNewPage<SchemaPage>();
            firstBatch.push_back(schemaPage.getPageID());
        }

        // Free all of them
        for (int pageID : firstBatch) {
            pager.freePage<SchemaPage>(pageID);
        }

        // Allocate again and ensure IDs are still valid
        for (int i = 0; i < std::min(FSMPage::getBlocksInPage() + 500, cts::MAX_BLOCKS - 100); ++i) {
            auto& schemaPage = pager.createNewPage<SchemaPage>();
            secondBatch.push_back(schemaPage.getPageID());
        }

        // The second batch should contain the same IDs as the first
        ASSERT_EQ(firstBatch, secondBatch);
    }
}

TEST_F(PagerTest, GetPageThrowsForUnallocatedPage) {
    IOHandler ioHandler("testfile.db");

    Pager pager(ioHandler);

    ASSERT_THROW(pager.getPage<SchemaPage>(100), std::invalid_argument);
}

TEST_F(PagerTest, FreePagesInReverseOrder) {
    IOHandler ioHandler("testfile.db");
    Vec<u32> pageIDs;

    {
        Pager pager(ioHandler);
        for (int i = 0; i < 10; ++i) {
            auto& schemaPage = pager.createNewPage<SchemaPage>();
            pageIDs.push_back(schemaPage.getPageID());
        }

        // Free in reverse order
        for (auto it = pageIDs.rbegin(); it != pageIDs.rend(); ++it) {
            pager.freePage<SchemaPage>(*it);
        }
    }
}

TEST_F(PagerTest, SchemaPageNumTablesPersistsAfterDestruction) {
    IOHandler ioHandler("testfile.db");

    u32 pageID;
    u8 expectedTableCount;

    {
        Pager pager(ioHandler);
        auto& schemaPage = pager.createNewPage<SchemaPage>();
        pageID = schemaPage.getPageID();

        schemaPage.addTable("Users", 5);
        schemaPage.addTable("Orders", 10);

        expectedTableCount = schemaPage.getNumTables();
    }

    Pager newPager(ioHandler);
    auto& reloadedSchemaPage = newPager.getPage<SchemaPage>(pageID);

    ASSERT_EQ(reloadedSchemaPage.getNumTables(), expectedTableCount);
}

TEST_F(PagerTest, SchemaPageRemainsEmptyAfterRemovingAllTables) {
    IOHandler ioHandler("testfile.db");
    u32 pageID;

    {
        Pager pager(ioHandler);
        auto& schemaPage = pager.createNewPage<SchemaPage>();
        pageID = schemaPage.getPageID();

        schemaPage.addTable("Users", 5);
        schemaPage.addTable("Orders", 10);

        schemaPage.removeTable("Users");
        schemaPage.removeTable("Orders");

        ASSERT_EQ(schemaPage.getNumTables(), 0);
    }

    Pager newPager(ioHandler);
    auto& reloadedSchemaPage = newPager.getPage<SchemaPage>(pageID);

    ASSERT_EQ(reloadedSchemaPage.getNumTables(), 0);
    ASSERT_TRUE(reloadedSchemaPage.getTables().empty());
}

TEST_F(PagerTest, SchemaPageHandlesLargeStringNames) {
    IOHandler ioHandler("testfile.db");
    u32 pageID;
    std::string longTableName(31, 'X');  // Max allowed name length

    {
        Pager pager(ioHandler);
        auto& schemaPage = pager.createNewPage<SchemaPage>();
        pageID = schemaPage.getPageID();

        schemaPage.addTable(longTableName, 5);
    }

    Pager newPager(ioHandler);
    auto& reloadedSchemaPage = newPager.getPage<SchemaPage>(pageID);

    auto tables = reloadedSchemaPage.getTables();
    ASSERT_TRUE(tables.find(longTableName) != tables.end());
}

TEST_F(PagerTest, SimpleTablePageWorks) {
    IOHandler ioHandler("testfile.db");

    Vec<Vari> types = {string(), string(), int(), double(), float(), int()};
    Pager pager(ioHandler);
    u32 pageID = pager.createNewPage<TablePage>(types, 5).getPageID();
    ASSERT_EQ(pager.getPage<TablePage>(pageID).getTypes(), types);
    ASSERT_EQ(pager.getPage<TablePage>(pageID).getPageID(), pageID);
    ASSERT_EQ(pager.getPage<TablePage>(pageID).getNumTuples(), 0);
    ASSERT_EQ(pager.getPage<TablePage>(pageID).getBtreePageID(), 5);

    pager.getPage<TablePage>(pageID).setBtreePageID(100);
    ASSERT_EQ(pager.getPage<TablePage>(pageID).getBtreePageID(), 100);
}

TEST_F(PagerTest, TablePageWorksWithAddAndRemove) {
    u32 pageID;
    Vec<Vari> types = {string(), string(), int(), double(), float(), int()};
    {
        IOHandler ioHandler("testfile.db");
        Pager pager(ioHandler);

        pageID = pager.createNewPage<TablePage>(types, 5).getPageID();
        pager.getPage<TablePage>(pageID).addTuple();
        pager.getPage<TablePage>(pageID).addTuple();
        pager.getPage<TablePage>(pageID).addTuple();
    }

    IOHandler ioHandler("testfile.db");
    Pager pager(ioHandler);

    ASSERT_EQ(pager.getPage<TablePage>(pageID).getNumTuples(), 3);
    ASSERT_EQ(pager.getPage<TablePage>(pageID).getPageID(), pageID);
    ASSERT_EQ(pager.getPage<TablePage>(pageID).getBtreePageID(), 5);
    ASSERT_EQ(pager.getPage<TablePage>(pageID).getTypes(), types);
}