`(+ ,a ,b)

(append (list 0 ) '(1 2 3))

(defmacro copyy (a) `,a)
(defmacro copy (a b) 
`(+ ,a ,b ))

(copy 12 13)
(copy 12 13)

(defmacro copypy (a b) 
`(+ ,a ,b (* ,a ,b)))


(copypy 12 13)
