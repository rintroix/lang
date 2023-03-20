alias r=run
alias b=build
alias c=clean
alias t=tools
alias d=debug

run() {
	build && DO app ex.r | tee "$BUILD/ex.c"
	cat > "$BUILD/bar.c" << EOF
		int bar(int x, int y) {
			return x * y + 10;
		}
EOF
	cat > "$BUILD/bar.h" << EOF
		int bar(int x, int y);
EOF
	RE $BUILD/ex.c
	RE $BUILD/bar.c
	LD ex $BUILD/bar.o $BUILD/ex.o 
	if DO ex; then
		echo >&2 RC 0
	else
		echo >&2 RC $?
	fi
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

