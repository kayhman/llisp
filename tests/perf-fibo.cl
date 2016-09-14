(defun fibo (N)
  (if (< N 3)
    1
    (+ (fibo (- N 1))
       (fibo (- N 2)))))


(time (fibo 25))
(compile "fibo")
(time (fibo 25))
(fibo 25)
