#!/usr/bin/Rscript

# Simple R script to make graphs from ndnSIM tracers - Satisfied Interests
# 2014-06-29
# Jairo Eduardo Lopez

suppressPackageStartupMessages(library (ggplot2))
suppressPackageStartupMessages(library (scales))
suppressPackageStartupMessages(library (optparse))
suppressPackageStartupMessages(library (doBy))

rawdata.flood = read.table ("/home/user1/git/ccn-sim-1/ndn-campus/results/CCNWireless-rate-trace-flood-matome.txt", header=T)
rawdata.bestr = read.table ("/home/user1/git/ccn-sim-1/ndn-campus/results/CCNWireless-rate-trace-bestr-matome.txt", header=T)
rawdata.smart = read.table ("/home/user1/git/ccn-sim-1/ndn-campus/results/CCNWireless-rate-trace-smart-matome.txt", header=T)
rawdata.perfect = read.table ("/home/user1/git/ccn-sim-1/ndn-campus/results/CCNWirelessPerfect-rate-trace-flood-01-001-012.txt", header=T)

rawdata.flood$Type = factor (rawdata.flood$Type)
rawdata.bestr$Type = factor (rawdata.bestr$Type)
rawdata.smart$Type = factor (rawdata.smart$Type)
rawdata.perfect$Type = factor (rawdata.perfect$Type)

rawdata.flood = subset (rawdata.flood, Time < 28 )
rawdata.bestr = subset (rawdata.bestr, Time < 28 )
rawdata.smart = subset (rawdata.smart, Time < 28 )
rawdata.perfect = subset (rawdata.perfect, Time < 28)

dataname = "Data packets transmitted"
intname = "Interest packets transmitted"
satname = "Fate of Interest packets"

# combine stats from all faces
flood.combined = summaryBy (. ~ Time + Type, data=rawdata.flood, FUN=sum)
bestr.combined = summaryBy (. ~ Time + Type, data=rawdata.bestr, FUN=sum)
smart.combined = summaryBy (. ~ Time + Type, data=rawdata.smart, FUN=sum)
perfect.combined = summaryBy (. ~ Time + Type, data=rawdata.perfect, FUN=sum)

flood.combined$Variable = "flood"
bestr.combined$Variable = "bestr"
smart.combined$Variable = "smart"
perfect.combined$Variable = "tech"

multiple = 10

if (multiple > 1) {
  flood.combined$PacketRaw.sum = flood.combined$PacketRaw.sum / multiple
  bestr.combined$PacketRaw.sum = bestr.combined$PacketRaw.sum / multiple
  smart.combined$PacketRaw.sum = smart.combined$PacketRaw.sum / multiple
}

flood.data.in = subset (flood.combined, Type %in% c("InData"))
flood.data.out = subset (flood.combined, Type %in% c("OutData"))
flood.int.in = subset (flood.combined, Type %in% c("InInterests"))
flood.int.out = subset (flood.combined, Type %in% c("OutInterests"))
flood.sat.sat = subset (flood.combined, Type %in% c("SatisfiedInterests"))
flood.sat.out = subset (flood.combined, Type %in% c("TimedOutInterests"))

bestr.data.in = subset (bestr.combined, Type %in% c("InData"))
bestr.data.out = subset (bestr.combined, Type %in% c("OutData"))
bestr.int.in = subset (bestr.combined, Type %in% c("InInterests"))
bestr.int.out = subset (bestr.combined, Type %in% c("OutInterests"))
bestr.sat.sat = subset (bestr.combined, Type %in% c("SatisfiedInterests"))
bestr.sat.out = subset (bestr.combined, Type %in% c("TimedOutInterests"))

smart.data.in = subset (smart.combined, Type %in% c("InData"))
smart.data.out = subset (smart.combined, Type %in% c("OutData"))
smart.int.in = subset (smart.combined, Type %in% c("InInterests"))
smart.int.out = subset (smart.combined, Type %in% c("OutInterests"))
smart.sat.sat = subset (smart.combined, Type %in% c("SatisfiedInterests"))
smart.sat.out = subset (smart.combined, Type %in% c("TimedOutInterests"))

