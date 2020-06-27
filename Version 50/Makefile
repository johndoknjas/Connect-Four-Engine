# Only use this makefile if you want to play around with changing the C++ code and are compiling via the command line (e.g., the Linux terminal). For just using the application
# normally, everything is already compiled and all you have to do is open and run the interface.exe program in /bin/Debug/dist
# (after using pyinstaller on interface.py, or by downloading the dist folder from Google Docs).

# Modify this variable to add source files:
SOURCES += notes
SOURCES += sqlite3
SOURCES += main

# Compiler flags:
CXXFLAGS = -std=c++14 -O2

# Directories:
BIN_DIR = bin/Debug
OBJ_DIR = obj/Debug
OBJ_FILES = $(addprefix ${OBJ_DIR}/,$(addsuffix .o,$(SOURCES)))

# Targets:
.PHONY: clean default all
default: all
all: $(BIN_DIR)/connect4

# Executables:
$(BIN_DIR)/connect4: $(OBJ_FILES)
	mkdir -p ${BIN_DIR}
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)
	
# Objects:
$(OBJ_DIR)/sqlite3.o: sqlite3.c
	mkdir -p ${OBJ_DIR}
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ_DIR)/%.o: %.cpp
	mkdir -p ${OBJ_DIR}
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# Clean:
clean:
	rm ${BIN_DIR}/connect4
	rm $(OBJ_FILES)
