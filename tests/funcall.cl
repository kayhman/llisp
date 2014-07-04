(setq rabbit (defun lapin (a b c) (+ a b c)))

(let ((offset 5)) 
  (setq roger (lambda (u v) (+ offset (* u v)))))

(when 
    (and
     (eq 6 (lapin 1 2 3))
     (eq (funcall lapin 1 2 3) (lapin 1 2 3))
     (eq (funcall rabbit 1 2 3) (lapin 1 2 3))
     (eq (funcall roger  2.23 3.45) (+ 5 (* 2.23 3.45))))
  (print "funcall succeed"))
(exit)
