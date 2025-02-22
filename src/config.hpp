// config.hpp
#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include <pugixml.hpp>

using json = nlohmann::json;

class ServiceData {
	private:
		json data_model;
		json ems_model;
		sqlite3* db;
	public:
		ServiceData(const std::string model_path, std::string db_file) {
			std::string data_model_path = model_path + "/data_model.json";
			std::string ems_model_path = model_path + "/ems_model.json";
			std::ifstream data_model_file(data_model_path);
			std::ifstream ems_model_file(ems_model_path);

			if (!data_model_file.is_open() || !ems_model_file.is_open()) {
				std::cerr << "Error: Unable to open model files" << std::endl;
				exit(1);
			}
			
			data_model = json::parse(data_model_file);
			ems_model = json::parse(ems_model_file);

			if (sqlite3_open(db_file.c_str(), &db) != SQLITE_OK) {
				std::cerr << "Error: Unable to open database" << std::endl;
				exit(1);
			}

			  
			
			}
			
		};


		~ServiceData() {};
};

#endif // _CONFIG_HPP
