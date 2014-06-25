(defun fibo (N)
	(if (< N 2)
		1
		(+ (fibo (- N 1))
			 (fibo (- N 2)))))
(compile "fibo")

(time (fibo 40))