perfect.data.in = subset (perfect.combined, Type %in% c("InData"))
perfect.data.out = subset (perfect.combined, Type %in% c("OutData"))
perfect.int.in = subset (perfect.combined, Type %in% c("InInterests"))
perfect.int.out = subset (perfect.combined, Type %in% c("OutInterests"))
perfect.sat.sat = subset (perfect.combined, Type %in% c("SatisfiedInterests"))
perfect.sat.out = subset (perfect.combined, Type %in% c("TimedOutInterests"))

library(plyr)

# inlist = list(flood.data.in, bestr.data.in, smart.data.in, perfect.data.in)
# outlist = list(flood.data.out, bestr.data.out, smart.data.out, perfect.data.out)
# intinlist = list(flood.int.in, bestr.int.in, smart.int.in, perfect.int.in)
# intoutlist = list(flood.int.out, bestr.int.out, smart.int.out, perfect.int.out)
# satsatlist = list(flood.sat.sat, bestr.sat.sat, smart.sat.sat, perfect.sat.sat)
# satoutlist = list(flood.sat.out, bestr.sat.out, smart.sat.out, perfect.sat.out)
# 
# allindata = do.call(rbind.fill, inlist)
# alloutdata = do.call(rbind.fill, outlist)
# allintindata = do.call(rbind.fill, intinlist)
# allintoutdata = do.call(rbind.fill, intoutlist)
# allsatdata = do.call(rbind.fill, satsatlist)
# satoutdata = do.call(rbind.fill, satoutlist)
# 
# g.datain2 = ggplot (data=allindata, aes (x=Time, y=PacketRaw.sum, linetype=Variable, colour=Variable, shape=Variable)) +
#   geom_line () +
#   xlab("Time") +
#   ylab("Packets") +
#   ggtitle("Data Packets received") +
#   scale_linetype_discrete(name = "Strategy Types",
#                           labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal")) +
#   scale_colour_discrete(name  ="Strategy Types",
#                         labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal")) +
#   scale_shape_discrete(name  = "Strategy Types",
#                        labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal"))
# 
# g.dataout2 = ggplot (data=alloutdata, aes (x=Time, y=PacketRaw.sum, linetype=Variable, colour=Variable, shape=Variable)) +
#   geom_line () +
#   xlab("Time") +
#   ylab("Packets") +
#   ggtitle("Data Packets transmitted") +
#   scale_linetype_discrete(name = "Strategy Types",
#                           labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal")) +
#   scale_colour_discrete(name  ="Strategy Types",
#                         labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal")) +
#   scale_shape_discrete(name  = "Strategy Types",
#                        labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal"))
# 
# g.dataintin = ggplot (data=allintindata, aes (x=Time, y=PacketRaw.sum, linetype=Variable, colour=Variable, shape=Variable)) +
#   geom_line () +
#   xlab("Time") +
#   ylab("Packets") +
#   ggtitle("Interest Packets Received") +
#   scale_linetype_discrete(name = "Strategy Types",
#                           labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal")) +
#   scale_colour_discrete(name  ="Strategy Types",
#                         labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal")) +
#   scale_shape_discrete(name  = "Strategy Types",
#                        labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal"))
# 
# g.dataintout = ggplot (data=allintoutdata, aes (x=Time, y=PacketRaw.sum, linetype=Variable, colour=Variable, shape=Variable)) +
#   geom_line () +
#   xlab("Time") +
#   ylab("Packets") +
#   ggtitle("Interest Packets Transmitted") +
#   scale_linetype_discrete(name = "Strategy Types",
#                           labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal")) +
#   scale_colour_discrete(name  ="Strategy Types",
#                         labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal")) +
#   scale_shape_discrete(name  = "Strategy Types",
#                        labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal"))
# 
# g.intsat2 = ggplot (data=allsatdata, aes (x=Time, y=PacketRaw.sum, linetype=Variable, colour=Variable, shape=Variable)) +
#   geom_line () +
#   xlab("Time") +
#   ylab("Packets Satisfied") +
#   ggtitle("Interest Packets Satisfied") +
#   scale_linetype_discrete(name = "Strategy Types",
#                           labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal")) +
#   scale_colour_discrete(name = "Strategy Types",
#                           labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal")) +
#   scale_shape_discrete(name = "Strategy Types",
#                           labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal"))
# 
# g.intout2 = ggplot (data=satoutdata, aes (x=Time, y=PacketRaw.sum, linetype=Variable, colour=Variable, shape=Variable)) +
#   geom_line () +
#   xlab("Time") +
#   ylab("Timed out Packets") +
#   ggtitle("Timed Out Interest Packets") +
#   scale_linetype_discrete(name = "Strategy Types",
#                           labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal")) +
#   scale_colour_discrete(name = "Strategy Types",
#                         labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal")) +
#   scale_shape_discrete(name = "Strategy Types",
#                        labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal"))
#   
# 
# print (g.datain2)
# print (g.dataout2)
# print (g.dataintin)
# print (g.dataintout)
# print (g.intsat2)
# print (g.intout2)

