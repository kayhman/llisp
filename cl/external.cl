(defun test (a b)
	(+ a b))


(test 3 4)

(defun ff (a b)
	(test 1 1))

(defun gg (a b)
	(test 1 1))

(compile "ff")
(compile "gg")
(gg 1 2)
