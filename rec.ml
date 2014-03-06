(defun fac (x)
	(if (eq x 0)
		1
		(fac (- x 1)) ))	

(fac 3)
