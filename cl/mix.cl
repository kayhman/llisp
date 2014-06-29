(defun sub (e f)
	(- e f))

(defun add (e f)
	(+ e f))


(add 3 4)

(defun mult (a b)
	(* a b))

(defun double (a)
  (mult 2 a))

(defun double-add (a b)
  (add (mult 2 a) b))

(defun double-sum (x y)
  (mult (add y x) 2))

(defun mult-add (x y)
	(mult (add x y) 
	      (add x y)))

(mult-add 3 4)
(double 333)
(double-add 333 3)
(double-sum 333 3)

(compile "mult")
(mult 3 4)
(double 333)
(double-add 333 3)
(double-sum 333 3)
