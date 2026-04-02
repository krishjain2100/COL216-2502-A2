
### ------------------------------------------------------------###
### ------------------------------------------------------------###
### -------------------------  TESTING  ------------------------###
### ------------------------------------------------------------###
### ------------------------------------------------------------###

CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Iinclude
SRCS = $(wildcard src/*.cpp)
CORE_SRCS = $(filter-out src/main.cpp, $(SRCS))

.PHONY: compile test clean

compile:
	@echo "Compiling the processor..."
	@$(CXX) $(CXXFLAGS) $(SRCS) -o main
	@echo "Success!"

test: 
	@if [ -z "$(FILE)" ]; then echo "Error: FILE argument is required. Usage: make test FILE=<filename.txt>"; exit 1; fi
	@echo "Preprocessing..."
	@echo '#include "Preprocessor.h"' > temp_prep.cpp
	@echo 'int main(int argc, char** argv) { if(argc>1) return preprocess(argv[1]) ? 0 : 1; return 1; }' >> temp_prep.cpp
	@$(CXX) $(CXXFLAGS) $(CORE_SRCS) temp_prep.cpp -o prep
	@./prep $(FILE)
	@rm -f prep temp_prep.cpp
	@echo "Running..."
	@./main $(FILE)

clean:
	rm -f main prep temp_prep.cpp


### ------------------------------------------------------------###
### ------------------------------------------------------------###
### -----------------------  SUBMISSION  -----------------------###
### ------------------------------------------------------------###
### ------------------------------------------------------------###

# CXX = g++
# CXXFLAGS = -std=c++17 -O3 -Wall -Iinclude
# CORE_SRCS = $(filter-out src/main.cpp, $(wildcard src/*.cpp))

# .PHONY: compile run clean

# compile:
# 	@if [ -z "$(FILE)" ]; then echo "Error: FILE argument is required. Usage: make compile FILE=<filename.cpp>"; exit 1; fi
# 	@echo "Compiling processor alongside $(FILE)..."
# 	@$(CXX) $(CXXFLAGS) $(CORE_SRCS) $(FILE) -o main
# 	@echo "Successfully created executable: main"

# run:
# 	@if [ -z "$(FILE)" ]; then echo "Error: FILE argument is required. Usage: make run FILE=<filename.s>"; exit 1; fi
# 	@echo "Preprocessing..."
# 	@echo '#include "Preprocessor.h"' > temp_prep.cpp
# 	@echo 'int main(int argc, char** argv) { if(argc>1) return preprocess(argv[1]) ? 0 : 1; return 1; }' >> temp_prep.cpp
# 	@$(CXX) $(CXXFLAGS) $(CORE_SRCS) temp_prep.cpp -o prep
# 	@./prep $(FILE)
# 	@rm -f prep temp_prep.cpp

# clean:
# 	@rm -f main prep temp_prep.cpp
