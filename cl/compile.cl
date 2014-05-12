(defun ab (a b)
  (- (* a 2) (+ 3 b (/ b 5 2))))

(defun bb (a b)
  (< a b))


(defun fibo (N)
  (if (< N 2)
    1
    (+ (fibo (- N 1))
       (fibo (- N 2)))))

(time (ab 5 3))
(compile "ab")
(time (ab 5 3))
(time (ab 6 8))

(bb 12 16)
(compile "bb")
(bb 13 16)
(bb 18 16)
