(cond 
	(nil
		(print "trop nil"))
	((eq 12 13)
		(print "coucou"))
  ((eq 12 12)
		(progn
			(print "cond succeed"))))
(exit)
