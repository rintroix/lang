PG() { OK "$@" || ( cd build/; ./packcc -o parser ../"$2"; echo g "$1"; ) }

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

	mkdir -p build/

	CC build/test.o   src/test.c
	LD build/packcc   external/packcc/src/packcc.c
	PG build/parser.c src/parser.peg
	LD build/test     build/test.o
	CC build/main.o   src/main.c
	CC build/parser.o build/parser.c

	./build/test >/dev/null || ./build/test

	LD build/app build/main.o build/parser.o 
}


	# case "$arg" in
	# 	# c|clean) for x in build; do rm -rf "$x"; echo - "$x"; done;; 
	# 	c|clean) clean;; 
	# 	d|debug) "$0"; gdb ./build/app;;
	# 	r|run)   "$0"; ./build/app < ex.r;;
	# 	t|tools) bear -- "$0" c b;; 
	# 	b|build) "$0";;
	# 	*) echo >&2 WAT: "$@"; exit 1;;
	# esac

