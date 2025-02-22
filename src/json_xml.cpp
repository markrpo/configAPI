#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include <pugixml.hpp>

// to compile use g++ -std=c++11 -o json_xml json_xml.cpp -lsqlite3 -lpugixml

using json = nlohmann::json;

json load_config(const std::string& path) {
	std::ifstream file(path);
    return json::parse(file);
}

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
    sqlite3_close(db);
}

void process_service(const json& service, const std::string& xml_path) {

	for (const auto& param : service["parameters"]) {
		std::string xml_source = param["xml_source"];
		std::string value;
		std::string db_column = param["db_column"];
		std::string type = param["type"];
		
		std::cout << "xml_source: " << xml_source << std::endl;
		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(xml_path.c_str());
		if (!result) {
			std::cout << "XML [" << xml_path << "] parsed with errors, attr value: [" << result.description() << "]" << std::endl;
		}
		pugi::xml_node node = doc.child("ems"); // root node
		// config node
		node = node.child("config");
		if (!node) {
			std::cout << "Root node not found" << std::endl;
		}
		value = node.child_value(xml_source.c_str());
		if (value.empty()) {
			std::cout << "Value not found for xml_source: " << xml_source << std::endl;
			std::string sql = "INSERT INTO ems_config (atribute, xml_source, type) VALUES ('" + db_column + "', '" + xml_source + "', '" + type + "');";
			insert_db("./schema/ems.db", sql);
		} else {
			std::cout << "value: " << value << std::endl;
			std::string sql = "INSERT INTO ems_config (atribute, xml_source, value, type) VALUES ('" + db_column + "', '" + xml_source + "', '" + value + "', '" + type + "');";
			insert_db("./schema/ems.db", sql); 
		}
		/*for (const auto& node : node.children()) { // iterate through all children
			std::string node_name = node.name();
			std::cout << "Checking node: " << node_name << std::endl;
			if (node_name == xml_source) {
				value = node.child_value();
				std::cout << "value: " << value << std::endl;
			}
		}*/ 
	}
}

void generate_xml(const std::string& db_path, const std::string& xml_path) {
	sqlite3* db;
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc) {
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
    }
    std::string sql = "SELECT * FROM ems_config;";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
		std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
	}
	pugi::xml_document doc;
	doc.load_file(xml_path.c_str());
	pugi::xml_node root = doc.child("ems");
	if (!root) {
		std::cout << "Root node not found, generating new one" << std::endl;
		root = doc.append_child("ems");
	}
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		std::string value;
		std::string atribute = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
		std::cout << "atribute: " << atribute << std::endl;
		std::string xml_source = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
		int column_type = sqlite3_column_type(stmt, 3);
		if (column_type == SQLITE_NULL) {
			std::cout << "Value is NULL" << std::endl;
		}
		else {
			std::cout << "Value is not NULL" << std::endl;
			value = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
			std::cout << "value: " << value << std::endl;
		}
		std::string type = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
		std::cout << "type: " << type << std::endl;
		if (!value.empty()) {
		std::cout << "atribute: " << atribute << ", xml_source: " << xml_source << ", value: " << value << ", type: " << type << std::endl;
		pugi::xml_node node = root.append_child(xml_source.c_str());
		node.append_child(pugi::node_pcdata).set_value(value.c_str());
		}
	}
	doc.save_file(xml_path.c_str());
	sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void process_evchargers(const json& evcharger, const std::string& xml_path) {
	std::string sql = "CREATE TABLE IF NOT EXISTS evcharger (id INTEGER PRIMARY KEY, ";
	std::cout << "Processing evcharger" << std::endl;
	// create query for creating table
	for (const auto& param : evcharger["parameters"]) {
		std::cout << "Processing evcharger parameters" << std::endl;
		std::string db_column = param["db_column"];
		std::string type = param["type"];
		sql += db_column + " TEXT, ";
		std::cout << "db_column: " << db_column << ", type: " << type << std::endl;
	}
	sql.pop_back(); // remove last comma
	sql.pop_back(); // remove last space
	sql += ");";
	std::cout << "sql: " << sql << std::endl;

	// create table
	insert_db("./schema/ems.db", sql);
	
	sql.clear();

	pugi::xml_document doc;
	doc.load_file(xml_path.c_str());
	pugi::xpath_node_set evchargers = doc.select_nodes("/ems/assets/evcharger");
	int number_of_evchargers = evchargers.size();
	std::cout << "Number of evchargers: " << number_of_evchargers << std::endl;
}
	

void delete_all_db(const std::string& db_path, const std::string& table) {
    sqlite3* db;
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc) {
	std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
	sqlite3_close(db);
    }
	std::string sql = "DROP TABLE IF EXISTS " + table + ";";
	std::cout << "sql: " << sql << std::endl;
    rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0); 
    if (rc != SQLITE_OK) {
		std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
    }
    sqlite3_close(db);
}

void start_sqlite(const std::string& db_path, const json& config) {
	sqlite3* db;
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc) {
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
    }
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
		rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
		std::cout << "sql: " << sql << std::endl;	
		if (rc != SQLITE_OK) {
			std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
			sqlite3_close(db);
		}
	}
    sqlite3_close(db);

}

void load_default(const std::string& db_path, const json& config) {
	sqlite3* db;
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc) {
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
    }
	for (const auto& table : config["DataModel"]) {
		std::string table_name = table["database_table"];
		std::string sql = table["default_sql"];
	   	if (sql.empty()) {	
			continue;
		}
		rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
		std::cout << "sql: " << sql << std::endl;	
		if (rc != SQLITE_OK) {
			std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
			sqlite3_close(db);
		}
	}
    sqlite3_close(db);
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

void generate_service(const std::string& db_path, json& service, const std::string& xml_path) {
	sqlite3* db;
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc) {
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
    }
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
		rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
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


int main() {
    json config = load_config("./schema/data-model.json");
	for (const auto& table : config["DataModel"]) {
		std::string table_name = table["database_table"];
		delete_all_db("./schema/db/data-model.db", table_name);
	}
	start_sqlite("./schema/db/data-model.db", config);
	load_default("./schema/db/data-model.db", config);
	json services = load_config("./schema/ems-model.json");
	generate_service("./schema/db/data-model.db", services, "./schema/etc/emsoutput.xml");

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
