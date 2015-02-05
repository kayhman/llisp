(when 
	(eq 4.27 (native "./native.so" "testadd" "fff" 1.12 3.15))
	(print "native succeed"))
(native "./native.so" "testconcat" "sss" "lapin" "loup")
(exit)
