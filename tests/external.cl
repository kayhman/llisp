(defun test (e f)
	(+ e f))


(test 3 4)

(defun ff (a b)
	(test 1 1))

(defun gg (c d)
	(test c d))

(defun bou (x y)
	(gg x y))

(bou 3 4)

(compile "ff")
(compile "gg")
(gg 1 7)

(gg 1 7)

(+ (gg 1 7) 12 )

(bou 3 (+ 3 7))
