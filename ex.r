(fn foo (x (y int)) x)
(fn add (a b) . foo a b)
(fn add float ((a int) (b int)) . foo a b)

(fn main int ()
  (foo 1 2)
  (add 3 4))
