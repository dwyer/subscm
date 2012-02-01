; These are functions described in R5RS as ``library procedures'' meaning they
; can be implemented in Scheme with primitive procedures, so that's what I've
; done. There is of course no requirement that they must be.

; Works similar to assert. I'm thinking about throwing out type-checking for
; arithmatic procedures and letting the passing of non-numbers to them be
; unspecified. It could lead to some interesting hacking but I haven't thought
; too much about the security implications.
(define (check condition who msg . args)
  (if (not condition)
    (apply error (cons who (cons msg args)))))

; This is horribly inefficient and should probably be a C function anyway.
(define (equal? obj1 obj2)
  (if (and (pair? obj1) (pair? obj2))
    (and (equal? (car obj1) (car obj2))
         (equal? (cdr obj1) (cdr obj2)))
    (eqv? obj1 obj2)))

(define (zero? z)
  (check (number? z) 'zero? "not a number" z)
  (= z 0))

(define (positive? x)
  (check (number? x) 'positive? "not a number" x)
  (>= x 0))

(define (negative? x)
  (check (number? x) 'negative? "not a number" x)
  (< x 0))

(define (odd? n)
  (check (integer? n) 'odd? "not a number" n)
  (= (modulo n 2) 1))

(define (even? n)
  (check (integer? n) 'even? "not a number" n)
  (= (modulo n 2) 0))

(define (max x . args)
  (if (null? args)
    x
    (let ((y (car args)))
      (if (and (number? x) (number? y))
        (apply max (cons (if (> x y) x y) (cdr args)))
        (error 'max "arguments must be real numbers" x args)))))

(define (min x . args)
  (if (null? args)
    x
    (let ((y (car args)))
      (if (and (number? x) (number? y))
        (apply min (cons (if (< x y) x y) (cdr args)))
        (error 'min "arguments must be real numbers" x args)))))


(define (abs x)
  (check (number? x) 'abs "not a number" x)
  (if (< x 0)
    (- x)
    x))

;(define (gcd . nn)
;  (if (null? nn)
;    0
;    ...))

; Returns the greatest common divisor or of its arguments.
; The result is always non-negative.
(define (gcd . args)
  (define (euclid a b) ; Euclid's algoritm
    (check (and (integer? a) (integer? b)) 'gcd "args must be itegers" args)
    (if (= b 0)
      a
      (euclid b (remainder a b))))
  (cond ((null? args) 0)
        ((null? (cdr args)) (abs (car args)))
        (else
          (apply gcd (cons (euclid (car args) (cadr args))
                           (cddr args))))))

(define (lcm . args)
  (define (algo a b)
    (/ (abs (* a b)) (gcd a b)))
  (cond ((null? args) 1)
        ((null? (cdr args)) (abs (car args)))
        (else
          (apply lcm (cons (algo (car args) (cadr args))
                           (cddr args))))))
(define (not obj)
  (if obj #f #t))

(define (boolean? obj)
  (or (eq? obj #f) (eq? obj #t)))

(define (null? obj)
  (eq? obj '()))

(define (list? obj)
  (if (null? obj)
    #t
    (and (pair? obj) (list? (cdr obj)))))

; Greatest function ever.
(define (list . objs)
  objs)

(define (length lst)
  (cond ((null? lst) 0)
        ((pair? lst)
         (+ 1 (length (cdr lst))))
        (else (error 'lst "arg must be a list" lst))))

(define (append . args)
  (define (iter lst1 lst2)
    (cond ((null? lst1) lst2)
          ((pair? lst1)
           (cons (car lst1) (iter (cdr lst1) lst2)))
          (else (error 'append "not a list" lst1))))
  (cond ((null? args) '())
        ((null? (cdr args)) (car args))
        (else
          (apply append (cons (iter (car args) (cadr args))
                              (cddr args))))))

;(define (reverse lst)
;  (assert (list? lst))
;  (if (null? lst) lst
;    (if (null? (cdr lst)) lst
;      (cons (car (reverse (cdr lst))) (cons (car lst) '())))))

; Taken from R5RS
(define (list-tail lst k)
  (if (= k 0)
    lst
    (list-tail (cdr lst) (- k 1))))

(define (list-ref lst k)
  (car (list-tail lst k)))

(define (memq obj lst)
  (cond ((null? lst) #f)
        ((not (pair? lst))
         (error 'memq "arg2 must be a list" lst))
        ((eq? obj (car lst)) lst)
        (else (memq obj (cdr lst)))))

(define (memv obj lst)
  (cond ((null? lst) #f)
        ((not (pair? lst))
         (error 'memv "arg2 must be a list" lst))
        ((eqv? obj (car lst)) lst)
        (else (memv obj (cdr lst)))))

(define (member obj lst)
  (cond ((null? lst) #f)
        ((not (pair? lst))
         (error 'member "arg2 must be a list" lst))
        ((equal? obj (car lst)) lst)
        (else (member obj (cdr lst)))))

(define (assq obj lst)
  (cond ((null? lst) #f)
        ((not (and (pair? lst) (pair? (car lst))))
         (error 'assq "arg2 must be a list of pairs" lst))
        ((eq? obj (caar lst)) (car lst))
        (else (assq obj (cdr lst)))))

(define (assv obj lst)
  (cond ((null? lst) #f)
        ((not (and (pair? lst) (pair? (car lst))))
         (error 'assv "arg2 must be a list of pairs" lst))
        ((eqv? obj (caar lst)) (car lst))
        (else (assv obj (cdr lst)))))

(define (assoc obj lst)
  (cond ((null? lst) #f)
        ((not (and (pair? lst) (pair? (car lst))))
         (error 'assoc "arg2 must be a list of pairs" lst))
        ((equal? obj (caar lst)) (car lst))
        (else (assoc obj (cdr lst)))))

; chars are not yet supported
;(define (string . chars) ...)

; implemented internally
;(define (vector . objs)
;  (list->vector objs));

; This is supposed to be implemented internally but I chose to implement
; vector / list->vector instead.
(define (make-vector k . fills)
  (if (and (not (null? fills)) (not (null? (cdr fills))))
    (error 'make-vector "too many arguments"))
  (if (or (not (integer? k)) (< k 0))
    (error 'make-vector "arg1 must be a positive integer" k))
  (define (make-list k fill)
    (if (= k 0)
      '()
      (cons fill (make-list (- k 1) fill))))
  (list->vector (make-list k (if (null? fills) '() (car fills)))))

(define (vector->list vec)
  (if (not (vector? vec))
    (error 'vector->list "arg must be a vector" vec))
  (define (iter k)
    (if (= k (vector-length vec))
      '()
      (cons (vector-ref vec k) (iter (+ k 1)))))
  (iter 0))

(define (list->vector lst)
  (if (not (list? lst))
    (error 'list->vector "arg must be a list" lst))
  (apply vector lst))

(define (vector-fill! vec fill)
  (if (not (vector? vec))
    (error 'vector-fill! "arg1 must be a vector" vec))
  (let ((len (vector-length vec)))
    (define (iter k)
      (if (< k len)
        (begin (vector-set! vec k fill)
               (iter (+ k 1)))
        vec))
    (iter 0)))

(define (map proc . lsts)
  (if (not (procedure? proc))
    (error 'map "arg1 must be a procedure" proc))
  (define (cars lsts)
    (if (null? lsts)
      lsts
      (cons (caar lsts) (cars (cdr lsts)))))
  (define (cdrs lsts)
    (if (null? lsts)
      lsts
      (cons (cdar lsts) (cdrs (cdr lsts)))))
  (if (or (null? lsts) (null? (car lsts)))
    '()
    (cons (apply proc (cars lsts))
          (apply map (cons proc (cdrs lsts))))))

(define (for-each proc . lsts)
  (apply map (cons proc lsts))
  '())

;(define (force promise) ...)

(define (caar obj)
  (car (car obj)))

(define (cadr obj) 
  (car (cdr obj)))

(define (cdar obj) 
  (cdr (car obj)))

(define (cddr obj)
  (cdr (cdr obj)))

(define (caaar obj)
  (car (car (car obj))))

(define (caadr obj)
  (car (car (cdr obj))))

(define (cadar obj)
  (car (cdr (car obj))))

(define (caddr obj)
  (car (cdr (cdr obj))))

(define (cdaar obj)
  (cdr (car (car obj))))

(define (cdadr obj)
  (cdr (car (cdr obj))))

(define (cddar obj)
  (cdr (cdr (car obj))))

(define (cdddr obj)
  (cdr (cdr (cdr obj))))

(define (caaaar obj)
  (car (car (car (car obj)))))

(define (caaadr obj)
  (car (car (car (cdr obj)))))

(define (caadar obj)
  (car (car (cdr (car obj)))))

(define (caaddr obj)
  (car (car (cdr (cdr obj)))))

(define (cadaar obj)
  (car (cdr (car (car obj)))))

(define (cadadr obj)
  (car (cdr (car (cdr obj)))))

(define (caddar obj)
  (car (cdr (cdr (car obj)))))

(define (cadddr obj)
  (car (cdr (cdr (cdr obj)))))

(define (cdaaar obj)
  (cdr (car (car (car obj)))))

(define (cdaadr obj)
  (cdr (car (car (cdr obj)))))

(define (cdadar obj)
  (cdr (car (cdr (car obj)))))

(define (cdaddr obj)
  (cdr (car (cdr (cdr obj)))))

(define (cddaar obj)
  (cdr (cdr (car (car obj)))))

(define (cddadr obj)
  (cdr (cdr (car (cdr obj)))))

(define (cdddar obj)
  (cdr (cdr (cdr (car obj)))))

(define (cddddr obj)
  (cdr (cdr (cdr (cdr obj)))))
