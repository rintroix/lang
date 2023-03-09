alias r=run
alias b=build
alias c=clean

run() {
	build && ./build/app < ex.r
}

tools() {
	clean && bear -- "$0"
}

build() {
	CFLAGS='-Isrc -Ibuild -g -O0 -DDEBUG -pipe -fno-omit-frame-pointer'

	mkdir -p "$BUILD/"

	LD packcc external/packcc/src/packcc.c

	RE src/test.c
	RE src/parser.peg
	RE src/main.c
	RE parser.c

	LD test build/test.o

	DO test >/dev/null || DO test

	LD app build/parser.o build/main.o 
}

_rule_t() {
	echo hello world	
}

t() {
	for i in $(seq 1 10); do
		RE bullshit.t
	done
}