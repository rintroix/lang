os   = linux
arch = x86_64
tbox = external/tbox

CFLAGS = -pipe

VPATH = build:src:external/packcc/src

debug: CFLAGS += -g -O0
debug: APPFLAGS += -D__tb_debug__ -DDEBUG
debug: M = debug
debug: build/app
	@$< < test.r

release: XXX = $@
release: M = release
release: clean build/app
	@echo I RELEASED $(XXX)

build/release/%.o build/debug/%.o: %.c | build/%/
	@echo o $@
	@$(CC) $(CFLAGS) -c $< -o $@ 

cpp: CFLAGS += -Ibuild -Isrc -I$(tbox)/src 
cpp:
	@cpp $(CFLAGS) $(APPFLAGS) src/main.c | clang-format 

build/main.o: build/tbox.config.h build/parser.h

build/app: CFLAGS += -Ibuild -Isrc -I$(tbox)/src $(APPFLAGS)
build/app: build/main.o build/parser.o build/libtbox.a  
	@echo b $@
	@$(CC) -lm -o $@ $^

build/tbox.config.h: | build/
	@echo c $@
	@cp $(tbox)/build/$(os)/$(arch)/$(M)/tbox.config.h $@
 
build/libtbox.a: | build/
	@echo a $@
	@cp $(tbox)/build/$(os)/$(arch)/$(M)/libtbox.a $@

$(tbox)/build/$(os)/$(arch)/$(M)/libtbox.a:
	cd $(tbox) && ./configure --mode=$* --demo=no && make

build/parser.c build/parser.h &: parser.peg | build/packcc
	@echo g $@
	@cd build && ./packcc -o parser ../$<
 
build/packcc: packcc.c | build/
	@echo b $@
	@$(CC) $^ -o $@

test: build/test
	@./$<

build/test: CFLAGS += -g -O0 -Isrc
build/test: src/test.c | build/
	@echo + $@
	@$(CC) $(CFLAGS) $< -o $@

%/:
	@echo d $@
	@mkdir -p $@

clean:
	@rm -rf build/

tools: clean
	@bear -- make

.PHONY: clean debug release tools test
.SUFFIXES: