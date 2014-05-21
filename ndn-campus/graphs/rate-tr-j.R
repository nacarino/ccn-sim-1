#!/usr/bin/Rscript

# Simple R script to make graphs from ndnSIM tracers - Rate tracing
# 2014-05-12
# Jairo Eduardo Lopez

suppressPackageStartupMessages(library (ggplot2))
suppressPackageStartupMessages(library (scales))
suppressPackageStartupMessages(library (optparse))
suppressPackageStartupMessages(library (doBy))

# set some reasonable defaults for the options that are needed
option_list <- list (
  make_option(c("-p", "--producers"), type="integer", default=1,
              help="Number of servers (producers) which will be displayed on\n\t\tthe graph title."),
  make_option(c("-c", "--clients"), type="integer", default=1,
              help="Number of the clients (consumers) which will be displayed\n\t\ton the graph title."),
  make_option(c("-s", "--contentsize"), type="integer", default=0,
              help="Content size which will be displayed on the graph\n\t\ttitle. Input in bytes"),
  make_option(c("-n", "--networks"), type="integer", default=1,
              help="Number of networks which will be displayed on the graph title."),
  make_option(c("-f", "--file"), type="character", default="results/rate-trace.txt",
              help="File which holds the raw drop data.\n\t\t[Default \"%default\"]"),
  make_option(c("-e", "--node"), type="integer", default=-1,
              help="Node data to graph. Default graphs all"),
  make_option(c("-t", "--tcp"), action="store_true", default=FALSE,
              help="Tell the script that the file contains TCP data"),
  make_option(c("-x", "--ndn"), action="store_true", default=FALSE,
              help="Tell the script that the file contains CCN data")
)

# Load the parser
opt = parse_args(OptionParser(option_list=option_list, description="Creates graphs from ndnSIM L3 Data rate Tracer data"))

data = read.table (opt$file, header=T)
data$Node = factor (data$Node)
data$Kilobits <- data$Kilobytes * 8
data$Type = factor (data$Type)

if (! opt$tcp && ! opt$ndn) {
  cat("No rate data type used! Check -t and -x!\n")
  quit('yes')
}

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
