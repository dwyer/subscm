(define (check condition who msg . args)
  (if (not condition)
    (apply error (cons who (cons msg args)))))

(define (lcm . args)
  (define (algorithm a b)
    (/ (abs (* a b)) (gcd a b)))
  (cond ((null? args) 1)
        ((null? (cdr args)) (abs (car args)))
        (else
          (apply lcm (cons (algorithm (car args) (cadr args))
                           (cddr args))))))

# Taken from R5RS
(define (list-tail lst k)
  (if (= k 0)
    lst
    (list-tail (cdr lst) (- k 1))))

(define (list-ref lst k)
  (car (list-tail lst k)))

(define (make-vector k . args)
  (if (and (not (null? args)) (not (null? (cdr args))))
    (error `make-vector "too many arguments"))
  (if (or (not (integer? k)) (< k 0))
    (error `make-vector "arg1 must be a positive integer" k))
  (define (make-list k fill)
    (if (= k 0)
      nil
      (cons fill (make-list (- k 1) fill))))
  (list->vector (make-list k (if (null? args) nil (car args)))))

(define (vector->list vec)
  (if (not (vector? vec))
    (error `vector->list "arg must be a vector" vec))
  (define (iter k)
    (if (= k (vector-length vec))
      nil
      (cons (vector-ref vec k) (iter (+ k 1)))))
  (iter 0))

(define (list->vector lst)
  (if (not (list? lst))
    (error `list->vector "arg must be a list" lst))
  (apply vector lst))

(define (vector-fill! vec fill)
  (if (not (vector? vec))
    (error `vector-fill! "arg1 must be a vector" vec))
  (let ((len (vector-length vec)))
    (define (iter k)
      (if (< k len)
        (begin (vector-set! vec k fill)
               (iter (+ k 1)))
        vec))
    (iter 0)))

(define (map proc . args)
  (define (map1 lst)
    (if (null? lst)
      nil
      (cons (proc (car lst))
            (map1 (cdr lst)))))
  (define (iter args)
    (if (or (null? args) (null? (car args)))
      nil
      (cons (apply proc (map car args))
            (iter (map cdr args)))))
  (cond ((null? args) nil)
        ((null? (cdr args))
         (map1 (car args)))
        (else (iter args))))

(define (for-each proc . args)
  (define (for-each-unary lst)
    (if (null? lst)
      nil
      (begin (proc (car lst))
             (for-each-unary (cdr lst)))))
  (define (iter args)
    (if (or (null? args) (null? (car args)))
      nil
      (begin (apply proc (map car args))
             (iter (map cdr args)))))
  (cond ((null? args) nil)
        ((null? (cdr args))
         (for-each-unary (car args)))
        (else (iter args))))
