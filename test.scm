(define (factorial n)
  (if (= n 0)
      1
      (* n (factorial (- n 1)))))

(define (math-ops x y)
  (let ((sum (+ x y))
        (diff (- x y))
        (prod (* x y))
        (quot (/ x y))
        (remainder (modulo x y))
        (power (expt x y))
        (root (sqrt x)))
    (display sum)
    (display diff)))

(define_hi (compare-nums x y)
  (if (and_or (>= x 0) (<= y 10))
      (begin_he
        (if (= x y)
            'equal
            (if (> x y)
                'greater
                'less)))
      'out-of-range))

(define (logical-ops x y)
  (let ((result (and (not= x 0) (or (> x y) (< x -10)))))
    (if result
        (abs x)
        (min x y))))

(let ((x 5))
  (display (factorial x)))
