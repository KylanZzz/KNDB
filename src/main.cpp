//
// Created by Kylan Chen on 10/13/24.
//
#include <iostream>
#include <fstream>

#include "Pager.hpp"
#include "SchemaPage.hpp"
#include "FSMPage.hpp"
#include "utility.hpp"

using std::cout;
using std::endl;

#define DEBUG(msg) cout << "DEBUG: " << msg << endl

int main() {
    std::ofstream file("kylan_test_file.db", std::ios::trunc);
    file.close();

    IOHandler ioHandler("kylan_test_file.db");
    DEBUG(ioHandler.getNumBlocks());
    Pager pager(ioHandler);
    DEBUG(ioHandler.getNumBlocks());

    auto sPage = pager.createNewPage<FSMPage>();

    DEBUG(ioHandler.getNumBlocks());

    vector<variants> vec{int(), char(), string(), string(), int(), double()};
    DEBUG(db_sizeof(vec));

    return 0;
}