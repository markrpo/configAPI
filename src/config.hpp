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
public:
	static ServiceData& getInstance(); 	// Making static to ensure only one instance of the class is created (Singleton)
																							// This is a static method that returns a reference to the instance of the class and is the only way to get the instance of the class
	ServiceData(const ServiceData&) = delete;					// Delete copy constructor to prevent cloning by copy (ServiceData a = b)
	ServiceData& operator=(const ServiceData&) = delete;		// Delete assignment operator to prevent cloning by assignment (a = b)
	const json& get_data_model() const;								// const is used to ensure that the method does not modify the object
	const json& get_ems_model() const;									// const is used to ensure that the method does not modify the object
	sqlite3* get_db() const;									// const is used to ensure that the method does not modify the object
	sqlite3* open_db(const std::string& db_file);				// const is used to ensure that the method does not modify the object1
	int loadDefault();
	int startTables();

private:
	const std::string model_path = "./schema";
	const std::string db_file = "./schema/db/data-model.db";
	
	json m_data_model;
	json m_ems_model;
	sqlite3* m_database;

	ServiceData() {
		std::ifstream ifs(model_path + "/data-model.json");
		std::ifstream ifs2(model_path + "/ems-model.json");
		if (!ifs.is_open() || !ifs2.is_open()) {
			std::cerr << "Error opening file" << std::endl;
			return;
		}
		m_data_model = json::parse(ifs);
		m_ems_model = json::parse(ifs2);
		m_database = open_db(db_file);
	}
		
};

#endif // _CONFIG_HPP

	//const std::string db_file = "./schema/db/data-model.db";
	// const char* model_path = "./schema"; can also be used
	// const char* is similar to const char[] but the former is a pointer to a constant character while the latter is an array of constant characters
	// This means that the former can be reassigned to point to another memory location while the latter cannot be reassigned to point to another memory location
	// cont char* can point to another memory location (example using malloc) but can not change the value of the memory location it points to (example using strcpy or strcat or realloc)
	// if you have a buffer like char buffer[100] you can add a string to it using strcpy or strcat<.
	// const char* str1 = "hello" // you can do std1 = "world" but you can't do str1[0] = 'H'
	// const char str2[] = "world" 
	// char buffer[100];
	//
	// strcpy(buffer, str1);  // strcpy is used to copy the contents of str1 to buffer from the beginning of buffer
	// strcat(buffer, str2);  // strcat is used to append the contents of str2 to buffer from the end of buffer
	
/* More about strings and buffers:
 * you can pass a char* to a function that expects a const char* but you can't pass a const char* to a function that expects a char*
 * you can pas a string to a const char* using c_str() method
 * you can pass a string to a char* using data() method (C++11 and above, because data() returns a pointer to the first element of the string)
 * to a function that expets a std::string& you can pass a string directly but you can't pass a const char* or a char* directly
 *
 * Thing about references and pointers:
 * As we know a reference:
 * void funct(int& a) {}
 * int a = 10;
 * funct(a);
 * int* b = &a;
 * funct(*b);
 *
 * With a reference to a char*:
 * void funct(char*& str) {}
 * char* str = "hello";
 * funct(str);
 *
 *	SO TO A FUNCTION THAT EXPECTS A REFERENCE YOU HAVE TO PASS THE SAME TYPE OF VARIABLE, NOTHING EXTRA HAS TO BE DONE
 *	int& a = b; // a reference to an integer, b is an integer
 *	int* a = &b; // a pointer to an integer, b is an integer
 *
 * a string is a reference to a buffer of characters so you can pass a string to a function that expects a reference to a buffer of characters or a reference to a string:
 * void funct(std::string& str) {}
 * std::string str = "hello";
 * funct(str); (if a const std::string& is expected you can pass "Hello" directly)
 * 
 * if you need to change the value of the string you can pass a reference to the string but
 * if you need to change the pointer to the string you can pass a pointer to the string:
 * void funct(std::string* str) {}
 * 
 * a funtion that receives a char** expects a pointer to a pointer to a buffer of characters
 * this is used to change the pointer to the buffer of characters (is important to free the memory of the previous buffer before changing the pointer)
 * void funct(char** str) {
 * 	*str = "world";
 * 		}
 *
 *
 * 	*/
