//
// Created by Kylan Chen on 10/13/24.
//

#include <iostream>
#include "Schema.hpp"
#include "Pager.hpp"
#include "backend/HeapFileManager.hpp"

int main () {
    Pager pager;
    HeapFileManager hfm(pager);
    Schema sc(pager, hfm);
    sc.createTable("table 1", {3, 'a', 'a', true});
    return 0;
}