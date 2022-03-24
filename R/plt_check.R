#' Check if a file is an image from a supported format
#'
#' `plt_check()` is a wrapper around the C++ function `stbi_info` from
#' `stb_image.h`. It checks whether a file is an image from a supported format.
#' This is useful for checking whether `plt_tize()` can create a color palette
#' from a file without having to decode the entire file.
#'
#' Future versions of this function will expose how to query the width, height,
#' and component count of an image.
#'
#' @usage
#' plt_check(path)
#'
#' @param path A path to a file to be checked.
#'
#' @return
#' `plt_check()` returns `TRUE` if the file is an image from a supported format,
#' and returns `FALSE` for non-image files and images from unsupported formats.
#'
#' @rdname plt_check
#' @export
plt_check <- function(path) {
  path <- normalizePath(path)
  plt_check_(path)
}
