(defun ab (a b)
  (- (* a 2) (+ 3 b (/ b 5 2))))

(defun bb (a b)
  (< a b))

(defun cc (a b)
  (if (< a b) 
	a 
	b))

(defun dd (a b)
  (if (> a b) 
	a 
	b))


(defun fibo0 (N)
  (if (< N 2)
    1
    (+ (fibo (- N 1))
       (- N 2))))

(defun fibo (N)
  (if (< N 2)
    1
    (+ (fibo (- N 1))
       (fibo (- N 2)))))


(time (ab 5 3))

(time (ab 5 3))
(time (ab 6 8))

(bb 12 16)

(bb 13 16)
(bb 18 16)

(compile "cc")
(cc 3 4)
(cc 4 3)

(compile "dd")
(dd 3 4)
(dd 4 3)

(time (fibo 25))
(compile "fibo")

(time (fibo 25))
(time (fibo 40))

(defun ff (a b)
	(test 1 1))

(defun gg (a b)
	(test 1 1))

(compile "ff")
(compile "gg")
(gg 1 2)
