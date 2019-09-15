library(tools)
library(ggplot2)
library(tidyr)
library(dplyr)
library(colorspace)
library(gridExtra)
library(tidyverse)

source("C:/Users/Martin Schonger/source/repos/CREP/eval/plots/scripts/config.R")

directory <- "C:/Users/Martin Schonger/source/repos/CREP/eval/plots/"
data_directory <- paste(directory, "data/", sep = "")
out_directory <- "C:/Users/Martin Schonger/OneDrive/TUM/sem6 - 19S/bthesis_shared/thesis/source/figures/"

summary_filename <- "facebook.csv"
summary <- read.csv(paste(data_directory, summary_filename, sep = ""))

preproc <- summary %>%
  select("expl_strat","db","gamma","time_total","com_cost","mig_cost")

plot_data <- preproc %>%
  select(-c("time_total")) %>%
  gather(key="cost_type",value="cost",c("com_cost","mig_cost"))

my_ylim <- c(0, 50000)
my_ybreaks <- c(0,10000,20000,30000,40000,50000)
my_ylabels <- c("0","10k","20k","30k","40k","50k")

p <- ggplot(data=plot_data, aes(x=expl_strat, y=cost, fill=cost_type)) +
  geom_boxplot() + 
  ylab("Cost") + 
  scale_x_discrete(labels=c("STAT", "CC", "HOP")) + 
  coord_cartesian(ylim=my_ylim) + 
  scale_y_continuous(breaks=my_ybreaks, labels=my_ylabels) + 
  theme_bw() + 
  theme(axis.title.x=element_blank(), legend.title=element_blank()) +
  #scale_fill_grey(start=0.95, end=0.4) + 
  scale_fill_brewer(palette="Dark2", labels=c("com","mig")) + 
  theme(text=element_text(size=12))

  p <- p + 
    theme(legend.justification = c(-0.1, 1.1), legend.position = c(0, 1))

outfile <- paste(out_directory, "facebook.pdf", sep="")
ggsave(filename=outfile, plot=p, width = plotwidth_half, height = plotheight_half)
