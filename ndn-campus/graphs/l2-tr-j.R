#!/usr/bin/Rscript

# Simple R script to make graphs from ndnSIM tracers
# 2014-05-11
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
  'file', 'f', 1, "character"
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
  opt$file = "results/drop-trace.txt"
  setfile = FALSE
}
if (is.null(opt$verbose ) ) { opt$verbose = FALSE }

#print some progress messages to stderr, if requested.
if ( opt$verbose ) { write("writing...",stderr()); }

data = read.table (opt$file, header=T)
data$Node = factor (data$Node)
data$Kilobits <- data$Kilobytes * 8
data$Type = factor (data$Type)

sel = c()

for (i in 0:(length(levels(data$Node))-1)) {
  if (mean(data[data$Node %in% i,]$Kilobytes) > 0) {
    sel = append(sel, i)
  }
}

if (length(sel) != 0) {
  # Filter with sel
  
  data = data[data$Node %in% sel]
  
  name = sprintf("Drop rate of Campus Network, %d campuses, %d server, %d client, %d MB of content transmitted",
                 opt$networks, opt$producers, opt$clients, (opt$contentsize /1048576))
  
  # graph rates on all nodes in Kilobits
  g.all <- ggplot (data, aes (x=Time, y=Kilobits, color=Type)) +
    geom_point (size=2) +
    geom_line () +
    ylab ("Packet drop rate [Kbits/s]") +
    ggtitle (name) +
    facet_wrap (~ Node) +
    theme_bw ()
  
  # Get the basename of the file
  tmpname = strsplit(opt$file, "/")[[1]]
  filename = tmpname[length(tmpname)]
  # Get rid of the extension
  noext = gsub("\\..*", "", filename)
  
  outpng = ""
  # The output png
  if (setfile) {
    outpng = sprintf("%s.png", noext)
  } else {
    outpng = sprintf("%s-%02d-%02d-%02d-%*d.png", noext,
                     opt$networks, opt$producers, opt$clients,
                     12, opt$contentsize)
  }
  
  
  png (outpng, width=1024, height=768)
  print (g.all)
  x = dev.off ()
} else {
  cat("There are no dropped packets in this simulation!\n")
}