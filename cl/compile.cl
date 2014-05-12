(defun ab (a b)
  (- (* a 2) (+ 3 b (/ b 5 2))))

(defun fibo (N)
  (if (< N 2)
    1
    (+ (fibo (- N 1))
       (fibo (- N 2)))))

(time (ab 5 3))
(time (ab 5 3))
(time (ab 5 3))
(time (ab 5 3))
(compile "ab")
(time (ab 5 3))
(time (ab 5 3))
(time (ab 5 3))
