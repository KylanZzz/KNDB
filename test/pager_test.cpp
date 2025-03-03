//
// Created by Kylan Chen on 3/2/25.
//

#include <gtest/gtest.h>
#include <fstream>

#include "Pager.hpp"
#include "IOHandler.hpp"
#include "SchemaPage.hpp"
#include "constants.hpp"
#include "utility.hpp"


struct PagerTest : testing::Test {
    void SetUp() override {
        std::ofstream file("testfile.db", std::ios::trunc);
        file.close();
    }
};

TEST_F(PagerTest, ConstructPagerCreatesSchemaPage) {
    IOHandler ioHandler("testfile.db");
    Pager pager(ioHandler);
    ASSERT_NO_THROW(pager.getPage<SchemaPage>(cts::SCHEMA_PAGE_NO));
}

TEST_F(PagerTest, CreateNewPageAllocatesUniquePageID) {
    IOHandler ioHandler("testfile.db");
    size_t pageID1;
    size_t pageID2;

    {
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
    size_t storedPageType;
    file.read(reinterpret_cast<char*>(&storedPageType), sizeof(size_t));

    ASSERT_EQ(storedPageType, get_page_type_id<SchemaPage>());

    file.seekg(pageID2 * cts::PG_SZ, std::ios::beg);
    file.read(reinterpret_cast<char*>(&storedPageType), sizeof(size_t));

    ASSERT_EQ(storedPageType, get_page_type_id<SchemaPage>());
}

TEST_F(PagerTest, SchemaPageTablePersistenceAfterPagerDestruction) {
    IOHandler ioHandler("testfile.db");

    size_t pageID;
    std::unordered_map<std::string, size_t> expectedTables;

    {
        Pager pager(ioHandler);
        auto& schemaPage = pager.createNewPage<SchemaPage>();
        pageID = schemaPage.getPageID();

        // Add tables to SchemaPage
        schemaPage.addTable("Users", {int(), std::string()}, 5);
        schemaPage.addTable("Orders", {bool(), char()}, 10);

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

    size_t pageID;
    std::unordered_map<string, size_t> expectedTables;

    {
        Pager pager(ioHandler);
        auto& schemaPage = pager.createNewPage<SchemaPage>();
        pageID = schemaPage.getPageID();

        schemaPage.addTable("Users", {int(), std::string()}, 5);
        schemaPage.addTable("Orders", {bool(), char()}, 10);

        expectedTables = schemaPage.getTables();

        // Get the page again and ensure it's still consistent
        auto& retrievedPage = pager.getPage<SchemaPage>(pageID);
        ASSERT_EQ(retrievedPage.getTables(), expectedTables);
    }
}

TEST_F(PagerTest, FreePageAllowsReallocation) {
    IOHandler ioHandler("testfile.db");

    size_t freedPageID;
    size_t reallocatedPageID;

    {
        Pager pager(ioHandler);
        auto& schemaPage = pager.createNewPage<SchemaPage>();
        freedPageID = schemaPage.getPageID();

        pager.freePage<SchemaPage>(freedPageID);

        // Allocate a new page, seeing if it will reuse the freed page
        auto& newSchemaPage = pager.createNewPage<SchemaPage>();
        reallocatedPageID = newSchemaPage.getPageID();

        ASSERT_EQ(freedPageID, reallocatedPageID);
    }
}

TEST_F(PagerTest, GetPageThrowsForFreedPage) {
    IOHandler ioHandler("testfile.db");

    size_t pageID;

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
    std::vector<size_t> pageIDs;

    {
        Pager pager(ioHandler);

        // subtract a hundred or so pages for bitmaps
        for (size_t i = 0; i < cts::MAX_BLOCKS - (cts::MAX_BLOCKS / FSMPage::getBlocksInPage()) - 2; ++i) {
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

    size_t pageID;

    {
        Pager pager(ioHandler);

        for (size_t i = 0; i < cts::MAX_BLOCKS * 3; ++i) {
            auto& schemaPage = pager.createNewPage<SchemaPage>();
            pageID = schemaPage.getPageID();

            pager.freePage<SchemaPage>(pageID);
        }
    }
}

TEST_F(PagerTest, FreeEveryOtherPageThenReallocate) {
    IOHandler ioHandler("testfile.db");
    std::vector<size_t> allocatedPages;

    {
        Pager pager(ioHandler);

        for (size_t i = 0; i < 50; ++i) {
            auto& schemaPage = pager.createNewPage<SchemaPage>();
            allocatedPages.push_back(schemaPage.getPageID());
        }

        // Free every other page
        for (size_t i = 0; i < allocatedPages.size(); i += 2) {
            pager.freePage<SchemaPage>(allocatedPages[i]);
        }

        // Allocate new pages and check if freed IDs are reused
        for (size_t i = 0; i < allocatedPages.size() / 2; ++i) {
            auto& newSchemaPage = pager.createNewPage<SchemaPage>();
            ASSERT_NE(std::find(allocatedPages.begin(), allocatedPages.end(), newSchemaPage.getPageID()), allocatedPages.end());
        }
    }
}

TEST_F(PagerTest, FreeAllPagesAndReallocate) {
    IOHandler ioHandler("testfile.db");
    std::vector<size_t> firstBatch, secondBatch;

    {
        Pager pager(ioHandler);

        // Allocate a bunch of pages
        for (size_t i = 0; i < FSMPage::getBlocksInPage() + 500; ++i) {
            auto& schemaPage = pager.createNewPage<SchemaPage>();
            firstBatch.push_back(schemaPage.getPageID());
        }

        // Free all of them
        for (size_t pageID : firstBatch) {
            pager.freePage<SchemaPage>(pageID);
        }

        // Allocate again and ensure IDs are still valid
        for (size_t i = 0; i < FSMPage::getBlocksInPage() + 500; ++i) {
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
    std::vector<size_t> pageIDs;

    {
        Pager pager(ioHandler);
        for (size_t i = 0; i < 10; ++i) {
            auto& schemaPage = pager.createNewPage<SchemaPage>();
            pageIDs.push_back(schemaPage.getPageID());
        }

        // Free in reverse order
        for (auto it = pageIDs.rbegin(); it != pageIDs.rend(); ++it) {
            pager.freePage<SchemaPage>(*it);
        }
    }
}

TEST_F(PagerTest, SchemaPageTableTypesPersistAfterDestruction) {
    IOHandler ioHandler("testfile.db");

    size_t pageID;
    vector<variants> expectedTypes;
    vector<variants> expectedTypes2;

    {
        Pager pager(ioHandler);
        auto& schemaPage = pager.createNewPage<SchemaPage>();
        pageID = schemaPage.getPageID();

        // Add table with specific types
        expectedTypes = {int(), bool(), std::string()};
        expectedTypes2 = {int(), bool(), float(), string(), string()};
        schemaPage.addTable("Users", expectedTypes, 5);
        schemaPage.addTable("Players", expectedTypes2, 5);
    }

    Pager newPager(ioHandler);
    auto& reloadedSchemaPage = newPager.getPage<SchemaPage>(pageID);

    ASSERT_EQ(reloadedSchemaPage.getTableTypes("Users"), expectedTypes);
    ASSERT_EQ(reloadedSchemaPage.getTableTypes("Players"), expectedTypes2);
}

TEST_F(PagerTest, SchemaPageNumTablesPersistsAfterDestruction) {
    IOHandler ioHandler("testfile.db");

    size_t pageID;
    size_t expectedTableCount;

    {
        Pager pager(ioHandler);
        auto& schemaPage = pager.createNewPage<SchemaPage>();
        pageID = schemaPage.getPageID();

        schemaPage.addTable("Users", {int(), std::string()}, 5);
        schemaPage.addTable("Orders", {bool(), char()}, 10);

        expectedTableCount = schemaPage.getNumTables();
    }

    Pager newPager(ioHandler);
    auto& reloadedSchemaPage = newPager.getPage<SchemaPage>(pageID);

    ASSERT_EQ(reloadedSchemaPage.getNumTables(), expectedTableCount);
}

TEST_F(PagerTest, SchemaPageRemainsEmptyAfterRemovingAllTables) {
    IOHandler ioHandler("testfile.db");
    size_t pageID;

    {
        Pager pager(ioHandler);
        auto& schemaPage = pager.createNewPage<SchemaPage>();
        pageID = schemaPage.getPageID();

        schemaPage.addTable("Users", {int(), std::string()}, 5);
        schemaPage.addTable("Orders", {bool(), char()}, 10);

        schemaPage.removeTable("Users");
        schemaPage.removeTable("Orders");

        ASSERT_EQ(schemaPage.getNumTables(), 0);
    }

    Pager newPager(ioHandler);
    auto& reloadedSchemaPage = newPager.getPage<SchemaPage>(pageID);

    ASSERT_EQ(reloadedSchemaPage.getNumTables(), 0);
    ASSERT_TRUE(reloadedSchemaPage.getTables().empty());
}

TEST_F(PagerTest, SchemaPageModificationsPersistAfterDestruction) {
    IOHandler ioHandler("testfile.db");
    size_t pageID;

    {
        Pager pager(ioHandler);
        auto& schemaPage = pager.createNewPage<SchemaPage>();
        pageID = schemaPage.getPageID();

        schemaPage.addTable("Users", {int(), std::string()}, 5);
    }

    // Reconstruct Pager and modify the existing table
    {
        Pager pager(ioHandler);
        auto& schemaPage = pager.getPage<SchemaPage>(pageID);

        schemaPage.removeTable("Users");
        schemaPage.addTable("Users", {bool(), float()}, 5);
    }

    // Reconstruct again to verify changes persisted
    Pager newPager(ioHandler);
    auto& reloadedSchemaPage = newPager.getPage<SchemaPage>(pageID);

    auto types = reloadedSchemaPage.getTableTypes("Users");
    ASSERT_EQ(types.size(), 2);
    ASSERT_TRUE(std::holds_alternative<bool>(types[0]));
    ASSERT_TRUE(std::holds_alternative<float>(types[1]));
}

TEST_F(PagerTest, SchemaPageHandlesLargeStringNames) {
    IOHandler ioHandler("testfile.db");
    size_t pageID;
    std::string longTableName(31, 'X');  // Max allowed name length

    {
        Pager pager(ioHandler);
        auto& schemaPage = pager.createNewPage<SchemaPage>();
        pageID = schemaPage.getPageID();

        schemaPage.addTable(longTableName, {std::string(), std::string()}, 5);
    }

    Pager newPager(ioHandler);
    auto& reloadedSchemaPage = newPager.getPage<SchemaPage>(pageID);

    auto tables = reloadedSchemaPage.getTables();
    ASSERT_TRUE(tables.find(longTableName) != tables.end());
}
