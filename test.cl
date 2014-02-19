(+ 1 2 3)
(+ 6 2 4)
(- 6 2 5)
(* 1 2 3)
(setq lapin 0.666)
(+ lapin lapin lapin)
(* 3 lapin)
(/ 4 2 3)


(defun myfunc (a b) (+ a b))
(myfunc lapin 13)
(myfunc lapin 3)

(setq lapin (myfunc lapin 13))
(myfunc lapin 3)
