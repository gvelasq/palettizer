#' Create a color palette
#'
#' @description
#' `plt_tize()` creates a color palette from a supported image file.
#'
#' @usage
#' plt_tize(path, cluster_count, seed, sort_type = "weight")
#'
#' @param path A path to a supported image file.
#' @param cluster_count The number of clusters for k-means clustering.
#' @param seed An integer to specify the seed for the random number generator.
#' @param sort_type A character vector, one of "weight" (the default), "red", "green",
#' or "blue".
#'
#' @return
#' A character vector of hexadecimal colors.
#'
#' @rdname plt_tize
#' @export
plt_tize <- function(path, cluster_count = 5, seed = 42 , sort_type = "weight") {
  path <- normalizePath(path)
  stopifnot("The seed argument must be an integer or a number coercible to an integer" = is_integerish(seed))
  plt_tize_(path, cluster_count, seed, sort_type)
}
