#include "config.hpp"

ServiceData::ServiceData(const std::string& model_path, const std::string& db_file) {
			std::string data_model_path = model_path + "/data-model.json";
			std::string ems_model_path = model_path + "/ems-model.json";
			std::ifstream data_model_file(data_model_path);
			std::ifstream ems_model_file(ems_model_path);

			if (!data_model_file.is_open() || !ems_model_file.is_open()) {
				std::cerr << "Error: Unable to open model files" << std::endl;
				std::cerr << "Data model path: " << data_model_path << std::endl;
				std::cerr << "EMS model path: " << ems_model_path << std::endl;
				exit(1);
			}
			
			data_model = json::parse(data_model_file);
			ems_model = json::parse(ems_model_file);

			if (sqlite3_open(db_file.c_str(), &db) != SQLITE_OK) {
				std::cerr << "Error: Unable to open database" << std::endl;
				exit(1);
			}
}

ServiceData& ServiceData::getInstance(const std::string model_path, std::string db_file) { // we dont have to put here static keyword because we are defining it inside the class
	static ServiceData instance(model_path, db_file);
	return instance;
}

const json& ServiceData::get_data_model() const{
	return data_model;
}

const json& ServiceData::get_ems_model() const{
	return ems_model;
}

sqlite3* ServiceData::get_db() const{
	return db;
}
