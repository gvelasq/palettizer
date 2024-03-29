% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/plt_tize.R
\name{plt_tize}
\alias{plt_tize}
\title{Create a color palette}
\usage{
plt_tize(path, cluster_count, seed, sort_type = "weight")
}
\arguments{
\item{path}{A path to a supported image file.}

\item{cluster_count}{The number of clusters for k-means clustering.}

\item{seed}{An integer to specify the seed for the random number generator.}

\item{sort_type}{A character vector, one of "weight" (the default), "red", "green",
or "blue".}
}
\value{
A character vector of hexadecimal colors.
}
\description{
\code{plt_tize()} creates a color palette from a supported image file.
}
