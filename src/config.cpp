#include "config.hpp"

ServiceData::ServiceData() {
			std::string data_model_path = this->model_path + "/data-model.json";
			std::string ems_model_path = this->model_path + "/ems-model.json";
			std::ifstream data_model_file(data_model_path);
			std::ifstream ems_model_file(ems_model_path);

			if (!data_model_file.is_open() || !ems_model_file.is_open()) {
				std::cerr << "Error: Unable to open model files" << std::endl;
				std::cerr << "Data model path: " << data_model_path << std::endl;
				std::cerr << "EMS model path: " << ems_model_path << std::endl;
				exit(1);
			}
			
			this->data_model = json::parse(data_model_file);
			this->ems_model = json::parse(ems_model_file);	
			this->database = open_db(this->db_file);
}

ServiceData& ServiceData::getInstance() { 	// we dont have to put here static keyword because we are defining it inside the class (hpp file)
	static ServiceData instance;										// instance will be created only once (constructor will be called only once because of static keyword)
	return instance;
}

const json& ServiceData::get_data_model() const{
	return data_model;
}

const json& ServiceData::get_ems_model() const{
	return ems_model;
}

sqlite3* ServiceData::get_db() const{
	return this->database;
}

sqlite3* ServiceData::open_db(const std::string& db_path) {
	sqlite3* db;
	if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
		std::cerr << "Error: Unable to open database" << std::endl;
		exit(1);
	}
	return db;
}
