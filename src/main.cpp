//
// Created by Kylan Chen on 10/13/24.
//
#include <iostream>

#include "Pager.hpp"

using std::cout;
using std::endl;

int main() {
    FSMPage page(0);
    cout << page.getSpaceLeft() << endl;
    page.allocBit(2);
    cout << page.getSpaceLeft() << endl;

    ByteVec vec(cts::PG_SZ);
    page.toBytes(vec);
    FSMPage page2(0, vec);
    cout << page2.getSpaceLeft() << endl;

    for (int i = 0; i < 32575; i++) {
        int next = page2.findNextFree();
        page2.allocBit(next);
    }

    cout << page2.getSpaceLeft() << endl;


    return 0;
}