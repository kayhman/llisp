(append (list 1 2) (list 3 4))
(append (list 1 2) (quote (3 4)))
(append (list 1 2) '(3 4))

(eq (list 1 2 3) (list 1 2 3))
