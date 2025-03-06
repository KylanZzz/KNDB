//
// Created by Kylan Chen on 10/13/24.
//
#include <iostream>
#include <fstream>

#include "Pager.hpp"
#include "SchemaPage.hpp"
#include "FSMPage.hpp"
#include "utility.hpp"
#include "Schema.hpp"

using std::cout;
using std::endl;

#define DEBUG(msg) cout << "DEBUG: " << msg << endl

int main() {
    std::ofstream file(cts::DATABASE_NAME, std::ios::trunc);
    file.close();

    IOHandler ioHandler(cts::DATABASE_NAME);
    Pager pager(ioHandler);

    // Check if db file exists. If not, create one
    try {
        pager.getPage<SchemaPage>(cts::SCHEMA_PAGE_NO);
    } catch (std::invalid_argument& e) {
        DEBUG(e.what());
        DEBUG("Creating schema page for database");
        if (pager.createNewPage<SchemaPage>().getPageID() != cts::SCHEMA_PAGE_NO)
            throw std::runtime_error("Error while creating schema page");
    } catch (std::exception& e) {
        DEBUG(e.what());
    }

    return 0;
}