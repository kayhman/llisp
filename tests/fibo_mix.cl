(defun test (x) 
	(< x 2))


(defun fibo (N)
	(if (test N)
		1
		(+ (fibo (- N 1))
			 (fibo (- N 2)))))


(time (fibo 15))
(compile "test")
(compile "fibo")
(time (fibo 40))

