//
// Created by kylan on 7/20/2025.
//

#include <gtest/gtest.h>
#include <fstream>
#include <ranges>

#include "PageCache.hpp"
#include "SchemaPage.hpp"
#include "TablePage.hpp"
#include "constants.hpp"
#include "IOHandler.hpp"

using namespace backend;

struct PageCacheTest : testing::Test {
    std::unique_ptr<IOHandler> ioHandler;
    std::unique_ptr<PageCache> cache;

    const std::string kTestFile = "testfile.db";

    void init() {
        ioHandler = std::make_unique<IOHandler>(kTestFile);
        cache = std::make_unique<PageCache>(*ioHandler, 100);
    }

    void reset() {
        cache.reset();
        ioHandler.reset();
    }

    void SetUp() override {
        std::remove(kTestFile.c_str());
        init();
    }

    void TearDown() override {
        reset();
        std::remove(kTestFile.c_str());
    }
};

TEST_F(PageCacheTest, InsertAndRetrieveSamePage) {
    auto pageID = ioHandler->createNewBlock();
    auto page = std::make_unique<SchemaPage>(pageID);
    page->addTable("Users", 1);
    cache->insertPage(std::move(page));
    auto& retrieved = cache->retrievePage<SchemaPage>(0);
    ASSERT_EQ(retrieved.getTables().at("Users"), 1);
}

TEST_F(PageCacheTest, DestructorWritesPages) {
    auto pageID = ioHandler->createNewBlock();
    auto page = std::make_unique<SchemaPage>(pageID);
    page->addTable("Users", 1);
    cache->insertPage(std::move(page));

    reset();
    init();

    auto& tablePage = cache->retrievePage<SchemaPage>(pageID);
    ASSERT_EQ(tablePage.getTables().at("Users"), 1);
}

TEST_F(PageCacheTest, RetrievePageThrowsForUnallocatedPage) {
    ASSERT_THROW(cache->retrievePage<SchemaPage>(3), std::runtime_error);
}

TEST_F(PageCacheTest, RetrievePageTwiceReturnsSameInstance) {
    auto pageID = ioHandler->createNewBlock();
    auto page = std::make_unique<SchemaPage>(pageID);
    page->addTable("A", 1);
    cache->insertPage(std::move(page));

    auto& first = cache->retrievePage<SchemaPage>(pageID);
    auto& second = cache->retrievePage<SchemaPage>(pageID);
    ASSERT_EQ(first.getTables(), second.getTables());
}

TEST_F(PageCacheTest, RetrieveInvalidCastThrows) {
    auto pageID = ioHandler->createNewBlock();
    auto page = std::make_unique<SchemaPage>(pageID);
    cache->insertPage(std::move(page));

    ASSERT_THROW({
        cache->retrievePage<TablePage>(pageID);
    }, std::bad_cast);
}

TEST_F(PageCacheTest, InsertAndRetrieveForManyPages) {
    std::unordered_map<pgid_t, int> map;
    for (int i = 0; i < 10000; i++) {
        auto pageID = ioHandler->createNewBlock();
        auto page = std::make_unique<SchemaPage>(pageID);
        page->addTable("Table", i);
        map[pageID] = i;
        cache->insertPage(std::move(page));
    }

    for (const auto& [pgid, val]: map) {
        ASSERT_EQ(cache->retrievePage<SchemaPage>(pgid).getTables().at("Table"), val);
    }
}