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

CXX = g++
CXXFLAGS = -std=c++17 -Wall

compile:
	@echo "Compiling simulator..."
	$(CXX) $(CXXFLAGS) $(FILE) -o main
	@echo "Build successful, 'main' executable created."

run:
	@echo "Building C++ preprocessor..."
	$(CXX) $(CXXFLAGS) Preprocessor.cpp -o preprocessor
	@echo "Preprocessing $(FILE)..."
	./preprocessor $(FILE)
	@echo "Preprocessing complete."

clean:
	rm -f main preprocessor
	@echo "Cleaned up executable files."