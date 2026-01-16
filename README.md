# SQLid Database Engine

File-based SQL database engine written in C++ from scratch

## Core Philosophy

SQLid builds on one fundamental principle: **ID is everything**. Unlike traditional databases where ID is just another column, in SQLid:

- **ID is built-in** - Exists in every table by design
- **Zero storage cost** - O(0) memory overhead  
- **Always returned** - Every operation reveals affected IDs
- **Non-creatable** - ID emerges from system, not user input

## Why "SQLid"?

The name reflects architecture:
- **SQL** - Full SQL support with complex queries
- **id** - ID-centric design where identifiers drive everything


## SQLid Performance (12k records)

| Operation          | Time (10k operations) | Time per query |
|--------------------|----------------------|----------------|
| Full scan (O(n))   | 59.8 sec             | ~6.0 ms        |
| Index search (O(1))| 1.0 sec              | ~0.1 ms        |
| Range query (O(m)) | 1.2 sec              | ~0.12 ms       |
| Insert (O(1))      | 0.9 sec              | ~0.09 ms       |

**Indexed search acceleration: 60x**

## Asymptotic Performance

| Operation | Complexity | Description |
|-----------|------------|-------------|
| Single Insert | O(1) | Hash-based indexing |
| Batch Insert | O(k) | k = batch size |
| Direct Search | O(1) | By primary key (ID) |
| Range Query | O(m) | m = range size |
| Full Scan | O(n) | n = total records |

> Read asymptotics.md for more info

## Features

- Full SQL Support - SELECT, INSERT, UPDATE, DELETE with complex WHERE clauses **and ORDER BY**
- Multi-Threaded - Concurrent access with stripe-based locking
- File-Based Storage - Direct file operations with memory mapping  
- Network API - Remote connections via TCP/IP
- Backup System - Database backup and restore functionality
- Cross-Platform - Works on Windows and Linux

## Quick Start

### Server: 
```
cmake .. -DCMAKE_CXX_STANDARD=23 
make -j4
./SQLid [port] [working_path]
OR
./SQLid
# Set wanted port using console
# Set wanted path using console
```

### Client:
```cpp
SQLid_API api(boost_contex, ip, port);
api.connect();

auto result = api.request(SQL_command); //Work with database
```
> Look for examples/ on 'ImGuiDemo' branch for more info

### SQL Examples:
```sql
CREATE DATABASE mydb
CREATE TABLE FOR mydb users name STRING[32] age INT32
ATTACH TO mydb
INSERT INTO users "John" 25
SELECT * FROM users WHERE age > 20
SELECT * FROM users LIMIT 10 ORDER BY name DESC age ASC
UPDATE users SET age = 26 WHERE id == 1
DELETE FROM users WHERE name == "John"
```
> Read documentary.md for more info

## Technical Details

- Language: C++23
- Storage: Offset-based file system
- Concurrency: Stripe locking (128 segments)
- Networking: Boost.Asio  
- SQL Parser: Custom recursive descent parser

## Project Structure

```
src/
	files/          # Database engine
	clients/        # TCP server/client
	parser/         # SQL parser
	data_base/      # Command execution
	exception/      # Exceptions that can be throwed
	result/         # Result class
	system/         # Control databases
include/
	SQLid_API.h	    # Api for distant work
```
## Supported Types

- Integers: INT32, INT64
- Floats: FLOAT, DOUBLE
- Strings: STRING[N], TEXT
- Boolean: BOOL
