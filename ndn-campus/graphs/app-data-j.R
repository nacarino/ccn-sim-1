#!/usr/bin/Rscript

# Simple R script to make graphs from ndnSIM tracers - App delay
# 2014-05-26
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
              help="File which holds the raw app delay data.\n\t\t[Default \"%default\"]"),
  make_option(c("-t", "--tcp"), action="store_true", default=FALSE,
              help="Tell the script that the file contains TCP data"),
  make_option(c("-x", "--ndn"), action="store_true", default=FALSE,
              help="Tell the script that the file contains CCN data"),
  make_option(c("-d", "--delay"), action="store_true", default=FALSE,
              help="Tell the script to graph delay data"),
  make_option(c("-j", "--hop"), action="store_true", default=FALSE,
              help="Tell the script to graph hop data"),
  make_option(c("-o", "--output"), type="character", default=".",
              help="Output directory for graphs.\n\t\t[Default \"%default\"]")
)

# Load the parser
opt = parse_args(OptionParser(option_list=option_list, description="Creates graphs from ndnSIM App Delay Tracer data"))

data = read.table (opt$file, header=T)
data$Node = factor (data$Node)
data$Type = factor (data$Type)

data$TimeSec = 1 * ceiling (data$Time)

if (! opt$tcp && ! opt$ndn) {
  cat("No rate data type used! Check -t and -x!\n")
  quit('yes')
}

# exclude irrelevant types - CCN
if (opt$ndn) {
  data = subset (data, Type %in% c("LastDelay"))
}

# exclude irrelevant types - TCP/IP
if (opt$tcp) {
  data = subset (data, Type %in% c("In", "Out", "Drop"))
}

# Get the basename of the file
tmpname = strsplit(opt$file, "/")[[1]]
filename = tmpname[length(tmpname)]
# Get rid of the extension
noext = gsub("\\..*", "", filename)

if (opt$delay) {
  name = sprintf("Network average delay of Campus Network, %d campuses, %d server, %d client, %d MB of content transmitted",
               opt$networks, opt$producers, opt$clients, (opt$contentsize /1048576))
  
  data.combined = summaryBy (. ~ TimeSec + Type, data=data, FUN=mean)
  # graph rates on all nodes in Kilobits
  g.all <- ggplot (data.combined, aes(colour=Legend)) +
    geom_line (aes (x=TimeSec, y=DelayUS.mean, colour="Avg Delay"), size=1) +
    ggtitle (name) +
    ylab ("Delay [us]")
  
  outpng = sprintf("%s/%s-app-delay.png", opt$output, noext)
  
  png (outpng, width=1024, height=768)
  print (g.all)
  x = dev.off ()
}

if (opt$hop) {
  name = sprintf("Network average data hop of Campus Network, %d campuses, %d server, %d client, %d MB of content transmitted",
                 opt$networks, opt$producers, opt$clients, (opt$contentsize /1048576))
  
  data.combined = summaryBy (. ~ TimeSec + Type, data=data, FUN=mean)
  
  g.all <- ggplot (data.combined, aes(colour=Legend)) +
    geom_line (aes (x=TimeSec, y=HopCount.mean, colour="Avg Hops"), size=1) +
    ggtitle (name) +
    ylab ("Hop Count")
  
  outpng = sprintf("%s/%s-app-hop.png", opt$output, noext)
  
  png (outpng, width=1024, height=768)
  print (g.all)
  x = dev.off ()
}