(defun mapcar (func l)
 (if (cdr l)
     (progn 
       (append (list (funcall func (car l))) (mapcar func (cdr l))))
   (progn
     (list (funcall func (car l))))))

(defmacro mac (ll)
  `(append ,ll ,(mapcar fun '(1 2 3))))

(setq fun (defun f (a) (+ a 12)))

(when 
    (and
     (eq '(13 14 15) (mapcar fun '(1 2 3)))
     (eq '(13 14 15) (mapcar f '(1 2 3)))
     (eq '(4 5 6 13 14 15) (mac (4 5 6)))
     )
  (print "mapcar succeed"))
