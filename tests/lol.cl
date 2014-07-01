(let ((lapin 12))
  (progn
    (defun boubou (a b)
      (progn 
	(setq lapin (+ lapin 1))
	(+ a b lapin)))))

(when (and 
       (eq 3 (let ((a 1) (b 2))
	       (+ a b)))
       (eq 16 (boubou 1 2))
       (eq 17 (boubou 1 2))
       (eq 18 (boubou 1 2)))
  (print "lol succeed ;)"))
(exit)
