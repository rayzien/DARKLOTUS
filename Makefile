CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-missing-field-initializers -O2 -Iinclude -Ithird_party/raylib/include
LDFLAGS = third_party/raylib/lib/libraylib.a -lm -lpthread -ldl -lrt -lX11

SRCS = src/main.cpp \
       src/Core/GameManager.cpp \
       src/Managers/ClickManager.cpp \
       src/Managers/ButtonManager.cpp \
       src/Managers/SettingsManager.cpp \
       src/Managers/GraphicsManager.cpp \
       src/Managers/CharacterManager.cpp \
       src/Managers/AnimationManager.cpp \
       src/Managers/UIManager.cpp \
       src/Managers/CombatManager.cpp \
       src/Managers/SoundManager.cpp \
       src/Managers/StageManager.cpp \
       src/Entities/Fighter.cpp \
       src/Utils/Logger.cpp

OBJS = $(SRCS:src/%.cpp=build/%.o)
TARGET = bin/DarkLotus

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS) | bin
	$(CXX) $(OBJS) $(LDFLAGS) -o $@
	@echo "========================================"
	@echo "  DARKLOTUS BUILD SUCCESSFUL!"
	@echo "  Run with: ./bin/DarkLotus"
	@echo "========================================"

build/%.o: src/%.cpp | build
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

bin:
	mkdir -p bin

build:
	mkdir -p build

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build bin/DarkLotus