combinedlist = list(flood.combined, bestr.combined, smart.combined, perfect.combined)
allcombineddata = do.call(rbind.fill, combinedlist)

satTime = subset(allcombineddata, Type %in% c("SatisfiedInterests", "TimedOutInterests"))


g.test = ggplot (satTime, aes(linetype=Type, colour=Variable)) +
  geom_line(aes (x=Time, y=PacketRaw.sum), size=1.5) +
  xlab("Time") +
  ylab("Packets") +
  ggtitle("Satisfied and Timed out packets") +
  scale_colour_discrete(name = "Strategy Types",
                        labels = c("BestRoute", "Flooding", "SmartFlooding", "Ideal"))

#print (g.test)

summaryBy (Packets ~ Type, data=rawdata.flood, FUN=sum)
summaryBy (Packets ~ Type, data=rawdata.bestr, FUN=sum)
summaryBy (Packets ~ Type, data=rawdata.smart, FUN=sum)
summaryBy (Packets ~ Type, data=rawdata.perfect, FUN=sum)

png ("/home/user1/git/ccn-sim-1/documents/papers/satisfied-timeout.png", width=1024, height=768)
print (g.test)
x = dev.off ()

# # Total packets - Data
# g.data <- ggplot (flood.data.combined, aes(x=Time, y=PacketRaw.sum, color=Type)) +
#   geom_line(aes (linetype=Type), size=0.5) + 
#   geom_point(aes (shape=Type), size=1) +
#   geom_line(data=bestr.data.combined, aes(x=Time, y=PacketRaw.sum, color=Type)) +
#   geom_point(data=bestr.data.combined, aes (shape=Type), size=1) +
#   ggtitle (dataname) +
#   ylab ("Packets/s")

# # Total Packets - Interests
# g.int <- ggplot (intdata.combined, aes(x=Time, y=PacketRaw.sum, color=Type)) +
#   geom_line(aes (linetype=Type), size=0.5) + 
#   geom_point(aes (shape=Type), size=1) +  
#   ggtitle (intname) +
#   ylab ("Packets/s")
# 
# # Total Packets - Satisfied
# g.sat <- ggplot (satdata.combined, aes(x=Time, y=Packets.sum, color=Type)) +
#   geom_line(aes (linetype=Type), size=0.5) + 
#   geom_point(aes (shape=Type), size=1) +  
#   ggtitle (satname) +
#   ylab ("Packets/s")

# # Get the basename of the file
# tmpname = strsplit(".", "/")[[1]]
# filename = tmpname[length(tmpname)]
# # Get rid of the extension
# noext = gsub("\\..*", "", filename)
# 
# outDatapng = sprintf("%s/%s-data.png", opt$output, noext)
# outInpng = sprintf("%s/%s-in.png", opt$output, noext)
# outSatpng = sprintf("%s/%s-sat.png", opt$output, noext)
# 
# png (outDatapng, width=1024, height=768)
# print (g.data)
# x = dev.off ()

# png (outInpng, width=1024, height=768)
# print (g.int)
# x = dev.off ()
# 
# png (outSatpng, width=1024, height=768)
# print (g.sat)
# x = dev.off ()