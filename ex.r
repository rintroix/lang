(fn foo int ((x int) (y int)) x)
(fn add int ((a int) (b int)) . foo a b)

(fn main int ()
  (add 1 2)
  (add 3 4))
