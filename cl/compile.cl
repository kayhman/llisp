(defun ab (a b)
  (+ a b))

(defun fibo (N)
  (if (< N 2)
    1
    (+ (fibo (- N 1))
       (fibo (- N 2)))))

(ab 2 3)
(funcode "ab")
(funcode "fibo")
(compile "ab")
