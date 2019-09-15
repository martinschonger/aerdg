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
      select(-c("time_total")) %>%
      filter((gamma == gamm | expl_strat == ""), db == data_trace) %>%
      gather(key="cost_type",value="cost",c("com_cost","mig_cost"))
    
    if (data_trace == "p_fab") {
      my_ylim <- c(0, 120000)
      my_ybreaks <- c(0,40000,80000,120000)
      my_ylabels <- c("0","40k","80k","120k")
    }
    if (data_trace == "hpc") {
      my_ylim <- c(0, 100000)
      my_ybreaks <- c(0,25000,50000,75000,100000)
      my_ylabels <- c("0","25k","50k","75k","100k")
    }
    
    p <- ggplot(data=plot_data, aes(x=expl_strat, y=cost, fill=cost_type)) +
      geom_boxplot() + 
      ylab("Cost") + 
      scale_x_discrete(labels=c("STAT", "NAIVE", "CC", "GREEDY", "HOP")) + 
      coord_cartesian(ylim=my_ylim) + 
      scale_y_continuous(breaks=my_ybreaks, labels=my_ylabels) + 
      theme_bw() + 
      theme(axis.title.x=element_blank(), legend.title=element_blank()) +
      #scale_fill_grey(start=0.95, end=0.4) + 
      scale_fill_brewer(palette="Dark2", labels=c("com","mig")) + 
      theme(text=element_text(size=12))
    
    if (gamm == 1.0) {
      p <- p +
        theme(legend.position = "none")
    }
    if (gamm == 0.7) {
      p <- p + 
        theme(legend.justification = c(1.1, 1.1), legend.position = c(1, 1))
    }
    
    outfile <- paste(out_directory, "overview_", age, "_", data_trace, ".pdf", sep="")
    ggsave(filename=outfile, plot=p, width = plotwidth_half, height = plotheight_half)
    
  }
}
