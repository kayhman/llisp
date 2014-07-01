(defun fibo (N)
  (if (< N 2)
      1
      (+ (fibo (- N 1))
	 (fibo (- N 2)))))

(setq res (fibo 6))
(compile "fibo")
(setq resc (fibo 6))

(when (and 
       (eq 13 res)
       (eq 13 resc)
       (eq res resc))
  (print "fibo succeed ;)"))
(exit)
