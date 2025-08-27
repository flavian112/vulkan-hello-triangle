# config
APP       ?= vulkan-hello-triangle
RUN_ARGS  ?=
BUILD     ?= release
SRCDIR    ?= src
INCDIR    ?= include
SHADERDIR ?= shaders
BUILDDIR  ?= build
OUT       := $(BUILDDIR)/$(APP)

# tools
CC     ?= clang
GLSLC  ?= glslc

# pkg-config
PKG        ?= vulkan glfw3
PKG_CFLAGS := $(shell pkg-config --cflags $(PKG))
PKG_LIBS   := $(shell pkg-config --libs   $(PKG))

# sources, objects, dependencies
SOURCES := $(shell find $(SRCDIR) -type f -name '*.c' -print)
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))
DEPS    := $(OBJECTS:.o=.d)

# shaders
SHADER_EXTS    := vert frag comp geom tesc tese
SHADER_SOURCES := $(sort $(foreach ext,$(SHADER_EXTS),$(shell find $(SHADERDIR) -type f -name '*.$(ext)' -print 2>/dev/null)))
SHADER_OUTDIR  ?= $(BUILDDIR)/shaders
SHADER_SPV     := $(patsubst $(SHADERDIR)/%,$(SHADER_OUTDIR)/%.spv,$(SHADER_SOURCES))

# make
.SUFFIXES:
.DELETE_ON_ERROR:
.DEFAULT_GOAL := all
SHELL := /bin/sh

# flags
CPPFLAGS := -I$(INCDIR) $(PKG_CFLAGS) -MMD -MP -DSHADER_BIN_DIR="$(SHADER_OUTDIR)"
WARN     := -Wall -Wextra -Wpedantic -Wformat=2 -Wformat-security \
            -Wshadow -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes \
            -Wno-unused-parameter
CSTD     := -std=c23
CFLAGS   := $(CSTD) $(WARN)
LDFLAGS  := -Wl,-rpath,$(shell brew --prefix)/lib -Wl,-rpath,$(shell brew --prefix vulkan-validationlayers)/lib
LDLIBS   := $(PKG_LIBS)

ifeq (,$(filter $(BUILD),release debug))
$(error BUILD=$(BUILD) is invalid)
endif

ifeq ($(BUILD),debug)
  CFLAGS  += -O0 -g3 -fsanitize=address,undefined -fno-omit-frame-pointer -UNDEBUG -DDEBUG
  LDLIBS  += -fsanitize=address,undefined
else
  CFLAGS  += -O3 -g -D_FORTIFY_SOURCE=2 -fstack-protector-strong -DNDEBUG
endif

# targets
.PHONY: all run clean distclean format tidy compile_commands shaders help


all: $(OUT)

$(OUT): $(OBJECTS) | $(BUILDDIR)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | shaders $(BUILDDIR)
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $@

run: all
	$(OUT) $(RUN_ARGS)


shaders: $(SHADER_SPV)

$(SHADER_OUTDIR)/%.spv: $(SHADERDIR)/% | $(SHADER_OUTDIR)
	@mkdir -p $(@D)
	$(GLSLC) -o $@ $<

$(SHADER_OUTDIR):
	@mkdir -p $@


format:
	find $(SRCDIR) $(INCDIR) -type f \( -name '*.c' -o -name '*.h' \) -print0 | xargs -0 clang-format -i --verbose

tidy: compile_commands
	clang-tidy -p . $(SOURCES)


compile_commands:
	bear --output compile_commands.json -- $(MAKE) -B all


clean:
	rm -rf $(BUILDDIR)

distclean: clean
	rm -rf compile_commands.json


help:
	@echo "Targets: all (default), run, shaders, format, tidy, compile_commands, clean, distclean"
	@echo "Vars: BUILD=debug|release (default: $(BUILD)), RUN_ARGS."

# auto deps
-include $(DEPS)
