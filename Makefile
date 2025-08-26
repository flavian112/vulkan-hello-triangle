# project config
APP       := vulkan-hello-triangle
BUILD     ?= release # release | debug
SRCDIR    := src
INCDIR    := include
BUILDDIR  := build

CC        := clang
CSTD      := -std=c23
PKG       := vulkan glfw3
RM        := rm -rf

# tools / extras
TIDY         ?= clang-tidy
GLSLC        ?= glslc

# paths / sources
OUT      := $(BUILDDIR)/$(APP)
SOURCES  := $(shell find $(SRCDIR) -type f -name '*.c')
OBJECTS  := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))
DEPS     := $(OBJECTS:.o=.d)

# shader paths
SHADERDIR   := shaders
VERT_SRC    := $(SHADERDIR)/shader.vert
FRAG_SRC    := $(SHADERDIR)/shader.frag
VERT_SPV    := $(BUILDDIR)/vert.spv
FRAG_SPV    := $(BUILDDIR)/frag.spv
SHADER_SPV  := $(VERT_SPV) $(FRAG_SPV)

# flags
INCLUDES         := -I$(INCDIR)
COMMON_CFLAGS    := -MMD -MP \
                    $(shell pkg-config --cflags $(PKG)) \
                    $(CSTD) $(INCLUDES) \
                    -Wall -Wextra -Wpedantic \
                    -Wformat=2 -Wformat-security \
                    -Wshadow -Wpointer-arith \
                    -Wstrict-prototypes -Wmissing-prototypes \
                    -Wno-unused-parameter
COMMON_LDFLAGS := $(shell pkg-config --libs $(PKG)) \
                  -Wl,-rpath,$(shell brew --prefix)/lib \
                  -Wl,-rpath,$(shell brew --prefix vulkan-validationlayers)/lib

RELEASE_CFLAGS   := -O3 -g -D_FORTIFY_SOURCE=2 -fstack-protector-strong -DNDEBUG
RELEASE_LDFLAGS  :=

DEBUG_CFLAGS     := -O0 -g3 -fsanitize=address,undefined -fno-omit-frame-pointer -UNDEBUG -DDEBUG
DEBUG_LDFLAGS    := -fsanitize=address,undefined

ifeq ($(BUILD),debug)
  CFLAGS  := $(COMMON_CFLAGS) $(DEBUG_CFLAGS)
  LDFLAGS := $(COMMON_LDFLAGS) $(DEBUG_LDFLAGS)
else
  CFLAGS  := $(COMMON_CFLAGS) $(RELEASE_CFLAGS)
  LDFLAGS := $(COMMON_LDFLAGS) $(RELEASE_LDFLAGS)
endif

# targets
.PHONY: all clean run compile_commands format tidy shaders

all: $(OUT) $(SHADER_SPV)

$(OUT): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(OUT) $(SHADER_SPV)
	"$(OUT)" $(RUN_ARGS)

clean:
	$(RM) "$(BUILDDIR)"

# generate compile_commands.json (for clangd / lsp / clang-tidy)
compile_commands:
	$(MAKE) clean
	bear -- $(MAKE) BUILD=$(BUILD)

format:
	find $(SRCDIR) $(INCDIR) -type f \( -name '*.c' -o -name '*.h' \) | xargs clang-format -i

tidy: compile_commands
	$(TIDY) -p . $(SOURCES)

shaders: $(SHADER_SPV)

$(VERT_SPV): $(VERT_SRC)
	@mkdir -p $(dir $@)
	$(GLSLC) -o $@ $<

$(FRAG_SPV): $(FRAG_SRC)
	@mkdir -p $(dir $@)
	$(GLSLC) -o $@ $<

# Auto deps
-include $(DEPS)
