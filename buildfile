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

	LD packcc external/packcc/src/packcc.c

	CC src/test.c
	PG src/parser.peg
	CC parser.c

	LD test build/test.o

	# DO test >/dev/null || DO test

	wait
	CC src/main.c
	wait
	LD app build/parser.o build/main.o 
	wait
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

