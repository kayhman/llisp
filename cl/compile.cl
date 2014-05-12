(defun ab (a b)
  (+ a (+ 3 b)))

(defun fibo (N)
  (if (< N 2)
    1
    (+ (fibo (- N 1))
       (fibo (- N 2)))))

(ab 4 3)
(funcode "ab")
(funcode "fibo")
(compile "ab")
(ab 5 3)
