//
// Created by Kylan Chen on 10/13/24.
//
#include <iostream>
#include <fstream>

#include "Pager.hpp"
#include "SchemaPage.hpp"
#include "utility.hpp"
#include "StorageEngine.hpp"

using std::cout;
using std::endl;

#define DEBUG(msg) cout << "DEBUG: " << msg << endl

int main() {
    std::ofstream file(std::string(backend::cts::DATABASE_NAME), std::ios::trunc);
    file.close();

    backend::IOHandler ioHandler(backend::cts::DATABASE_NAME);
    backend::Pager pager(ioHandler);

    // Check if db file exists. If not, create one
    if (ioHandler.getNumBlocks() <= backend::cts::pgid::SCHEMA_ID) {
        DEBUG("Created DB page");
        if (pager.createNewPage<backend::SchemaPage>().getPageID() != backend::cts::pgid::SCHEMA_ID)
            throw std::runtime_error("Error while creating schema page");
    }
    DEBUG("number blocks: " << ioHandler.getNumBlocks());

    backend::StorageEngine storage_engine(pager, backend::cts::pgid::SCHEMA_ID);

    return 0;
}