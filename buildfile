alias r=run
alias b=build
alias c=clean
alias t=tools
alias d=debug

run() {
	build && DO app ex.r
}

tools() {
	clean && bear -- "$0"
}

debug() {
	build && lldb $BUILD/app ex.r
}

test() {
	CFLAGS="-Isrc -I$BUILD -g -O0 -DDEBUG -pipe -fno-omit-frame-pointer"

	mkdir -p "$BUILD/"

	RE src/test.c
	LD test build/test.o
	DO test
}

build() {
	CFLAGS="-Isrc -I$BUILD -g -O0 -DDEBUG -pipe -fno-omit-frame-pointer"

	mkdir -p "$BUILD/"

	LD packcc external/packcc/src/packcc.c

	RE src/test.c
	LD test build/test.o

	RE src/parser.peg
	RE src/main.c
	RE parser.c

	DO test >/dev/null || DO test

	LD app build/parser.o build/main.o 
}

