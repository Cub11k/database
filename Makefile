CXX = g++

SOURCES = db.cpp rbtree.cpp selection.cpp criteria.cpp tokenizer.cpp student.cpp compare.cpp queries.cpp test.cpp server.cpp client.cpp
OBJECTS = $(SOURCES:.cpp=.o)

MAIN_SOURCES = test.cpp server.cpp client.cpp
MAIN_OBJECTS = $(MAIN_SOURCES:.cpp=.o)

EXECUTABLES = $(MAIN_SOURCES:.cpp=)
DEBUG_EXECUTABLES = $(MAIN_SOURCES:.cpp=_debug)

RELEASE_DIR = release
RELEASE_OBJECTS = $(addprefix $(RELEASE_DIR)/, $(OBJECTS))

DEBUG_DIR = debug
DEBUG_OBJECTS = $(addprefix $(DEBUG_DIR)/, $(OBJECTS))

CXXFLAGS = -Wall -Werror -O2 -std=gnu++2a
CXXLIBS = -lm

release: prep_release $(EXECUTABLES)

all: 2020-21-109/Lyutyy_DI/4sem/release debug

debug: CXXFLAGS := $(filter-out -O2, $(CXXFLAGS))
debug: CXXFLAGS += -g
debug: prep_debug $(DEBUG_EXECUTABLES)

test: RELEASE_OBJECTS := $(filter-out $(addprefix $(RELEASE_DIR)/, $(filter-out test.o, $(MAIN_OBJECTS))), $(RELEASE_OBJECTS))
test: $(RELEASE_OBJECTS)
	$(CXX) $(RELEASE_OBJECTS) -o $@ $(CXXFLAGS) $(CXXLIBS)

server: RELEASE_OBJECTS := $(filter-out $(addprefix $(RELEASE_DIR)/, $(filter-out server.o, $(MAIN_OBJECTS))), $(RELEASE_OBJECTS))
server: $(RELEASE_OBJECTS)
	$(CXX) $(RELEASE_OBJECTS) -o $@ $(CXXFLAGS) $(CXXLIBS)

client: RELEASE_OBJECTS := $(filter-out $(addprefix $(RELEASE_DIR)/, $(filter-out client.o, $(MAIN_OBJECTS))), $(RELEASE_OBJECTS))
client: $(RELEASE_OBJECTS)
	$(CXX) $(RELEASE_OBJECTS) -o $@ $(CXXFLAGS) $(CXXLIBS)

test_debug: DEBUG_OBJECTS := $(filter-out $(addprefix $(DEBUG_DIR)/, $(filter-out test.o, $(MAIN_OBJECTS))), $(DEBUG_OBJECTS))
test_debug: $(DEBUG_OBJECTS)
	$(CXX) $(DEBUG_OBJECTS) -o $@ $(CXXFLAGS) $(CXXLIBS)

server_debug: DEBUG_OBJECTS := $(filter-out $(addprefix $(DEBUG_DIR)/, $(filter-out server.o, $(MAIN_OBJECTS))), $(DEBUG_OBJECTS))
server_debug: $(DEBUG_OBJECTS)
	$(CXX) $(DEBUG_OBJECTS) -o $@ $(CXXFLAGS) $(CXXLIBS)

client_debug: DEBUG_OBJECTS := $(filter-out $(addprefix $(DEBUG_DIR)/, $(filter-out client.o, $(MAIN_OBJECTS))), $(DEBUG_OBJECTS))
client_debug: $(DEBUG_OBJECTS)
	$(CXX) $(DEBUG_OBJECTS) -o $@ $(CXXFLAGS) $(CXXLIBS)

$(RELEASE_DIR)/%.o: %.cpp %.h
	$(CXX) -c $< -o $@ -MD -MP $(CXXFLAGS)

$(DEBUG_DIR)/%.o: %.cpp %.h
	$(CXX) -c $< -o $@ -MD -MP $(CXXFLAGS)

prep_release:
	mkdir -p $(RELEASE_DIR)

prep_debug:
	mkdir -p $(DEBUG_DIR)

.PHONY:	clean
clean:
	rm -rf $(RELEASE_DIR) $(DEBUG_DIR) $(EXECUTABLES) $(DEBUG_EXECUTABLES)