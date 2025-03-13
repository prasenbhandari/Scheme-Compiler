(define (factorial n)
  (if (= n 0)
      1
      (* n (factorial (- n 1)))))

(define (fibonacci n)
  (cond ((= n 0) 0)
        ((= n 1) 1)
        (else (+ (fibonacci (- n 1))
                 (fibonacci (- n 2))))))

(define test-list '(1 2 3 4 5))

(define (map-custom func lst)
  (if (null? lst)
      '()
      (cons (func (car lst))
            (map-custom func (cdr lst)))))

(let ((x 10)
      (y 20))
  (+ x y))

(define person '((name . "John")
                 (age . 30)
                 (city . "New York")))

(define (square x) (* x x))

(display "Hello, World!")
(newline)

(define result 
  (map-custom square test-list))

(if (> 5 3)
    (display "Five is greater than three")
    (display "This won't be printed"))

; Testing different types of numbers
42          ; integer
3.14159     ; float
-17         ; negative
+123        ; positive with explicit sign
#t          ; boolean true
#f          ; boolean false
