#!/usr/bin/Rscript

# Simple R script to make graphs from ndnSIM tracers - Rate tracing
# 2014-05-12
# Jairo Eduardo Lopez

library (ggplot2)
library (scales)
library (getopt)
library (doBy)

spec = matrix(c(
  'help', 'h', 0, "logical",
  'producers', 'p', 1, "integer",
  'clients', 'c', 1, "integer",
  'contentsize', 's', 1, "integer",
  'networks', 'n', 1, "integer",
  'file', 'f', 1, "character",
  'tcp', 't', 0, "logical",
  'ndn', 'd', 0, "logical"
), byrow=TRUE, ncol=4);

opt = getopt(spec);

# if help was asked for print a friendly message
# and exit with a non-zero error code
if (!is.null(opt$help)) {
  cat(getopt(spec, usage=TRUE));
  q(status=1);
}

setfile = TRUE

#set some reasonable defaults for the options that are needed,
#but were not specified.
if (is.null(opt$producers)) { opt$producers = 1 }
if (is.null(opt$clients)) { opt$clients = 1 }
if (is.null(opt$contentsize)) { opt$contentsize = 0 }
if (is.null(opt$networks)) { opt$networks = 1 }
if (is.null(opt$file)) {
  opt$file = "results/rate-trace.txt"
  setfile = FALSE
}
if (is.null(opt$verbose ) ) { opt$verbose = FALSE }

#print some progress messages to stderr, if requested.
if ( opt$verbose ) { write("writing...",stderr()); }

data = read.table (opt$file, header=T)
data$Node = factor (data$Node)
data$Kilobits <- data$Kilobytes * 8
data$Type = factor (data$Type)
