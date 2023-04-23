is_integerish <- function(x, tolerance = .Machine$double.eps^0.5) {
  abs(x - round(x)) < tolerance
}
