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
		ServiceData(const std::string& model_path, const std::string& db_file); 	
		

	public:

		static ServiceData& getInstance(const std::string model_path, std::string db_file); 	// Making static to ensure only one instance of the class is created (Singleton)
																								// This is a static method that returns a reference to the instance of the class and is the only way to get the instance of the class

		ServiceData(const ServiceData&) = delete;					// Delete copy constructor to prevent cloning by copy (ServiceData a = b)
		ServiceData& operator=(const ServiceData&) = delete;		// Delete assignment operator to prevent cloning by assignment (a = b)

		const json& get_data_model() const;								// const is used to ensure that the method does not modify the object
		const json& get_ems_model() const;									// const is used to ensure that the method does not modify the object
		sqlite3* get_db() const;									// const is used to ensure that the method does not modify the object

};

#endif // _CONFIG_HPP
