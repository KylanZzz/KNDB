//
// Created by Kylan Chen on 10/13/24.
//
#include <iostream>
#include <fstream>
#include <filesystem>

#include "Pager.hpp"
#include "SchemaPage.hpp"
#include "utility.hpp"
#include "StorageEngine.hpp"

using std::cout;
using std::endl;

#define DEBUG(msg) cout << "DEBUG: " << msg << endl

int main() {
    std::remove(std::string(backend::cts::DATABASE_NAME).c_str());

    backend::IOHandler ioHandler(backend::cts::DATABASE_NAME);
    backend::PageCache pageCache(ioHandler);
    backend::FreeSpaceMap freeSpaceMap(pageCache);
    backend::Pager pager(freeSpaceMap, ioHandler, pageCache);

    // Check if db file exists. If not, create one
    if (!std::filesystem::exists(backend::cts::DATABASE_NAME)) {
        if (pager.createNewPage<backend::SchemaPage>().getPageID() != backend::cts::SCHEMA_ID)
            throw std::runtime_error("Error while creating schema page");
        DEBUG("Created DB page");
    }

    backend::StorageEngine storage_engine(pager, backend::cts::SCHEMA_ID);
    storage_engine.createTable("Students", {std::string(), int(), double()});
    storage_engine.insertTuple("Students", {std::string("Kylan"), 20, 100.0});
    auto tuple = storage_engine.getTuple("Students", "Kylan");
    for (const auto& variant: tuple) {
        std::visit([](auto&& val) {
            std::cout << val << std::endl;
        }, variant);
    }

    return 0;
}