#include "config.hpp"
#include "sqlite.hpp"

ServiceData& ServiceData::getInstance() { 	// we dont have to put here static keyword because we are defining it inside the class (hpp file)
	static ServiceData instance;										// instance will be created only once (constructor will be called only once because of static keyword)
	return instance;
}

const json& ServiceData::get_data_model() const{
	return m_data_model;
}

const json& ServiceData::get_ems_model() const{
	return m_ems_model;
}

sqlite3* ServiceData::get_db() const{
	return this->m_database;
}

sqlite3* ServiceData::open_db(const std::string& db_path) {
	sqlite3* db;
	if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
		std::cerr << "Error: Unable to open database" << std::endl;
		exit(1);
	}
	return db;
}

int ServiceData::startTables() {
	int rc = 0;
	const json& config = get_data_model();
	for (const auto& table : config["DataModel"]) {
		std::string table_name = table["database_table"];
		bool unique = table["unique"];
		std::cout << "Unique: " << unique << std::endl;
		std::string sql;
		if (!unique) { 
			sql = "CREATE TABLE IF NOT EXISTS " + table_name + " (id INTEGER PRIMARY KEY, ";
		} else 
		{
			sql = "CREATE TABLE IF NOT EXISTS " + table_name + " (";
		}
		for (const auto& column : table["parameters"]) {
			std::string column_name = column["db_column"];
			std::string type = "TEXT";
			sql += column_name + " " + type + ", ";
		}
		sql.pop_back(); 
		sql.pop_back(); 
		sql += ");";
		if (SqliteUtils::executeSql(this->m_database, sql) != 0) {
			std::cerr << "Error: Unable to create table" << std::endl;
			rc = 1;
		}
	}
	return rc;
}

int ServiceData::loadDefault() {
	int rc = 0;
	const json& config = get_data_model();
	for (const auto& table : config["DataModel"]) {
		std::string table_name = table["database_table"];
		std::string sql = table["default_sql"];
	   	if (sql.empty()) {	
			continue;
		}
		if (SqliteUtils::executeSql(this->m_database, sql) != 0) {
			std::cerr << "Error: Unable to load default data" << std::endl;
			rc = 1;
		}
	}
	return rc;
}

