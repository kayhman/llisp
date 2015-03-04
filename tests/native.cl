(when 
    (and
     (eq 4.27 (native "./native.so" "testadd" "fff" 1.12 3.15))
     (eq "lapinloup" (native "./native.so" "testconcat" "sss" "lapin" "loup"))
     )
  (print "native succeed"))

(exit)
