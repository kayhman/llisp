(defun test (a b)
	(+ a b))


(test 3 4)

(defun ff (a b)
	(test 1 1))

(defun gg (c d)
	(test 1 7))

(defun bou (a b)
	(gg a b))

(bou 3 4)

(compile "ff")
(compile "gg")
(gg 1 7)

(bou 3 4)
