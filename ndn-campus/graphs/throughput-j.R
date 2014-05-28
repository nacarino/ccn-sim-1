#!/usr/bin/Rscript

# Simple R script to make graphs from ndnSIM tracers - Aggregate throughput
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
              help="File which holds the raw rate data.\n\t\t[Default \"%default\"]"),
  make_option(c("-t", "--tcp"), action="store_true", default=FALSE,
              help="Tell the script that the file contains TCP data"),
  make_option(c("-x", "--ndn"), action="store_true", default=FALSE,
              help="Tell the script that the file contains CCN data"),
  make_option(c("-o", "--output"), type="character", default=".",
              help="Output directory for graphs.\n\t\t[Default \"%default\"]")
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

intdata = data

# exclude irrelevant types - CCN
if (opt$ndn) {
  data = subset (data, Type %in% c("InData", "OutData") & Packets > 0)
  intdata = subset(intdata, Type %in% c("InInterests", "OutInterests") & Packets > 0)
}

# exclude irrelevant types - TCP/IP
if (opt$tcp) {
  data = subset (data, Type %in% c("In", "Out", "Drop") & Packets > 0)
}

name = sprintf("Network average throughput of Campus Network, %d campuses, %d server, %d client, %d MB of content transmitted",
               opt$networks, opt$producers, opt$clients, (opt$contentsize /1048576))
  
# combine stats from all faces
data.combined = summaryBy (. ~ Time + Type, data=data, FUN=mean)
  
# graph rates on all nodes in Kilobits
g.all <- ggplot (data.combined, aes (x=Time, y=Kilobits.mean, color=Type)) +
  geom_line(aes (linetype=Type), size=1) + 
  geom_point(aes (shape=Type), size=4) + 
  ggtitle (name) +
  ylab ("Rate [Kbits/s]")
  
# Get the basename of the file
tmpname = strsplit(opt$file, "/")[[1]]
filename = tmpname[length(tmpname)]
  
# Get rid of the extension
noext = gsub("\\..*", "", filename)
  
outpng = ""
# The output png
outpng = sprintf("%s/%s-throughput.png", opt$output, noext)
  
png (outpng, width=1024, height=768)
print (g.all)
x = dev.off ()
  
if (opt$ndn) {
  intname = sprintf("Interest average throughput of Campus Network, %d campuses, %d server, %d client, %d MB of content transmitted",
                    opt$networks, opt$producers, opt$clients, (opt$contentsize /1048576))
    
  intdata.combined = summaryBy (. ~ Time + Type, data=intdata, FUN=mean)
    
  # graph rates on all nodes in Kilobits
  g.int <- ggplot (intdata.combined, aes (x=Time, y=Kilobits.mean, color=Type)) +
    geom_line(aes (linetype=Type), size=1) + 
    geom_point(aes (shape=Type), size=4) +
    ggtitle (intname) +
    ylab ("Rate [Kbits/s]")
    
  outInpng = ""
  # The output png
  outInpng = sprintf("%s/%s-throughput-in.png", opt$output, noext)
    
  png (outInpng, width=1024, height=768)
  print (g.int)
  x = dev.off ()
}