(defmacro copy (a) ,a)

(copy 12)

(defmacro double (a b) (+ (+ ,a ,a) (+ ,b ,b)))

(double 12 13)
