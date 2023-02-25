os   = linux
arch = x86_64
tbox = external/tbox

CFLAGS = -pipe

debug: CFLAGS += -g -O0
debug: APPFLAGS += -D__tb_debug__
debug: build/debug/app
	@$^ < test.r

release: build/release/app
	@$^ < test.r

build/%/app: APPFLAGS += -lm -Ibuild/$* -Isrc -I$(tbox)/src
build/%/app: src/main.c build/%/parser.c build/%/libtbox.a | build/%/
	@echo cc '' $@
	@$(CC) $(CFLAGS) $(APPFLAGS) $^ -o $@

build/%/libtbox.a: $(tbox)/build/$(os)/$(arch)/%/libtbox.a | build/%/
	@cp $< $@
	@cp $(tbox)/build/$(os)/$(arch)/$*/tbox.config.h build/$*/

$(tbox)/build/$(os)/$(arch)/%/libtbox.a:
	cd $(tbox) && ./configure --mode=$* --demo=no && make

build/%/parser.c: src/parser.peg | build/packcc build/%/
	@echo peg $<
	@cd build/$*/ && ../packcc -o parser ../../$^
 
build/packcc: external/packcc/src/packcc.c | build/
	@echo cc '' $@
	@$(CC) $(CFLAGS) $^ -o $@

%/:
	@mkdir -p $@

clean:
	@rm -rf build/

.PHONY: clean debug release
.PRECIOUS: %/ build/%/libtbox.a build/%/parser.c   
