(defmacro copy (a) `,a)

(copy 12)

(defmacro double (a b) (+ (+ ,a ,a) (+ ,b ,b)))

(double 12 13)
(double 12 13)

(defun mydouble (a b)
  (double a b))
(printenv)
(mydouble 12 13)
(mydouble 12 13)
