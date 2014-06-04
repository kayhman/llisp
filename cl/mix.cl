(defun sub (e f)
	(- e f))

(add 3 4)

(defun mult (c d)
	(* c d))

(defun mult-add (x y)
	(mult (add x y) 
	      (add x y)))

(mult-add 3 4)

(compile "mult")
(mult-add 3 4)
(mult-add 3 4)

