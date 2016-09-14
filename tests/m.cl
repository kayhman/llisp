(defmacro c (a) `(+ 1 2 ,(+ 3 4)))
(defmacro b (a) `(+ 1 2 ,(+ 3 4 ) ,a))
(defmacro double (a b) `(+ (+ ,a ,a) (+ ,b ,b)))
(defun mydouble (a b)
  (double a b))

(defmacro complex (a b)
	(let ((c 12))
		`(+ 1 2 ,c ,(+ a b))))

(when 
    (and
     (eq 10 (c 12))
     (eq 23 (b 13))
     (eq 50 (double 12 13))
     (eq 58 (mydouble 14 15))
     (eq 30 (complex 7 8))
	)
  (print "macro succeed"))

(exit)






