ifeq ($(OS), Windows_NT)
	ifdef MSYSTEM
		CXX = g++

		RM = rm -fr
		MKDIR = mkdir -p
		TOUCH = touch
		PRINT = echo

		CMAKE = cmake -G "MSYS Makefiles"
	else
		$(error "Please run this makefile from MSYS2")
	endif
else
	CXX = g++

	RM = rm -rf
	MKDIR = mkdir -p
	TOUCH = touch
	PRINT = echo

	CMAKE = cmake
endif

SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = $(BIN_DIR)/src
LIB_DIR = lib

SRC = $(shell find $(SRC_DIR) -name "*.cpp")
OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))
DEP = $(OBJ:.o=.d)

DEPFLAGS = -MMD -MP

CXXFLAGS = -Wall -Wextra -Werror -I$(SRC_DIR)

ifeq ($(OS), Windows_NT)
	TARGET = vulkan-minecraft.exe
else
	TARGET = vulkan-minecraft
endif

GLFW_DIR = $(LIB_DIR)/glfw
GLFW_INC = $(GLFW_DIR)/include
GLFW_BIN = $(BIN_DIR)/lib/glfw
GLFW_LIB_DIR = $(GLFW_BIN)/src
GLFW_LIB = $(GLFW_LIB_DIR)/libglfw3.a

GLFW_FLAGS = -DBUILD_SHARED_LIBS=OFF \
			 -DGLFW_BUILD_EXAMPLES=OFF \
			 -DGLFW_BUILD_TESTS=OFF \
			 -DGLFW_BUILD_DOCS=OFF

CXXFLAGS += -I$(GLFW_INC)
LDFLAGS += -L$(GLFW_LIB_DIR) -lglfw3

ifeq ($(OS), Windows_NT)
	LDFLAGS += -lgdi32
endif

GLM_DIR = $(LIB_DIR)/glm

CXXFLAGS += -I$(GLM_DIR)

TOMPLUSPLUS_DIR = $(LIB_DIR)/tomplusplus
TOMPLUSPLUS_INC = $(TOMPLUSPLUS_DIR)/include

CXXFLAGS += -I$(TOMPLUSPLUS_INC)

STB_DIR = $(LIB_DIR)/stb

CXXFLAGS += -I$(STB_DIR)

FASTNOISE_DIR = $(LIB_DIR)/fastnoise
FASTNOISE_INC = $(FASTNOISE_DIR)/Cpp

CXXFLAGS += -I$(FASTNOISE_INC)

ifeq ($(OS), Windows_NT)
	VULKAN_INC = $(VULKAN_SDK)/Include
	VULKAN_LIB = $(VULKAN_SDK)/Lib
endif

VMA_DIR = $(LIB_DIR)/vma
VMA_INC = $(VMA_DIR)/include

CXXFLAGS += -I$(VMA_INC)

ifeq ($(OS), Windows_NT)
	CXXFLAGS += -I$(VULKAN_INC)
	LDFLAGS += -L$(VULKAN_LIB) -lvulkan-1
else
	CXXFLAGS += $(shell pkg-config --cflags vulkan)
	LDFLAGS += $(shell pkg-config --libs vulkan)
endif

SHADERS_DIR = $(SRC_DIR)/shaders
SHADERS_BIN = assets/shaders

SHADER_SRC = $(shell find $(SHADERS_DIR) -name "*.vert") \
			 $(shell find $(SHADERS_DIR) -name "*.frag")

SHADER_DST = $(patsubst $(SHADERS_DIR)/%.vert, $(SHADERS_BIN)/%.vert.spv, $(SHADER_SRC)) \
			 $(patsubst $(SHADERS_DIR)/%.frag, $(SHADERS_BIN)/%.frag.spv, $(SHADER_SRC))

ifeq ($(OS), Windows_NT)
	GLSLC = $(VULKAN_SDK)/Bin/glslc.exe
else
	GLSLC = glslangValidator -V
endif

MODE ?= debug

ifeq ($(MODE), debug)
	CXXFLAGS += -g -O3 -DDEBUG -pipe
else
	CXXFLAGS += -O3 -DNDEBUG -pipe
endif

ifeq ($(OS), Windows_NT)
	LDFLAGS += -static-libgcc -static-libstdc++ -static
	ifeq ($(MODE), release)
		LDFLAGS += -mwindows
	endif
else
	LDFLAGS += -ldl -pthread
endif

all: $(TARGET) $(SHADER_DST)

release:
	@$(MAKE) MODE=release all

$(TARGET): $(OBJ) | $(GLFW_LIB)
	@$(PRINT) "Linking $@"
	@$(CXX) -o $(TARGET) $(OBJ) $(LDFLAGS)

-include $(DEP)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	@$(PRINT) "Compiling $< -> $@"
	@$(MKDIR) $(dir $@)
	@$(CXX) -c $< -o $@ $(DEPFLAGS) $(CXXFLAGS)

$(BIN_DIR):
	@$(PRINT) "Creating $(BIN_DIR)"
	@$(MKDIR) $(BIN_DIR)

shaders: $(SHADER_DST)

$(SHADERS_BIN)/%.vert.spv: $(SHADERS_DIR)/%.vert | $(SHADERS_BIN)
	@$(PRINT) "Compiling $< -> $@"
	@$(MKDIR) $(dir $@)
	@$(GLSLC) $< -o $@

$(SHADERS_BIN)/%.frag.spv: $(SHADERS_DIR)/%.frag | $(SHADERS_BIN)
	@$(PRINT) "Compiling $< -> $@"
	@$(MKDIR) $(dir $@)
	@$(GLSLC) $< -o $@

$(SHADERS_BIN):
	@$(PRINT) "Creating $(SHADERS_BIN)"
	@$(MKDIR) $(SHADERS_BIN)

glfw: $(GLFW_LIB)
	
$(GLFW_LIB):
	@$(PRINT) "Building GLFW"
	@$(CMAKE) -S $(GLFW_DIR) -B $(GLFW_BIN) $(GLFW_FLAGS)
	@$(MAKE) -C $(GLFW_BIN)

.NOTPARALLEL: $(GLFW_LIB)

clean-glfw:
	@$(PRINT) "Cleaning GLFW"
	@$(RM) $(GLFW_BIN)

clean:
	@$(PRINT) "Cleaning"
	@$(RM) $(OBJ) $(DEP) $(TARGET)

clean-all:
	@$(PRINT) "Cleaning all"
	@$(RM) $(BIN_DIR) $(TARGET)

re: clean all

.PHONY: all release clean re clean-all glfw clean-glfw