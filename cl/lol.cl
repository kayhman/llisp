(let ((a 1) (b 2))
  (+ a b))

(let ((lapin 12))
   (progn
	(defun boubou (a b)
		(progn 
			(setq lapin (+ lapin 1))
			(+ a b lapin)))
   	(boubou 1 2)))

(boubou 1 2)
(boubou 1 2)
(boubou 1 2)
