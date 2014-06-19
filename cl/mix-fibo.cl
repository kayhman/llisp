(defun inf (a b)
	(< a b))

(defun fibo (N)
  (if (inf N 2)
    1
    (+ (fibo (- N 1))
       (fibo (- N 2)))))


(compile "fibo")
(time (fibo 15))
