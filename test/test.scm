(load "sub.scm")

; 11.2.1  Variable definitions
(define add3
  (lambda (x) (+ x 3)))
(assert (equal? (add3 3) 6))
(define first car)
(assert (equal? (first '(1 2)) 1))

; 11.3  Bodies
(assert (equal? (let ((x 5))
                  (define foo (lambda (y) (bar x y)))
                  (define bar (lambda (a b) (+ (* a b) a)))
                  (foo (+ x 3))) 45))

; 11.4.1  Quotation
(assert (equal? (quote a) 'a))
(assert (equal? (quote #(a b c)) #(a b c)))
(assert (equal? (quote (+ 1 2)) '(+ 1 2)))
(assert (equal? '"abc" "abc"))
(assert (equal? '145932 145932))
(assert (equal? 'a (quote a)))
(assert (equal? '() (quote ())))
(assert (equal? '(+ 1 2) (quote (+ 1 2))))
(assert (equal? '(quote a) (quote (quote a))))
(assert (equal? ''a (quote (quote a))))

; 11.4.2  Procedures
(lambda (x) (+ x x))
(assert (equal? ((lambda (x) (+ x x)) 4) 8))
(assert (equal? ((lambda (x)
                   (define (p y)
                     (+ y 1))
                   (+ (p x) x))
                 5) 11))
(define reverse-subtract
  (lambda (x y) (- y x)))
(assert (equal? (reverse-subtract 7 10) 3))
(define add4
  (let ((x 4))
    (lambda (y) (+ x y))))

(assert (equal? (add4 6) 10))
(assert (equal? ((lambda x x) 3 4 5 6) '(3 4 5 6)))
(assert (equal? ((lambda (x y . z) z)
                 3 4 5 6) '(5 6)))

; 11.4.3  Conditionals
(assert (equal? (if (> 3 2) 'yes 'no) 'yes))
(assert (equal? (if (> 2 3) 'yes 'no) 'no))
(assert (equal? (if (> 3 2)
                  (- 3 2)
                  (+ 3 2)) 1))
(assert (equal? (if #f #f) #f)) ; unspecified

; 11.4.4  Assignments
(assert (equal? (let ((x 2))
                  (+ x 1)
                  (set! x 4)
                  (+ x 1)) 5))

; 11.4.5  Derived conditionals
(assert (equal? (cond ((> 3 2) 'greater)
                      ((< 3 2) 'less)) 'greater))
(assert (equal? (cond ((> 3 3) 'greater)
                      ((< 3 3) 'less)
                      (else 'equal)) 'equal))
; (assert (equal? (cond ('(1 2 3) => cadr)
;       (else #f)) 2)) ; not supported
(assert (equal?
          (case (* 2 3)
            ((2 3 5 7) 'prime)
            ((1 4 6 8 9) 'composite))
          'composite))
(assert (equal? (case (car '(c d))
                  ((a) 'a)
                  ((b) 'b)) #f))
(assert (equal? (case (car '(c d))
                  ((a e i o u) 'vowel)
                  ((w y) 'semivowel)
                  (else 'consonant)) 'consonant))
(assert (equal? (and (= 2 2) (< 2 1)) #f))
(assert (equal? (and 1 2 'c '(f g)) '(f g)))
(assert (equal? (and) #t))
(assert (equal? (or (= 2 2) (> 2 1)) #t))
(assert (equal? (or (= 2 2) (< 2 1)) #t))
(assert (equal? (or #f #f #f) #f))
(assert (equal? (or '(b c) (/ 3 0)) '(b c)))

; 11.4.6  Binding constructs
(assert (equal? (let ((x 2) (y 3))
                  (* x y)) 6))
(assert (equal? (let ((x 2) (y 3))
                  (let ((x 7)
                        (z (+ x y)))
                    (* z x))) 35))
(assert (equal? (let ((x 2) (y 3))
                  (let* ((x 7)
                         (z (+ x y)))
                    (* z x))) 70))

; 11.4.7  Sequencing
(define x 0)
(assert (equal? (begin (set! x 5)
                       (+ x 1)) 6))
;(assert (equal? (begin (display "4 plus 1 equals ")
;                       (display (+ 4 1))) '())) ; unspecified

; 11.5  Equivalence predicates
(assert (equal? (eqv? 'a 'a) #t))
(assert (equal? (eqv? 'a 'b) #f))
(assert (equal? (eqv? 2 2) #t))
(assert (equal? (eqv? '() '()) #t))
(assert (equal? (eqv? 100000000 100000000) #t))
(assert (equal? (eqv? (cons 1 2) (cons 1 2)) #f))
(assert (equal? (eqv? (lambda () 1)
                      (lambda () 2)) #f))
(assert (equal? (eqv? #f 'nil) #f))
(assert (equal? (let ((p (lambda (x) x)))
                  (eqv? p p)) #t))
(assert (equal? (eqv? "" "") #f))
(assert (equal? (eqv? #() #()) #f))
(assert (equal? (eqv? (lambda (x) x)
                      (lambda (x) x)) #f))
(assert (equal? (eqv? (lambda (x) x)
                      (lambda (y) y)) #f))
(define gen-counter
  (lambda ()
    (let ((n 0))
      (lambda () (set! n (+ n 1)) n))))
(assert (equal? (let ((g (gen-counter)))
                  (eqv? g g)) #t))
(assert (equal? (eqv? (gen-counter) (gen-counter)) #f))
(define gen-loser
  (lambda ()
    (let ((n 0))
      (lambda () (set! n (+ n 1)) 27))))
(assert (equal? (let ((g (gen-loser)))
                  (eqv? g g)) #t))
(assert (equal? (eqv? (gen-loser) (gen-loser)) #f))
(assert (equal? (eqv? '(a) '(a)) #f))
(assert (equal? (eqv? "a" "a") #f))
(assert (equal? (eqv? '(b) (cdr '(a b))) #f))
(assert (equal? (let ((x '(a)))
                  (eqv? x x)) #t))
(assert (equal? (eq? 'a 'a) #t))
(assert (equal? (eq? '(a) '(a)) #f))
(assert (equal? (eq? (list 'a) (list 'a)) #f))
(assert (equal? (eq? "a" "a") #f))
(assert (equal? (eq? "" "") #f))
(assert (equal? (eq? '() '()) #t))
(assert (equal? (eq? 2 2) #f))
(assert (equal? (eq? car car) #t))
(assert (equal? (let ((n (+ 2 3)))
                  (eq? n n)) #t))
(assert (equal? (let ((x '(a)))
                  (eq? x x)) #t))
(assert (equal? (let ((x #()))
                  (eq? x x)) #t))
(assert (equal? (let ((p (lambda (x) x)))
                  (eq? p p)) #t))
(assert (equal? (equal? 'a 'a) #t))
(assert (equal? (equal? '(a) '(a)) #t))
(assert (equal? (equal? '(a (b) c)
                        '(a (b) c)) #t))
(assert (equal? (equal? "abc" "abc") #t))
(assert (equal? (equal? 2 2) #t))
(assert (equal? (equal? (make-vector 5 'a)
                        (make-vector 5 'a)) #t))
(assert (equal? (equal? (lambda (x) x)
                        (lambda (y) y)) #f))

; 11.6  Procedure predicate
(assert (equal? (procedure? car) #t))
(assert (equal? (procedure? 'car) #f))
(assert (equal? (procedure? (lambda (x) (* x x))) #t))
(assert (equal? (procedure? '(lambda (x) (* x x))) #f))

; 11.7  Arithmetic

; 11.7.4.3  Arithmetic operations
(assert (equal? (max 3 4) 4))
(assert (equal? (max 3.9 4) 4.0))
(assert (equal? (+ 3 4) 7))
(assert (equal? (+ 3) 3))
(assert (equal? (+) 0))
(assert (equal? (* 4) 4))
(assert (equal? (*) 1))
(assert (equal? (* 1.0 0) 0))
(assert (equal? (- 3 4) -1))
(assert (equal? (- 3 4 5) -6))
(assert (equal? (- 3) -3))
(assert (equal? (/ 3 4 5) (/ 3 20)))
(assert (equal? (/ 3) (/ 1 3)))
(assert (equal? (/ 0 3.5) 0.0))
(assert (equal? (abs -7) 7))
(assert (equal? (gcd 32 -36) 4))
(assert (equal? (gcd) 0))
(assert (equal? (lcm 32 -36) 288))
(assert (equal? (lcm 32.0 -36) 288.0))
(assert (equal? (lcm) 1))
(assert (equal? (expt 5 3) 125))
; This one passes under normal circumstances but fails in Valgrind.
; (assert (equal? (expt 5 -3) (/ 1 125)))
(assert (equal? (expt 5 0) 1))
(assert (equal? (expt 0 5) 0))
(assert (equal? (expt 0 0) 1))
(assert (equal? (expt 0.0 0.0) 1.0))
; 11.8  Booleans
(assert (equal? (not #t) #f))
(assert (equal? (not 3) #f))
(assert (equal? (not (list 3)) #f))
(assert (equal? (not #f) #t))
(assert (equal? (not '()) #f))
(assert (equal? (not (list)) #f))
(assert (equal? (not 'nil) #f))
(assert (equal? (boolean? #f) #t))
(assert (equal? (boolean? 0) #f))
(assert (equal? (boolean? '()) #f))
; 11.9  Pairs and lists
; ERROR: (assert (equal? (a b c . d) '(a . (b . (c . d)))))
(assert (equal? (pair? '(a . b)) #t))
(assert (equal? (pair? '(a b c)) #t))
(assert (equal? (pair? '()) #f))
(assert (equal? (pair? #(a b)) #f))
(assert (equal? (cons 'a '()) '(a)))
(assert (equal? (cons '(a) '(b c d)) '((a) b c d)))
(assert (equal? (cons "a" '(b c)) '("a" b c)))
(assert (equal? (cons 'a 3) '(a . 3)))
(assert (equal? (cons '(a b) 'c) '((a b) . c)))
(assert (equal? (car '(a b c)) 'a))
(assert (equal? (car '((a) b c d)) '(a)))
(assert (equal? (car '(1 . 2)) '1))
(assert (equal? (cdr '((a) b c d)) '(b c d)))
(assert (equal? (cdr '(1 . 2)) '2))
(assert (equal? (list? '(a b c)) #t))
(assert (equal? (list? '()) #t))
(assert (equal? (list? '(a . b)) #f))
(assert (equal? (list 'a (+ 3 4) 'c) '(a 7 c)))
(assert (equal? (list) '()))
(assert (equal? (length '(a b c)) '3))
(assert (equal? (length '(a (b) (c d e))) '3))
(assert (equal? (length '()) '0))
(assert (equal? (append '(x) '(y)) '(x y)))
(assert (equal? (append '(a) '(b c d)) '(a b c d)))
(assert (equal? (append '(a (b)) '((c))) '(a (b) (c))))
(assert (equal? (append '(a b) '(c . d)) '(a b c . d)))
(assert (equal? (append '() 'a) 'a))
(assert (equal? (reverse '(a b c)) '(c b a)))
(assert (equal? (reverse '(a (b c) d (e (f)))) '((e (f)) d (b c) a)))
(assert (equal? (list-tail '(a b c d) 2) '(c d)))
(assert (equal? (list-ref '(a b c d) 2) 'c))
(assert (equal? (map cadr '((a b) (d e) (g h))) '(b e h)))
(assert (equal? (map (lambda (n) (expt n n))
                     '(1 2 3 4 5)) '(1 4 27 256 3125)))
(assert (equal? (map + '(1 2 3) '(4 5 6)) '(5 7 9)))
(assert (equal? (let ((count 0))
                  (map (lambda (ignored)
                         (set! count (+ count 1))
                         count)
                       '(a b))) '(1 2)))
(assert (equal? (let ((v (make-vector 5)))
                  (for-each (lambda (i)
                              (vector-set! v i (* i i)))
                            '(0 1 2 3 4))
                  v) #(0 1 4 9 16)))
(assert (equal? (for-each (lambda (x) x) '(1 2 3 4)) '()))
(assert (equal? (for-each even? '()) '()))

; 11.10  Symbols
(assert (equal? (symbol? 'foo) #t))
(assert (equal? (symbol? (car '(a b))) #t))
(assert (equal? (symbol? "bar") #f))
(assert (equal? (symbol? 'nil) #t))
(assert (equal? (symbol? '()) #f))
; BUG: (assert (equal? (symbol? #f) #f))
;(assert (equal? (symbol->string 'flying-fish) "flying-fish"))
;(assert (equal? (symbol->string 'Martin) "Martin"))
;(assert (equal? (symbol->string
;                  (string->symbol "Malvina")) "Malvina"))
(assert (equal? (eq? 'mISSISSIppi 'mississippi) #f))

; 11.11  Characters
; 11.12  Strings
; 11.13  Vectors
(assert (equal? #(0 (2 2 2 2) "Anna") #(0 (2 2 2 2) "Anna")))
(assert (equal? (vector 'a 'b 'c) #(a b c)))
(assert (equal? (vector-ref #(1 1 2 3 5 8 13 21) 5) 8))
(assert (equal? (let ((vec (vector 0 '(2 2 2 2) "Anna")))
                  (vector-set! vec 1 '("Sue" "Sue"))
                  vec) #(0 ("Sue" "Sue") "Anna")))
(assert (equal? (vector-set! #(0 1 2) 1 "doe") "doe"))
(assert (equal? (vector->list #(dah dah didah)) '(dah dah didah)))
(assert (equal? (list->vector '(dididit dah)) #(dididit dah)))
; 11.15  Control features
(assert (equal? (apply + (list 3 4)) 7))
(define compose
  (lambda (f g)
    (lambda args
      (f (apply g args)))))
(assert (equal? ((compose sqrt *) 12 75) 30))

; 11.16  Iteration
; SEGFAULT
;(assert (equal? (let loop ((numbers '(3 -2 1 6 -5))
;                           (nonneg '())
;                           (neg '()))
;                  (cond ((null? numbers) (list nonneg neg))
;                        ((>= (car numbers) 0)
;                         (loop (cdr numbers)
;                               (cons (car numbers) nonneg)
;                               neg))
;                        ((< (car numbers) 0)
;                         (loop (cdr numbers)
;                               nonneg
;                               (cons (car numbers) neg))))) '((6 1 3) (-5 -2))))

; force and delay
(assert (equal? (force (delay (+ 1 2))) '3))
(assert (equal? (let ((p (delay (+ 1 2))))
                  (list (force p) (force p))) '(3 3)))

(display "all tests passed!")
(newline)
