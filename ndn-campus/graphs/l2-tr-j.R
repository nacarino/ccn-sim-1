#!/usr/bin/Rscript

# Simple R script to make graphs from ndnSIM tracers
# 2014-05-11
# Jairo Eduardo Lopez

# Load packages but supress output
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
  make_option(c("-f", "--file"), type="character", default="results/drop-trace.txt",
              help="File which holds the raw drop data.\n\t\t[Default \"%default\"]"),
  make_option(c("-e", "--node"), type="integer", default=-1,
              help="Node data to graph. Default graphs all")
  )

# Load the parser
opt = parse_args(OptionParser(option_list=option_list, description="Creates graphs from ndnSIM L2 Tracer data"))

data = read.table (opt$file, header=T)
data$Node = factor (data$Node)
data$Kilobits <- data$Kilobytes * 8
data$Type = factor (data$Type)

sel = c()

# Check that the mean is above 0, if not discard graphing
for (i in 0:(length(levels(data$Node))-1)) {
  if (mean(data[data$Node %in% i,]$Kilobytes) > 0) {
    sel = append(sel, i)
  }
}

if (length(sel) != 0) {
  
  name = ""
  if (opt$node >= 0) {
    data = subset (data, Node %in% opt$node)
    
    if (dim(data)[1] == 0) {
      cat(sprintf("There is no Node %d in this trace!", opt$node))
      quit("yes")
    }
    name = sprintf("Drop of Node %d of Campus Network, %d campuses, %d server, %d client, %d MB of content transmitted",
                   opt$node, opt$networks, opt$producers, opt$clients, (opt$contentsize /1048576))
  } else {
    # Filter with sel
    data = data[data$Node %in% sel]
    
    name = sprintf("Drop rate of Campus Network, %d campuses, %d server, %d client, %d MB of content transmitted",
                   opt$networks, opt$producers, opt$clients, (opt$contentsize /1048576))
  }
  
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
    outpng = sprintf("%s-%02d-%03d-%03d-%*d.png", noext,
                     opt$networks, opt$producers, opt$clients,
                     12, opt$contentsize)
  }
    
  png (outpng, width=1024, height=768)
  print (g.all)
  x = dev.off ()
} else {
  cat("There are no dropped packets in this simulation!\n")
}