CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -O3 -MMD -MP
INCLUDES := -Isrc
TARGET := build/main
BUILDDIR := build

SOURCES := \
    src/main.cpp \
    src/utils/algorithm.cpp \
    src/utils/graph.cpp \
    src/utils/graph_sparsifier.cpp \
    src/utils/find_union.cpp \
    src/utils/dynamic_order.cpp \
    src/utils/rng.cpp \
    src/utils/algorithm_factory.cpp \
    src/bender/two_way_search.cpp \
    src/bender/naive_one_way_search.cpp \
    src/bender/one_way_search.cpp \
    src/bernstein/sample_search.cpp \
    src/naive/naive_dfs.cpp \
    src/haeupler/limited_search.cpp \
    src/haeupler/compatible_search.cpp \
    src/haeupler/soft_threshold_search.cpp \
    src/haeupler/topological_search.cpp \
    src/haeupler/haeupler_search.cpp

# Object files in build/
OBJECTS := $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

all: $(TARGET)

# Link
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^
# Compile each .cpp into build/... .o
$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)

-include $(DEPS)

.PHONY: all clean
