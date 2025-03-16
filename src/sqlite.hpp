#ifndef SQLITE_HPP
#define SQLITE_HPP

#include <sqlite3.h>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class SqliteUtils {
public:
	static int deleteAllDb(sqlite3* db, const std::string& table) {
		std::string sql = "DROP TABLE IF EXISTS " + table + ";";
		std::cout << "sql: " << sql << std::endl;
    	int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0); 
    	if (rc != SQLITE_OK) {
			std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
			sqlite3_close(db);
    	}
	}

	static int executeSql(sqlite3* db, const std::string& sql) {
		std::cout << "sql: " << sql << std::endl;
    	int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0); 
    	if (rc != SQLITE_OK) {
			std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
			sqlite3_close(db);
    	}
	}

};	

#endif
