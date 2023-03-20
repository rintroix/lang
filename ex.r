(extern fn bar int ((x int) (y int)))

(fn foo (x (y int)) x)
(fn add int (a b) . foo a b)

(fn main int ()
  (bar 2 (add 3 100500)))
