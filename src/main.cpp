//
// Created by Kylan Chen on 10/13/24.
//
#include <iostream>
#include <fstream>

#include "Pager.hpp"
#include "SchemaPage.hpp"

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

    auto sPage = pager.createNewPage<SchemaPage>();

    DEBUG(ioHandler.getNumBlocks());

    return 0;
}