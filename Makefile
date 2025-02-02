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
	@$(SUDO) rm -rf $(INCLUDES)/crow_all.h
	@git clone -b v1.2.0  https://github.com/CrowCpp/Crow.git
	@cd ./Crow/scripts && sudo ./merge_all.py ../include crow_all.h
	@mv ./Crow/scripts/crow_all.h $(INCLUDES)
	@rm -rf Crow

build_local:
	@mkdir -p ~/Desktop/static
	@cp -r ./include/static/* ~/Desktop/static/
	@g++ $(LIBRARIES) -o $(OUTPUT) $(FILES) 

clean:
	@rm -f ../../builds/my_app
	@rm -f ../../builds/my_app_arm

install_docker:
	@$(SUDO) apt-get install -y qemu qemu-user-static
	@$(SUDO) rm -f /etc/apt/sources.list.d/docker.list
	@for pkg in docker.io docker-doc docker-compose docker-compose-v2 podman-docker containerd runc; do $(SUDO) apt-get remove $$pkg; done || exit 1
	@$(SUDO) apt-get update || exit 1; \
	$(SUDO) apt-get install ca-certificates curl || exit 1; \
	$(SUDO) install -m 0755 -d /etc/apt/keyrings || exit 1; \
	$(SUDO) curl -fsSL https://download.docker.com/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc || exit 1; \
	$(SUDO) chmod a+r /etc/apt/keyrings/docker.asc;
	@$(SUDO) echo \ "deb [arch=$$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/ubuntu $$(. /etc/os-release && echo "$$VERSION_CODENAME") stable" |    \
	$(SUDO) tee /etc/apt/sources.list.d/docker.list > /dev/null || exit 1;
	@$(SUDO) apt-get update || exit 1; \
	$(SUDO) apt-get install docker-ce=$(DOCKER_VERSION) docker-ce-cli=$(DOCKER_VERSION) containerd.io docker-buildx-plugin docker-compose-plugin \
	|| $(SUDO) apt-get install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin || exit 1; \

build_arm:
	@$(SUDO) docker buildx create --name $(ARM_BUILDER)
	@$(SUDO) docker buildx use $(ARM_BUILDER)
	@$(SUDO) docker buildx inspect --bootstrap
	@$(SUDO) sudo docker buildx build --platform linux/arm64 -t $(ARM_IMAGE) -f ./docker/dockerfile . --load
	@$(SUDO) docker run -t --platform linux/arm64 --name $(ARM_CONTAINER) $(ARM_IMAGE)
	@$(SUDO) docker cp $(ARM_CONTAINER):/app/cpp/my_app ../../builds/my_app_arm

docker_clean:
	@$(SUDO) docker stop $(ARM_CONTAINER) || true
	@$(SUDO) docker rm $(ARM_CONTAINER) || true
	@$(SUDO) docker rmi $(ARM_IMAGE) || true 
	@$(SUDO) docker buildx rm $(ARM_BUILDER) || true

bootstrap_lib:
	@$(SUDO) apt-get update
	@$(SUDO) apt-get install -y cmake g++
	@$(SUDO) apt-get install -y libasio-dev
	@$(SUDO) apt-get install -y redis-server
	@$(SUDO) apt-get install -y libhiredis-dev 
	@$(SUDO) apt-get install -y yaml-cpp

.PHONY: build_local clean build_arm docker_clean install_docker bootstrap_lib compile_crow

