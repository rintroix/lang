os   = linux
arch = x86_64
tbox = external/tbox

debug: CFLAGS += -g -O0 -D__tb_debug__
debug: build/debug
	@$^ < test.r

release: build/release
	@$^ < test.r

build/%: CFLAGS += -I$(tbox)/build/$(os)/$(arch)/%
build/%: CFLAGS += -lm -Ibuild -Isrc -I$(tbox)/src
build/%: src/main.c build/parser.c $(tbox)/build/$(os)/$(arch)/%/libtbox.a
	$(CC) $(CFLAGS) $^ -o $@

build/parser.c: src/parser.peg | build/packcc
	cd build/ && ./packcc -o parser ../$^
 
build/packcc: external/packcc/src/packcc.c | build
	$(CC) $(CFLAGS) $^ -o $@

build:
	@mkdir $@

clean:
	@rm -rf build/

$(tbox)/build/$(os)/$(arch)/%/libtbox.a:
	@cd $(tbox) && ./configure --mode=$* && make

.PHONY: clean debug release
