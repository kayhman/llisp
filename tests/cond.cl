(cond 
  ((car (list 1 2 3))
   (print "trop t"))
  (nil
   (print "trop nil"))
  ((eq 12 13)
   (print "coucou"))
  ((eq 12 12)
   (progn
     (print "cond succeed"))))
(exit)
