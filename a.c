(progn 
	(+ 1 2 3)
	(- 1 2 3)
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
