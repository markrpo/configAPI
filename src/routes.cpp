#include "routes.hpp"

void set_routes(crow::SimpleApp& app){
	app.route_dynamic("/hello/<int>")
	([](const crow::request& req, crow::response& res, int id){
		res.set_header("Content-Type", "application/json");
		res.write("Hello, World!");
		res.end();
	});
}
