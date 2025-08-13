# KNDB

A lightweight, file-based relational database management system (RDBMS) built in modern C++.

## Overview

KNDB is a SQL-based database engine that handles the core relational database operations you'd expect. It's built with C++20 and provides a clean, simple API for basic database work while keeping your data safe and persistent.

## Features

- **File-based Storage**: Saves your database to disk with a smart page-based system
- **Flexible Schema**: Works with different data types like integers, floats, doubles, booleans, strings, and characters
- **CRUD Operations**: Full Create, Read, Update, Delete support for your data
- **B-tree Indexing**: Allows for indexing using B-trees for fast look-up and insertions
- **Page Cache**: Smart page caching that increases general performance

## Requirements

- **Compiler**: Any C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- **Build System**: CMake 3.26 or newer, Ninja build tool
- **Platform**: Works on Linux, macOS, and Windows

## Building

**Build:**
```bash
mkdir build && cd build
cmake -G Ninja ..
ninja
```

**Run:**
```bash
./src/KNDB
./test/backend_test
```

## Usage

```cpp
// Basic database operations
#include "backend/StorageEngine.hpp"
#include "backend/Pager.hpp"
#include "backend/PageCache.hpp"
#include "backend/IOHandler.hpp"

// Set up the components
IOHandler ioHandler("database.db");
PageCache pageCache(ioHandler, cacheSize);
FreeSpaceMap freeSpaceMap(pageCache);
Pager pager(freeSpaceMap, ioHandler, pageCache);

// Create your storage engine
StorageEngine storage_engine(pager, schemaPageId);

// Make a table with your column types
storage_engine.createTable("Students", {int(), int(), double()});

// Add some data
storage_engine.insertTuple("Students", {1, 2, 3.0});

// Check how many records you have
int tupleCount = storage_engine.getNumTuples("Students");
```

## Development Status

This project is still growing! Right now it handles the storage engine basics with CRUD operations, but the plan is to build a full SQL language with a query processor on top.
