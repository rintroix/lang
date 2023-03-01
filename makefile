os   = linux
arch = x86_64
tbox = external/tbox

CFLAGS = -pipe

debug: CFLAGS += -g -O0
debug: APPFLAGS += -D__tb_debug__ -DDEBUG
debug: build/debug/app
	@$< < test.r

release: build/release/app
	@$< < test.r

cpp: APPFLAGS += -lm -Iexternal/klib -Ibuild/debug -Isrc -I$(tbox)/src 
cpp:
	@cpp $(CFLAGS) $(APPFLAGS) src/main.c | clang-format 

build/%/app: APPFLAGS += -lm -Iexternal/klib -Ibuild/$* -Isrc -I$(tbox)/src
build/%/app: src/main.c build/%/parser.c build/%/libtbox.a | build/%/
	@echo + $@
	@$(CC) $(CFLAGS) $(APPFLAGS) $^ -o $@

build/%/libtbox.a: $(tbox)/build/$(os)/$(arch)/%/libtbox.a | build/%/
	@cp $< $@
	@cp $(tbox)/build/$(os)/$(arch)/$*/tbox.config.h build/$*/

$(tbox)/build/$(os)/$(arch)/%/libtbox.a:
	cd $(tbox) && ./configure --mode=$* --demo=no && make

build/%/parser.c: src/parser.peg src/common.h | build/packcc build/%/
	@echo + $@
	@cd build/$*/ && ../packcc -o parser ../../$<
 
build/packcc: external/packcc/src/packcc.c | build/
	@echo + $@
	@$(CC) $(CFLAGS) $^ -o $@

test: build/test
	@./$<

build/test: CFLAGS += -g -O0
build/test: src/test.c src/um.h | build/
	@echo + $@
	@$(CC) $(CFLAGS) $< -o $@

%/:
	@mkdir -p $@

clean:
	@rm -rf build/

tools: clean
	@bear -- make

.PHONY: clean debug release tools test
.PRECIOUS: %/ build/%/libtbox.a build/%/parser.c   
