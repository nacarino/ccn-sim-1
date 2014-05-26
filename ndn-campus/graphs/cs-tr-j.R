#!/usr/bin/Rscript

# Simple R script to make graphs from ndnSIM tracers - Content Store trace
# 2014-05-23
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
  make_option(c("-n", "--networks"), type="integer", default=1,
              help="Number of networks which will be displayed on the graph title."),
  make_option(c("-f", "--file"), type="character", default="results/cs-trace.txt",
              help="File which holds the raw content store data.\n\t\t[Default \"%default\"]"),
  make_option(c("-e", "--node"), type="character", default="",
              help="Node data to graph. Can be a comma separated list.\n\t\tDefault graphs data for all nodes."),
  make_option(c("-o", "--output"), type="character", default=".",
              help="Output directory for graphs.\n\t\t[Default \"%default\"]")
  )

# Load the parser
opt = parse_args(OptionParser(option_list=option_list, description="Creates graphs from ndnSIM Content Store Tracer data"))

data = read.table (opt$file, header=T)
data$Node = factor (data$Node)
data$Type = factor (data$Type)

sel = c()

times = factor(data$Time)

timevals = levels(times)

nTimes = length(timevals)[1]
nNodes = length(levels(data$Node))[1]

N = nTimes*nNodes

df = data.frame(Time=rep(NA, N), Node=rep(NA, N), Hit=rep(NA,N), Miss=rep(NA,N), HitRatio=rep(NA,N), MissRatio=rep(NA,N))

pos = 0

for (i in 1:nTimes) {
  iset = subset(data, Time %in% timevals[i])
  
  for (j in 0:(nNodes-1)) {
    
    jset = subset(iset, Node %in% j)
    
    hits = subset(jset, Type %in% "CacheHits")
    misses = subset(jset,  Type %in% "CacheMisses")
    
    h = hits$Packets
    m = misses$Packets
    denom = h + m
    
    hR = 0
    mR = 0
    if (denom > 0) {
      hR = h / denom
      hR = m / denom
    }
    
    df[pos,] = c(i, j, h, m, hR, mR)
    pos = pos + 1
  }
}


# Check that there are Hit or Miss numbers
for (i in 0:(nNodes-1)) {
  add = FALSE
  if (mean(df[df$Node %in% i,]$Hit) > 0) {
    add = TRUE
  }
  if (mean(df[df$Node %in% i,]$Miss) > 0) {
    add = TRUE
  }
  
  if (add) {
    sel = append(sel, i)
  }
}

filnodes = unlist(strsplit(opt$node, ","))

if (length(sel) != 0) {
  name = ""
  
  df$Node = factor (df$Node)
  
  if (nchar(opt$node) > 0) {
    df = subset (df, Node %in% filnodes)
    
    if (dim(data)[1] == 0) {
      cat(sprintf("There is no Node %s in this trace!", opt$node))
      quit("yes")
    }
    name = sprintf("Cache Hit Rates %% on Node %d of Campus Network, %d campuses, %d server, %d client",
                   opt$node, opt$networks, opt$producers, opt$clients)
  } else {
    # Filter with sel
    df = subset(df, Node %in% sel)
    
    name = sprintf("Cache Hit Rates %% for Campus Network, %d campuses, %d server, %d client",
                   opt$networks, opt$producers, opt$clients)
  }
  
  # graph cache hit rates on all nodes
  g.all <- ggplot (df, aes(colour=Action)) +
    geom_line(aes (x=Time, y=HitRatio, colour="Hit")) + 
    geom_line(aes (x=Time, y=MissRatio, colour="Miss")) +
    ylab ("Cache Hit/Miss Rate (%)") +
    ggtitle (name) +
    facet_wrap (~Node) +
    theme_bw ()
  
  # Get the basename of the file
  tmpname = strsplit(opt$file, "/")[[1]]
  filename = tmpname[length(tmpname)]
  # Get rid of the extension
  noext = gsub("\\..*", "", filename)
  
  # The output png
  if (nchar(opt$node) > 0) {
    outpng = sprintf("%s/%s-%s.png", opt$output, noext, paste(filnodes, sep="", collapse="_"))
  } else {
    outpng = sprintf("%s/%s.png", opt$output, noext)
  }
  
  png (outpng, width=1024, height=768)
  print (g.all)
  x = dev.off ()
} else {
  cat("There are no hits or misses in Cache in this simulation!\n")
}


