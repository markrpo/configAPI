#include "config.hpp"
#include "sqlite.hpp"

// to compile use g++ -std=c++11 -o json_xml json_xml.cpp -lsqlite3 -lpugixml

using json = nlohmann::json;

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


// generate xml file from json and database, this is only for test purposes and is
// not clean code, it is just to show how to generate xml from json and database
void generate_service(sqlite3* db, const json& service, const std::string& xml_path) {
	pugi::xml_document doc;
	doc.load_file(xml_path.c_str());
	if (!doc) {
		std::cerr << "Could not load xml file" << std::endl;
	}
	std::cout << "xml_path: " << xml_path << std::endl;
	pugi::xml_node root = doc;
	// generate child nodes
	for (const auto& param : service["xml"]) {
		pugi::xml_node node = root;
		std::cout << "Node: " << node.name() << std::endl;
		std::string databalse_table = param["database_table"];
		std::string xml_path = param["xml_path"]; // example: /ems/config maybe does not exist, split by /
		std::vector<std::string> xml_segments = split_paths(xml_path);
		for (const auto& segment : xml_segments) {
			// generate if not exists
			std::cout << "segment: " << segment << std::endl;
			pugi::xml_node node2 = node.child(segment.c_str());
			if (!node2) {
				std::cout << "node does not exist, creating" << std::endl;
				node = node.append_child(segment.c_str());
				std::cout << "Node: " << node.name() << std::endl;
			} else {
				node = node2;
			}
		}

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
			if (sqlite3_column_text(stmt, i) == NULL) {
				continue;
			} else {
				value = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
			}
			std::cout << "xml_source: " << xml_source << ", value: " << value << std::endl;
			pugi::xml_node sub_value = node.append_child(xml_source.c_str());
			sub_value.append_child(pugi::node_pcdata).set_value(value.c_str());
		}
		sqlite3_finalize(stmt);
	}
	doc.save_file(xml_path.c_str());
}

int main() {
	//delete xml:
	std::remove("./schema/etc/emsoutput.xml");

	ServiceData& sd = ServiceData::getInstance();

	for (const auto& table : sd.get_data_model()["DataModel"]) {
		std::string table_name = table["database_table"];
		SqliteUtils::deleteAllDb(sd.get_db(), table_name);
	}

	sd.startTables();
	sd.loadDefault();

	generate_service(sd.get_db(), sd.get_ems_model(), "./schema/etc/emsoutput.xml");

    return 0;
}
