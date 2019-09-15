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
  select(-c("com_cost","mig_cost")) %>%
  filter(expl_strat != "")

my_ylim <- c(0, 400)

p <- ggplot(data=plot_data, aes(x=expl_strat, y=time_total)) +
  geom_boxplot() + 
  ylab("Time [s]") + 
  scale_x_discrete(labels=c("CC", "HOP")) + 
  coord_cartesian(ylim=my_ylim) + 
  theme_bw() + 
  theme(axis.title.x=element_blank(), legend.title=element_blank()) +
  #scale_fill_grey(start=0.95, end=0.4) + 
  scale_fill_brewer(palette="Dark2") + 
  theme(text=element_text(size=12))

p <- p + 
  theme(legend.justification = c(1.1, 1.1), legend.position = c(1, 1))

outfile <- paste(out_directory, "facebook_runtime.pdf", sep="")
ggsave(filename=outfile, plot=p, width = plotwidth_half, height = plotheight_half)
