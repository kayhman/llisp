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

(defun test (a b)
       (+ a b))

(defun ff (a b)
	(test a b))

(defun gg (a b)
	(ff a b))


(setq r1 (cc 3 4))
(setq r2 (cc 4 3))
(setq r3 (dd 3 4))
(setq r4 (dd 4 3))
(setq r5 (time (fibo 25)))
(setq r6 (gg 1 2))

(compile "cc")
(compile "dd")
(compile "fibo")
(compile "test")

(compile "gg")

(when (and
      (eq r1 (cc 3 4))
      (eq r2 (cc 4 3))
      (eq r3 (dd 3 4))
      (eq r4 (dd 4 3))
      (eq r5 (time (fibo 25)))
      (eq r6 (gg 1 2))
      (compile "ff")
      (eq r6 (gg 1 2)))
      (print "compile succeed"))

(exit)
