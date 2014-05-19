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
  'node', 'e', 1, "integer",
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
if (is.null(opt$verbose)) { opt$verbose = FALSE }
if (is.null(opt$node)) { opt$node = -1 }

if (is.null(opt$tcp) && is.null(opt$ndn)) {
  opt$tcp = TRUE
  opt$ndn = FALSE
}

if (is.null(opt$tcp)) {
  opt$tcp = FALSE
}

if (is.null(opt$ndn)) {
  opt$ndn = FALSE
} 

#print some progress messages to stderr, if requested.
if ( opt$verbose ) { write("writing...",stderr()); }

data = read.table (opt$file, header=T)
data$Node = factor (data$Node)
data$Kilobits <- data$Kilobytes * 8
data$Type = factor (data$Type)

# exclude irrelevant types - CCN
if (opt$ndn) {
  data = subset (data, Type %in% c("InInterests", "OutInterests", "InData", "OutData"))
}

# exclude irrelevant types - TCP/IP
if (opt$tcp) {
  data = subset (data, Type %in% c("In", "Out", "Drop"))
}

name = ""
# Filter for a particular node
if (opt$node >= 0) {
  data = subset (data, Node %in% opt$node)
  
  if (dim(data)[1] == 0) {
    cat(sprintf("There is no Node %d in this trace!", opt$node))
    quit("yes")
  }
  name = sprintf("Data rate of Node %d of Campus Network, %d campuses, %d server, %d client, %d MB of content transmitted",
                 opt$node, opt$networks, opt$producers, opt$clients, (opt$contentsize /1048576))
} else {
  name = sprintf("Data rate of Campus Network, %d campuses, %d server, %d client, %d MB of content transmitted",
                 opt$networks, opt$producers, opt$clients, (opt$contentsize /1048576))
}

# combine stats from all faces
data.combined = summaryBy (. ~ Time + Node + Type, data=data, FUN=sum)

# graph rates on all nodes in Kilobits
g.all <- ggplot (data.combined) +
  geom_line (aes (x=Time, y=Kilobits.sum, color=Type), size=1) +
  ggtitle (name) +
  ylab ("Rate [Kbits/s]") +
  facet_wrap (~ Node)

# Get the basename of the file
tmpname = strsplit(opt$file, "/")[[1]]
filename = tmpname[length(tmpname)]
# Get rid of the extension
noext = gsub("\\..*", "", filename)

outpng = ""
# The output png
if (setfile) {
  if (opt$node > -1) {
    outpng = sprintf("%s-%d.png", noext, opt$node)
  } else {
    outpng = sprintf("%s.png", noext)
  }
} else {
  if (opt$node > -1) {
    outpng = sprintf("%s-%02d-%03d-%03d-%*d-%d.png", noext,
                   opt$networks, opt$producers, opt$clients,
                   12, opt$contentsize, opt$node)
  } else {
    outpng = sprintf("%s-%02d-%03d-%03d-%*d.png", noext,
                     opt$networks, opt$producers, opt$clients,
                     12, opt$contentsize)
  }
}

png (outpng, width=1024, height=768)
print (g.all)
x = dev.off ()
