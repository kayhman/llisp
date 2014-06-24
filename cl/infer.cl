(defun inf (a b)
	(< a b))


(defun infb (a b)
	(> a b))

(defun infc (a b)
	(if (> a b)
	  a
	b))

(defun cmp (sa sb)
  (if (> (length sa) (length sb))
    sa
    sb))

(defun cmp (sa sb)
  (if (> (length sa) (length sb))
    (length sa)
    (length sb)))
