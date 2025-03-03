# KNDB
RDBMS

Project Goals (preliminary):
- Load and persist a database to a file
- Create tables of varying length and types
  - Types supported include: int, float, double, bool, string, char
- Drop tables 
- CRUD operations on rows
  - Create tuples
    - Create a single tuple. First field will automatically be primary key.
  - Read tuples
    - Read a single tuple based on the primary key 
  - Update tuples
    - Update a single tuple, once again searching based on only on primary key
  - Delete tuples
    - Delete any single tuple based on primary key