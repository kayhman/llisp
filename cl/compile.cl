(defun ab (a b)
  (- (* a 2) (+ 3 b (/ b 5 2))))

(defun fibo (N)
  (if (< N 2)
    1
    (+ (fibo (- N 1))
       (fibo (- N 2)))))

(ab 4 3)
;(funcode "ab")
;(funcode "fibo")
;(compile "ab")
(ab 5 3)
