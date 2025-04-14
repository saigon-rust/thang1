CC = g++
CFLAGS = -pthread

PARENT_DIR := $(shell basename $(PWD))

BUILD_DIR = ../../build
OBJ_DIR = $(BUILD_DIR)/obj/$(PARENT_DIR)

SRC_CPP = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC_CPP))

#OBJS = $(wildcard $(OBJ_DIR)/*.o)

HEADER = ../../include
HEADER_DIR = $(HEADER)/$(PARENT_DIR)

BUILD_SERVER = $(BUILD_DIR)/server

all: create_dirs build_o build_server

create_dirs:
	@mkdir -p $(OBJ_DIR)

build_o: $(OBJ)

$(OBJ_DIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -I$(HEADER) -c $< -o $@

build_server: $(BUILD_SERVER)

$(BUILD_SERVER): $(OBJ) 
	$(CC) $(CFLAGS)  -o $@ $(OBJ) -lsqlite3

clean:
	rm -rf $(OBJ_DIR) $(BUILD_SERVER)

run:
	$(BUILD_SERVER)

.PHONY: all clean run
