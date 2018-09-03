# Initial setup

ifeq '$(shell uname -s)' 'Linux'
SHELL := /bin/bash
OS := linux 
else
$(error 'Makefile not setup for the current OS')
endif

# Linux
.PHONY: build-linux_on-linux

build-linux: TARGET_FOLDER = Linux Release
build-linux: build-linux_on-$(OS) extra_libraries_linux
	g++ -fmessage-length=0 -std=c++11 -Wno-sign-compare -Wno-unused-local-typedefs -Wno-reorder -Wno-switch -fpermissive -static-libstdc++ -static-libgcc -Wl,-rpath=. $(FIXED_INCLUDES) $(LIBPATH) $(LIBS) -L'$(TARGET_FOLDER)' -o '$(TARGET_FOLDER)/App'

extra_libraries_linux:
	gcc -fPIC -c src/Util/LzmaDec.c -o '$(TARGET_FOLDER)/liblzma.o'
	gcc -fPIC -c src/Util/miniz.c -o '$(TARGET_FOLDER)/libminiz.o'
	gcc -fPIC -Ilibraries/opencl/lib/linux -c libraries/bullet/include/clew/clew.c -o '$(TARGET_FOLDER)/libclew.o' 
	ar rcs '$(TARGET_FOLDER)/libclibs.a' '$(TARGET_FOLDER)/libminiz.o' '$(TARGET_FOLDER)/liblzma.o' '$(TARGET_FOLDER)/libclew.o'

build-linux_on-linux:
TARGET_FOLDER := Linux Release
FIXED_INCLUDES := -Isrc/Engine src/Engine/*.cpp -Isrc/Engine/Input src/Engine/Input/*.cpp -Isrc/Engine/Main src/Engine/Main/main_Linux.cpp src/Engine/Main/main_SDL.cpp -Isrc/Engine/Platform src/Engine/Platform/*.cpp -Isrc/Engine/Renderer src/Engine/Renderer/*.cpp -Isrc/GUI src/GUI/*.cpp -Isrc/GUI/Windows src/GUI/Windows/*.cpp -Isrc/Util src/Util/*.cpp -Isrc/App src/App/*.cpp
LIBPATH := $(shell for folder in libraries/*; do if [ -d "$$folder/lib/linux" ]; then echo "-L$$folder/lib/linux -I$$folder/include"; fi; done) -Ilibraries/glew/include/GL
LIBS := -lcurl -lz -lX11 -lXi -lGL -lGLU -lGLEW -lfreetype -lbass -lbass_fx -lOpenCL -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lenet -lpthread -ljpeg -lclibs

ifeq '$(shell if [ -d "$(TARGET_FOLDER)" ]; then echo 1; fi)' '1'
$(warning "Cleaning up the $(TARGET_FOLDER) folder...")
$(shell rm -rf './$(TARGET_FOLDER)/*')
else
$(warning "Creating the $(TARGET_FOLDER) folder...")
$(shell mkdir './$(TARGET_FOLDER)')
endif
