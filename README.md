# KNDB
RDBMS

Project Goals (preliminary):
- Load and persist a database to a file
- Create tables that contain tuples of varying length and types
  - Types supported so far include: [int, float, double, bool, string, char]
- Drop tables 
- CRUD operations on tuples
  - Create tuples
    - Create a single tuple at a time
    - First field will automatically be primary key
  - Read tuples
    - Read a single tuple at a time, searched on primary key
  - Update tuples
    - Update a single tuple at a time, searched on primary key
  - Delete tuples
    - Delete a single tuple at a time, searched on primary key