CC      = clang
AR      = ar
CFLAGS  = -O2 -Wall -Werror -std=c99 -pedantic -I.

SOURCES = $(shell find src -type f -name "*.c")
OBJECTS = $(SOURCES:src/%.c=build/%.o)


# Default make target.
build: build/libtwist.a

# Assemble the static library.
build/libtwist.a: $(OBJECTS)
	@printf "   AR  $@\n"
	@$(AR) cr $@ $(OBJECTS)

# Build individual translation units.
build/%.o: src/%.c
	@printf "   CC  $@\n"
	@mkdir -p $(shell dirname $@)
	@$(CC) -MM $(CFLAGS) $< | sed -e 's|^\(.*\):|build/\1:|' > $(@:.o=.d)
	@$(CC) $(CFLAGS) -c -o $@ $<

-include $(OBJECTS:%.o=%.d)


# Empty the build/ directory.
clean:
	@printf "   rm  build/*\n"
	@rm -rf build/*


.PHONY: build clean
