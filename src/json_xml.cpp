#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include <pugixml.hpp>

// to compile use g++ -std=c++11 -o json_xml json_xml.cpp -lsqlite3 -lnlohmann_json -lpugixml

using json = nlohmann::json;

json load_config(const std::string& path) {
	std::ifstream file(path);
    return json::parse(file);
}

void process_service(const json& service, const std::string& xml_path) {

	for (const auto& param : service["parameters"]) {
		std::string xml_source = param["xml_source"];
		std::string value;
		
		std::cout << "xml_source: " << xml_source << std::endl;
		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(xml_path.c_str());
		if (!result) {
			std::cout << "XML [" << xml_path << "] parsed with errors, attr value: [" << result.description() << "]" << std::endl;
		}
		pugi::xml_node node = doc.child("ems"); // root node
		value = node.child_value(xml_source.c_str());
		if (value.empty()) {
			std::cout << "Value not found for xml_source: " << xml_source << std::endl;
		} else {
			std::cout << "value: " << value << std::endl;
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

int main() {
    json config = load_config("./schema/config.json");
    for (const auto& service : config["services"]) {
	process_service(service, "./schema/ems.xml");
    }
    return 0;
}
