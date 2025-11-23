# SQLid Database Engine

High-performance file-based SQL database engine written in C++ from scratch

## Features

- Full SQL Support - SELECT, INSERT, UPDATE, DELETE with complex WHERE clauses
- Multi-Threaded - Concurrent access with stripe-based locking
- File-Based Storage - Direct file operations with memory mapping  
- Network API - Remote connections via TCP/IP
- Backup System - Database backup and restore functionality
- Cross-Platform - Works on Windows and Linux

## Performance

| Operation | Complexity | Description |
|-----------|------------|-------------|
| Single Insert | O(1) | Hash-based indexing |
| Batch Insert | O(k) | k = batch size |
| Direct Search | O(1) | By primary key (ID) |
| Range Query | O(m) | m = range size |
| Full Scan | O(n) | n = total records |

> Read asymptotics.md for more info

## Quick Start

### Server: 
```
cmake .. -DCMAKE_CXX_STANDARD=23 
make -j4
./SQLid 
# Set wanted port
```

### Clinet:
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