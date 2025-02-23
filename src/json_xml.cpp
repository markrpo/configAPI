#include "config.hpp"

// to compile use g++ -std=c++11 -o json_xml json_xml.cpp -lsqlite3 -lpugixml

using json = nlohmann::json;

void insert_db(const std::string& db_path, const std::string& sql) {
    sqlite3* db;
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc) {
	std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
	sqlite3_close(db);
    }
    rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0); 
    if (rc != SQLITE_OK) {
		std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
    }
}


void delete_all_db(sqlite3* db, const std::string& table) {
	std::string sql = "DROP TABLE IF EXISTS " + table + ";";
	std::cout << "sql: " << sql << std::endl;
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0); 
    if (rc != SQLITE_OK) {
		std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
    }
}

void start_sqlite(sqlite3* db, const json& config) {
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
		int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
		std::cout << "sql: " << sql << std::endl;	
		if (rc != SQLITE_OK) {
			std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
			sqlite3_close(db);
		}
	}
}

void load_default(sqlite3* db, const json& config) {
	for (const auto& table : config["DataModel"]) {
		std::string table_name = table["database_table"];
		std::string sql = table["default_sql"];
	   	if (sql.empty()) {	
			continue;
		}
		int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
		std::cout << "sql: " << sql << std::endl;	
		if (rc != SQLITE_OK) {
			std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
			sqlite3_close(db);
		}
	}
}

// split path by / and return vector of strings example: /ems/config -> ["ems", "config"]
std::vector<std::string> split_paths(const std::string& path) {
    std::vector<std::string> segments;
    std::stringstream ss(path);
    std::string segment;
    
    while (std::getline(ss, segment, '/')) {
        if (!segment.empty()) { 
            segments.push_back(segment);
        }
    }
    
    return segments;
}

void generate_service(sqlite3* db, const json& service, const std::string& xml_path) {
	pugi::xml_document doc;
	doc.load_file(xml_path.c_str());
	// generate root node
	pugi::xml_node root = doc.append_child(service["root"].get<std::string>().c_str());
	// generate child nodes
	for (const auto& param : service["xml"]) {
		std::string databalse_table = param["database_table"];
		std::string xml_path = param["xml_path"]; // example: /ems/config maybe does not exist, split by /
		std::vector<std::string> xml_segments = split_paths(xml_path);
		for (const auto& segment : xml_segments) {
			// generate if not exists
			pugi::xml_node node = root.child(segment.c_str());
			if (!node) {
				node = root.append_child(segment.c_str());
			}
		}
		pugi::xml_node node = root.child(xml_segments.back().c_str()); // get last element

		std::string sql = "SELECT * FROM " + databalse_table + ";";
		sqlite3_stmt* stmt;
		int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
		if (rc != SQLITE_OK) {
			std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		}
		sqlite3_step(stmt); // get first row
							// xml source = column name
							// value = column value
		std::string value;
		std::string xml_source;
		
		// for each column in table get value and xml_source
		for (int i = 0; i < sqlite3_column_count(stmt); i++) {
			xml_source = std::string(reinterpret_cast<const char*>(sqlite3_column_name(stmt, i)));
			value = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
			std::cout << "xml_source: " << xml_source << ", value: " << value << std::endl;
			//pugi::xml_node sub_value = node.append_child(xml_source.c_str());
			//sub_value.append_child(pugi::node_pcdata).set_value(value.c_str());
		}

	}	
}

void test(const char*& argv) {
	std::cout << "argv: " << argv << std::endl;
	/* Things about cout:
	 * with a char* or const char* it will print all the characters until it finds a null character '\0'
	 * with std::string the same.
	 * with a int, float, double, etc it will print the value of the variable.
	 * witha pointer that is not a char* or const char* it will print the memory address, so with a int* you need to dereference it to get the value:
	 * int a = 5;
	 * int* b = &a;
	 * std::cout << b << std::endl; // will print the memory address
	 * std::cout << *b << std::endl; // will print the value of a
	 */
}


int main() {

	const char* argv = "test";
	test(argv);

	ServiceData& sd = ServiceData::getInstance();

	for (const auto& table : sd.get_data_model()["DataModel"]) {
		std::string table_name = table["database_table"];
		delete_all_db(sd.get_db(), table_name);
	}
	start_sqlite(sd.get_db(), sd.get_data_model());
	load_default(sd.get_db(), sd.get_data_model());
	generate_service(sd.get_db(), sd.get_ems_model(), "./schema/etc/emsoutput.xml");

    /*for (const auto& service : config["services"]) {
	process_service(service, "./schema/ems.xml");
    }
	json evchargers = load_config("./schema/chargers.json");
	std::cout << "loaded evchargers" << std::endl;
	for (const auto& evcharger : evchargers["services"]) {
		process_evchargers(evcharger, "./schema/ems.xml");
	}
	// remove emsoutput.xml
	std::remove("./schema/emsoutput.xml");
	generate_xml("./schema/ems.db", "./schema/emsoutput.xml");*/
    return 0;
}
