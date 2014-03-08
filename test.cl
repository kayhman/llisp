(progn 
	(eq (+ 1 2 3) 6)
	(eq (- 1 2 3) (- 4)) 
	(* 1 2 3)
	(/ 1 2 3))

(setq lapin 0.666)

(- lapin 0.666)

(concat "ba" "ba")

(setq ba "bi")
(concat ba "ba")

(concat ba "bou")
(setq ba "bou")
(concat ba "bou")


(defun mymy (a b) 
	(progn 
         (* a b)))
	
(+ 1 3)
(mymy lapin 2)
(mymy lapin lapin)
(setq lapin 0.777)

(mymy lapin lapin)
(setq lapin (mymy 1 2))
(printenv)

(+ lapin 0)
(mymy lapin lapin)

(lambda (a b) (+ a b))
(setq lapin (lambda (a b) (+ a b)))
(printenv)
(funcall lapin 1 2)

(funcall (lambda (a b) (+ a b)) 3 4)

(defmacro copy (a)
  `,a)

(concat 'lapin `lapin)
