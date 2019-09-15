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

feature <- "gamma"

summary <- read.csv(paste(data_directory, feature, ".csv", sep = ""))

preproc <- summary %>%
  select("expl_strat","db","gamma","time_total","com_cost","mig_cost")

preproc$gamma <- as.factor(preproc$gamma)
preproc$gamma <- fct_inorder(preproc$gamma, ordered = NA)

for (data_trace in c("p_fab", "hpc")) {
  
  plot_data <- preproc %>%
    select(-c("com_cost","mig_cost")) %>%
    filter(db == data_trace, expl_strat != "")
  
  if (data_trace == "p_fab") {
    my_ylim <- c(0, 100)
  }
  if (data_trace == "hpc") {
    my_ylim <- c(0, 250)
  }
  
  p <- ggplot(data=plot_data, aes(x=gamma, y=time_total, fill=expl_strat)) +
    geom_boxplot() + 
    xlab(feature) + 
    ylab("Time [s]") + 
    coord_cartesian(ylim=my_ylim) + 
    theme_bw() + 
    theme(legend.title=element_blank()) +
    scale_fill_brewer(palette="Set1", labels=c("CC","HOP")) + 
    theme(text=element_text(size=12))
  
  if (my_expl_strat == "comp_mig_hop_age" && data_trace == "p_fab") {
    p <- p + 
      theme(legend.justification = c(-0.1, 1.1), legend.position = c(0, 1))
  } else {
    p <- p +
      theme(legend.position = "none")
  }
  
  outfile <- paste(out_directory, feature, "_runtime_", data_trace, ".pdf", sep="")
  ggsave(filename=outfile, plot=p, width = plotwidth_half, height = plotheight_half)
  
}
