CC=gcc -w -ggdb
CXX=g++ -Wall -std=c++11 -ggdb

### setup env
UNAME = $(shell uname)

### setup source dirs
SRC_DIR = src
LIB_DIR = lib
GTL_DIR = gtl
GLM_DIR = glm
LIB_DIRS = $(LIB_DIR) $(GLM_DIR) $(GTL_DIR) 
C_DIRS = common 
SHADER_DIR = shaders
MODEL_DIR = models
TEXTURE_DIR = textures
PROGRAM_DEF_FILE = programs.yaml

ifeq ($(UNAME), Darwin)
    PLATFORM = Mac
else
    PLATFORM = Linux
endif	

C_DIRS += common/$(PLATFORM)

### setup flags
INCLUDE_FLAGS = $(foreach dir, $(LIB_DIRS), -I$(dir)) $(foreach dir, $(C_DIRS), -isystem$(dir)) -I.
CFLAGS = -DGL_GLEXT_PROTOTYPES $(INCLUDE_FLAGS)

ifeq ($(UNAME), Darwin)
    LIBS = -framework OpenGL -framework Cocoa
else
    LIBS = -lXt -lX11 -lGL -lm
endif

### calculate source files 
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
LIB_FILES = $(foreach dir, $(LIB_DIRS), $(wildcard $(dir)/*.cpp))
C_FILES = $(foreach dir, $(C_DIRS), $(wildcard $(dir)/*.c))
ifeq ($(UNAME), Darwin)
    C_FILES += $(foreach dir, $(C_DIRS), $(wildcard $(dir)/*.m))
endif
SHADER_FILES = $(wildcard $(SHADER_DIR)/*)
MODEL_FILES = $(wildcard $(MODEL_DIR)/*)
TEXTURE_FILES = $(wildcard $(TEXTURE_DIR)/*)

## calculate dependencies
DEPS_SRC = $(LIB_FILES) $(C_FILES) $(SHADER_FILES)
DEPS_LIB = $(foreach dir, $(LIB_DIRS), $(wildcard $(dir)/*.hpp))
DEPS_C = $(foreach dir, $(C_DIRS), $(wildcard $(dir)/*.h))

### setup target dirs
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
OBJ_DIR_SRC = $(OBJ_DIR)/$(SRC_DIR)
OBJ_DIR_LIB = $(OBJ_DIR)/lib
OBJ_DIR_C = $(OBJ_DIR)/c
#SHADER_TARGET_DIR = $(BUILD_DIR)/$(SHADER_DIR)
MODEL_TARGET_DIR = $(BUILD_DIR)/$(MODEL_DIR)
TEXTURE_TARGET_DIR = $(BUILD_DIR)/$(TEXTURE_DIR)
PROGRAM_TARGET_DIR = $(BUILD_DIR)/programs
#BUILD_DIRS = $(BUILD_DIR) $(OBJ_DIR_SRC) $(OBJ_DIR_LIB) $(OBJ_DIR_C) $(SHADER_TARGET_DIR) $(MODEL_TARGET_DIR) $(TEXTURE_TARGET_DIR) $(PROGRAM_TARGET_DIR)
BUILD_DIRS = $(BUILD_DIR) $(OBJ_DIR_SRC) $(OBJ_DIR_LIB) $(OBJ_DIR_C) $(MODEL_TARGET_DIR) $(TEXTURE_TARGET_DIR) $(PROGRAM_TARGET_DIR)
CFLAGS += -I$(BUILD_DIR) 

### calculate target files
OBJ_SRC_FILES = $(addprefix $(OBJ_DIR_SRC)/, $(patsubst %.cpp, %.o, $(notdir $(SRC_FILES))))
OBJ_LIB_FILES = $(addprefix $(OBJ_DIR_LIB)/, $(patsubst %.cpp, %.o, $(notdir $(LIB_FILES))))
OBJ_C_FILES = $(addprefix $(OBJ_DIR_C)/, $(patsubst %.m, %.o, $(patsubst %.c, %.o, $(notdir $(C_FILES)))))
EXE_FILES = $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp, %.exe, $(notdir $(SRC_FILES))))
#SHADER_TARGETS = $(addprefix $(SHADER_TARGET_DIR)/, $(notdir $(SHADER_FILES)))
MODEL_TARGETS = $(addprefix $(MODEL_TARGET_DIR)/, $(notdir $(MODEL_FILES)))
TEXTURE_TARGETS = $(addprefix $(TEXTURE_TARGET_DIR)/, $(notdir $(TEXTURE_FILES)))
DEPS_BUILD = $(OBJ_LIB_FILES) $(OBJ_C_FILES)
SBT_MAKE = $(BUILD_DIR)/makefile

### define functions
cc_compile = $(CC) -c -o $(1) $(2) $(CFLAGS)
cxx_compile = $(CXX) -c -o $(1) $(2) $(CFLAGS) $(LIBS)
link = $(CXX) -o $(1) $(2) $(LIBS) 

### define rules
.SECONDARY: $(OBJ_SRC_FILES) $(OBJ_LIB_FILES) $(OBJ_C_FILES)

all: $(SBT_MAKE) $(BUILD_DIRS) $(EXE_FILES) $(MODEL_TARGETS) $(TEXTURE_TARGETS) 

$(SBT_MAKE): $(BUILD_DIRS) $(PROGRAM_DEF_FILE) $(SHADER_FILES)
	./sbt_make.py $(PROGRAM_DEF_FILE) $(BUILD_DIR)
	make -C $(BUILD_DIR)

$(OBJ_DIR_SRC)/%.o: $(SRC_DIR)/%.cpp $(DEPS_SRC) $(PROGRAM_TARGET_DIR) | $(SBT_MAKE)
		$(call cxx_compile, $@, $<)

$(OBJ_DIR_LIB)/%.o: $(LIB_DIR)/%.cpp $(DEPS_LIB) | $(SBT_MAKE)

		$(call cxx_compile, $@, $<)
$(OBJ_DIR_LIB)/%.o: $(GTL_DIR)/%.cpp $(DEPS_LIB) | $(SBT_MAKE)

		$(call cxx_compile, $@, $<)

$(OBJ_DIR_C)/%.o: common/%.c $(DEPS_C)
		$(call cc_compile, $@, $<)
ifeq ($(UNAME), Darwin)
$(OBJ_DIR_C)/%.o: common/Mac/%.m $(DEPS_C)
		$(call cc_compile, $@, $<)
else
$(OBJ_DIR_C)/%.o: common/Linux/%.c $(DEPS_C)
		$(call cc_compile, $@, $<)
endif

$(BUILD_DIR)/%.exe: $(OBJ_DIR_SRC)/%.o $(DEPS_BUILD) | $(SBT_MAKE)
		$(call link, $@, $^)

$(SHADER_TARGET_DIR)/%: $(SHADER_DIR)/%
	cp $< $@

$(MODEL_TARGET_DIR)/%: $(MODEL_DIR)/%
	cp $< $@

$(TEXTURE_TARGET_DIR)/%: $(TEXTURE_DIR)/%
	cp $< $@

$(BUILD_DIRS):
	mkdir -p $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
