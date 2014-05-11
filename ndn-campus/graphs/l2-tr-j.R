#!/usr/bin/Rscript

# Simple R script to make graphs from ndnSIM tracers
# 2014-05-11
# Jairo Eduardo Lopez

library (ggplot2)
library (scales)
library (getopt)

spec = matrix(c(
  'help', 'h', 0, "logical",
  'producers', 'p', 1, "integer",
  'clients', 'c', 1, "integer",
  'contentsize', 's', 1, "integer"
  'networks', 'n', 1, "integer"
  'file', 'f', 1, "character"
), byrow=TRUE, ncol=6);

opt = getopt(spec);

# if help was asked for print a friendly message
# and exit with a non-zero error code
if (!is.null(opt$help)) {
  cat(getopt(spec, usage=TRUE));
  q(status=1);
}

#set some reasonable defaults for the options that are needed,
#but were not specified.
if (is.null(opt$producers)) { opt$producers = 1 }
if (is.null(opt$clients)) { opt$clients = 1 }
if (is.null(opt$contentsize)) { opt$contentsize = 0 }
if (is.null(opt$file)) { opt$file = "results/" }
if (is.null(opt$verbose ) ) { opt$verbose = FALSE }

#print some progress messages to stderr, if requested.
if ( opt$verbose ) { write("writing...",stderr()); }

#do some operation based on user input.
cat(paste(rnorm(opt$count,mean=opt$mean,sd=opt$sd),collapse="\n"));
cat("\n");