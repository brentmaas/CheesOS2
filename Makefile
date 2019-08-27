TARGET = CheesOS2
BUILD = build
SRC = src
CXXFLAGS = -I$(SRC) -ffreestanding -g -std=c++17 -Wall -Wextra
LDFLAGS = -ffreestanding -nostdlib -g -lgcc

CXX = i686-elf-gcc

rwildcard = $(foreach d, $(wildcard $1*), $(call rwildcard, $d/, $2) $(filter $(subst *, %, $2), $d))

SRCS := $(patsubst $(SRC)/%, %, $(call rwildcard, $(SRC)/, *.cpp))
OBJECTS := $(SRCS:%.cpp=%.o)

TOTAL := $(words $(OBJECTS) .)
progress = $(or $(eval PROCESSED := $(PROCESSED) .),$(info [$(words $(PROCESSED))/$(TOTAL)] $1))

vpath %.o $(BUILD)/objects
vpath %.cpp $(SRC)

all: dir bootloader $(OBJECTS)
	@$(CXX) -T $(SRC)/linker.ld $(BUILD)/objects/boot.o $(BUILD)/objects/$(OBJECTS) -o $(BUILD)/$(TARGET).bin $(LDFLAGS)
	@echo Done

dir:
	@mkdir -p $(BUILD)

bootloader:
	@$(CXX) -c $(SRC)/boot.s -o $(BUILD)/objects/boot.o $(CXXFLAGS)

%.o: %.cpp
	@$(call progress,Compiling $<)
	@mkdir -p $(BUILD)/objects/$(dir $@)
	@$(CXX) -c $< -o $(BUILD)/objects/$@ $(CXXFLAGS)

clean:
	@rm -rf $(BUILD)
	@echo Done

run:
	@echo Starting $(TARGET) on QEmu-i386
	@qemu-system-i386 -kernel $(BUILD)/$(TARGET).bin