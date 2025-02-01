#include <iostream>
#include <string>
#include <vector>

#include "routes.hpp"


int main() {
    crow::SimpleApp app;

    set_routes(app);

    app.port(8080).multithreaded().run();
}


