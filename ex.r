(include <stdio.h>)
(include "bar.h")

(extern fn bar int ((x int) (y int)))

(fn foo (x (y int)) x)

(fn add int ((a int) b) . foo a b)

(fn add i32 ((a i32) b) b)

(fn i1 i32 () 1)
(fn i2 int () 1)

(fn main int ()
  (add (i1) 2)
  )
