USER_ID := $(shell id -u)
SUDO := $(if $(filter 0,$(USER_ID)),,sudo)

DOCKER_VERSION=5:26.1.3-1~ubuntu.24.04~noble 
ARM_CONTAINER=armdeb_container
ARM_IMAGE=armdeb_image
ARM_BUILDER=arm_builder

#This is only to compile CROW proyect
LIBRARIES=-I./include -lpthread -std=c++11
OUTPUT=./build/app
FILES=./Test.cpp
INCLUDES=./include

compile_crow:
	@$(SUDO) echo "Using sudo!!"
	@mkdir -p $(INCLUDES)
	@$(SUDO) rm -rf $(INCLUDES)/crow_all.h
	@git clone -b v1.2.0  https://github.com/CrowCpp/Crow.git
	@cd ./Crow/scripts && sudo ./merge_all.py ../include crow_all.h
	@mv ./Crow/scripts/crow_all.h $(INCLUDES)
	@rm -rf Crow

compile_plugixml:
	@$(SUDO) echo "Using sudo!!"
	@mkdir -p $(INCLUDES)
	@$(SUDO) rm -rf $(INCLUDES)/pugixml.hpp
	@git clone https://github.com/zeux/pugixml.git
	# install pugixml
	@cd pugixml && mkdir build && cd build && cmake .. && make && $(SUDO) make install
	@$(SUDO) rm -r pugixml

clean:
	@rm -f ../../builds/my_app
	@rm -f ../../builds/my_app_arm

bootstrap_lib:
	@$(SUDO) apt-get update
	@$(SUDO) apt-get install -y libgtest-dev
	@$(SUDO) apt-get install -y cmake g++
	@$(SUDO) apt-get install -y libasio-dev
	@$(SUDO) apt-get install -y sqlite3 libsqlite3-dev
	@$(SUDO) apt-get install -y nlohmann-json3-dev

.PHONY: build_local clean build_arm docker_clean install_docker bootstrap_lib compile_crow

