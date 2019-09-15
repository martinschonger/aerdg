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

feature <- "part_size"

summary <- read.csv(paste(data_directory, feature, ".csv", sep = ""))

preproc <- summary %>%
  select("expl_strat","db","p_size","time_total","com_cost","mig_cost")

preproc$p_size <- as.factor(preproc$p_size)
preproc$p_size <- fct_inorder(preproc$p_size, ordered = NA)

for (data_trace in c("p_fab", "hpc")) {
  
  plot_data <- preproc %>%
    select(-c("com_cost","mig_cost")) %>%
    filter(expl_strat != "", db == data_trace)
  
  if (data_trace == "p_fab") {
    my_ylim <- c(0, 80)
  }
  if (data_trace == "hpc") {
    my_ylim <- c(0, 250)
  }
  
  p <- ggplot(data=plot_data, aes(x=p_size, y=time_total, fill=expl_strat)) +
    geom_boxplot() + 
    xlab("Partition size") + 
    ylab("Time [s]") + 
    coord_cartesian(ylim=my_ylim) + 
    theme_bw() + 
    theme(legend.title=element_blank()) +
    scale_fill_brewer(palette="Set1", labels=c("CC","HOP")) + 
    theme(text=element_text(size=12))
  
  if (data_trace == "hpc") {
    p <- p + 
      theme(legend.justification = c(1.1, 1.1), legend.position = c(1, 1))
  } else {
    p <- p +
      theme(legend.position = "none")
  }
  
  outfile <- paste(out_directory, feature, "_runtime_", data_trace, ".pdf", sep="")
  ggsave(filename=outfile, plot=p, width = plotwidth_half, height = plotheight_half)
  
}
