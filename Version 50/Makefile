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

# Objects:
$(OBJ_DIR)/sqlite3.o: sqlite3.c
	mkdir -p ${OBJ_DIR}
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ_DIR)/%.o: %.cpp
	mkdir -p ${OBJ_DIR}
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# Executables:
$(BIN_DIR)/connect4: $(OBJ_FILES)
	mkdir -p ${BIN_DIR}
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)
	
