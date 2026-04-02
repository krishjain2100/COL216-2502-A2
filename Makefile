# # Compiler and flags
# CXX = g++
# CXXFLAGS = -std=c++17 -Wall

# # ==========================================
# # make compile FILE=<filename.cpp>
# # ==========================================
# # This target should compile your files with the provided 
# # main.cpp. The main.cpp will always #include "Processor.h" 
# # and will have its own main() function.
# compile:
# 	@echo "Compiling simulator:"
# 	$(CXX) $(CXXFLAGS) $(FILE) -o main
# 	@echo "Build successful, 'main' created."

# # ==========================================
# # make run FILE=<filename.s>
# # ==========================================
# # Update this target to run whatever script or 
# # program you wrote to preprocess the assembly labels. 
# # Example below assumes a Python script named 'compiler.py'.
# run:
# 	@echo "Preprocessing $(FILE)..."
# # 	$(FILE)
# 	@echo "Preprocessing complete."

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

# Directories and files
SRC_DIR = src
TARGET = main

# Automatically find all .cpp files in the src/ directory
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Default target when you just run 'make'
all: compile

# Target to compile the simulator
compile:
	@echo "Compiling simulator..."
	@ $(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)
	@echo "Build successful, '$(TARGET)' executable created."

# Target to clean up the compiled executable
clean:
	@echo "Cleaning up..."
	rm -f $(TARGET)