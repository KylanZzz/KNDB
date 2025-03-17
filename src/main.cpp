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
    std::ofstream file(backend::cts::DATABASE_NAME, std::ios::trunc);
    file.close();

    backend::IOHandler ioHandler(backend::cts::DATABASE_NAME);
    backend::Pager pager(ioHandler);

    // Check if db file exists. If not, create one
    try {
        pager.getPage<backend::SchemaPage>(backend::cts::pgid::SCHEMA_ID);
    } catch (std::invalid_argument& e) {
        DEBUG(e.what());
        DEBUG("Creating schema page for database");
        if (pager.createNewPage<backend::SchemaPage>().getPageID() != backend::cts::pgid::SCHEMA_ID)
            throw std::runtime_error("Error while creating schema page");
    } catch (std::exception& e) {
        DEBUG(e.what());
    }



    return 0;
}