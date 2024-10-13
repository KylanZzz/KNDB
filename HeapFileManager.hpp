//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_HEAPFILEMANAGER_HPP
#define KNDB_HEAPFILEMANAGER_HPP

#include <Pager.hpp>
#include <iostream>

class HeapFileManager {
public:
    HeapFileManager(Pager& pgr): pager(pgr){
        std::cout << "constructed hfm" << std::endl;
    };
private:
    Pager pager;
};


#endif //KNDB_HEAPFILEMANAGER_HPP
