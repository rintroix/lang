(fn add :int (a :int b :int) . a + b)

(fn main :int ()
  (add 1 2)
  (add 3 4))
