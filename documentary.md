# SQLid Database System

## System Commands 

SHOW - display 'database1 [ table1 ... tableN ] ...'

CREATE DATABASE DATABASE_NAME

CREATE TABLE FOR DATABASE_NAME TABLE_NAME COL1_NAME COL1_TYPE ... COLN_NAME COLN_TYPE

ATTACH TO DATABASE_NAME - connect to some database

BACKUP DATABASE_NAME AS DATABASE_BACKUP_NAME

RESTORE DATABASE_BACKUP_NAME AS DATABASE_NAME

DROP DATABASE_NAME - delete database

DISCONNECT - turn off connection

LEAVE - dettach from current database

HELP - guess what 

## SQLid SYNTAX

### CONDITIONS

... WHERE id == x - direct indexing, O(1)

... WHERE id >= x AND id <= y - range indexing, O(y-x)

... WHERE col1 == x AND col2 > y AND col3 < z OR col1 != v - O(n)

... WHERE some_cond LIMIT L - O(n)

#### Valid examples:

... WHERE id == 100

... WHERE id >= 10 AND id <= 80 AND name < Numa AND age > 20 OR SALARY == 1.1 LIMIT 10

### Queries

SELECT col1 ... col2 FROM table_name WHERE ... - '*' is supported

INSERT INTO table_name val1 ... valN - you can also insert multiple data using one insert

UPDATE table_name SET col1 = val1 ... colN = valN WHERE ...

DELETE FROM table_name WHERE ... - 'WHERE' is required

### Order by for SELECT

ORDER BY MUST CONTAIN THE SAME COLUMNS TO SORT AS SELECT'S OUTPUT

Example: table [name, age, salary]
SELECT name FROM table WHERE cond LIMIT N ORDER BY age DESC - incorrect
SELECT * FROM table WHERE cond LIMIT N ORDER BY age DESC - correct  
SELECT * FROM table WHERE cond LIMIT N ORDER BY firstname DESC - incorrect

> DESC and ASC supported as usual.
ORDER BY must be at the end of query.

### Supported Types

Integers: INT32, INT64

Floats: FLOAT, DOUBLE

STRING[N] - Fixed-size string (N characters)

TEXT - char array with the static size 128

Boolean: BOOL