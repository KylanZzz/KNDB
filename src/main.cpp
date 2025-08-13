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
    backend::PageCache pageCache(ioHandler, backend::cts::CACHE_SZ);
    backend::FreeSpaceMap freeSpaceMap(pageCache);
    backend::Pager pager(freeSpaceMap, ioHandler, pageCache);

    // Create DB file
    if (pager.createNewPage<backend::SchemaPage>().getPageID() != backend::cts::SCHEMA_ID)
        throw std::runtime_error("Error while creating schema page");
    DEBUG("Created DB page");

    backend::StorageEngine storage_engine(pager, backend::cts::SCHEMA_ID);

    // run some test queries on it.
    storage_engine.createTable("Students", {int(), int(), double()});
    for (int i = 0; i < 1000000; ++i) {
        ASSUME_S(storage_engine.insertTuple("Students", {i, i * 2, i * 3.0 / 0.5}), "Failed to insert tuple");
    }
    ASSUME_S(storage_engine.getNumTuples("Students") == 1000000, "There should be 1 million tuples");
    DEBUG("Passed Test");

    return 0;
}