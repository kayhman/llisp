(setq lapin (lambda (e f) (concat e f)))
(when 
    (and 
     (eq "pierrepaul" (funcall lapin "pierre" "paul"))
     (not (let ((a 23))))
     (let ((loup 12)
	   (e 13)
	   (f 14)
	   (lapin (lambda (e f) (+ e f))))
       
       (and 
	(eq 27 (+ e f))
	(eq 8 (funcall lapin 5 3)))))
  (print "let succeed"))
(exit)



