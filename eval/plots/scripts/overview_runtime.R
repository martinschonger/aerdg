library(tools)
library(ggplot2)
library(tidyr)
library(dplyr)
library(colorspace)
library(gridExtra)
library(tidyverse)

source("C:/Users/Martin Schonger/source/repos/CREP/doc/crep_functions.R")

directory <- "C:/Users/Martin Schonger/source/repos/CREP/eval/plots/"
data_directory <- paste(directory, "data/", sep = "")
out_directory <- "C:/Users/Martin Schonger/OneDrive/TUM/sem6 - 19S/bthesis_shared/thesis/source/figures/"

summary_filename <- "overview.csv"
summary <- read.csv(paste(data_directory, summary_filename, sep = ""))

preproc <- summary %>%
  select("expl_strat","db","gamma","time_total","com_cost","mig_cost")

for (gamm in c(1.0, 0.7)) {
  for (data_trace in c("p_fab", "hpc")) {
    
    age <- ""
    if (gamm == 1.0) {
      age <- "no_age"
    }
    if (gamm == 0.7) {
      age <- "age"
    }
    
    plot_data <- preproc %>%
      select(-c("com_cost","mig_cost")) %>%
      filter(gamma == gamm, db == data_trace)
    
    if (data_trace == "p_fab") {
      my_ylim <- c(0, 50)
    }
    if (data_trace == "hpc") {
      my_ylim <- c(0, 800)
    }
    
    p <- ggplot(data=plot_data, aes(x=expl_strat, y=time_total)) +
      geom_boxplot() + 
      ylab("Time [s]") + 
      scale_x_discrete(labels=c("NAIVE", "CC", "GREEDY", "HOP")) + 
      coord_cartesian(ylim=my_ylim) + 
      theme_bw() + 
      theme(axis.title.x=element_blank(), legend.title=element_blank()) +
      scale_fill_brewer(palette="Dark2") + 
      theme(text=element_text(size=12))
    p
    
    outfile <- paste(out_directory, "overview_runtime_", age, "_", data_trace, ".pdf", sep="")
    ggsave(filename=outfile, plot=p, width = plotwidth_half, height = plotheight_half)
    
  }
}
