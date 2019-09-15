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

feature <- "alpha"

summary <- read.csv(paste(data_directory, feature, ".csv", sep = ""))

preproc <- summary %>%
  select("expl_strat","db","alpha","time_total","com_cost","mig_cost")

preproc$alpha <- as.factor(preproc$alpha)

for (my_expl_strat in c("comp_mig_cc_age", "comp_mig_hop_age")) {
  for (data_trace in c("p_fab", "hpc")) {
    
    plot_data <- preproc %>%
      select(-c("time_total")) %>%
      filter(expl_strat == my_expl_strat, db == data_trace) %>%
      gather(key="cost_type",value="cost",c("com_cost","mig_cost"))
    
    if (data_trace == "p_fab") {
      my_ylim <- c(0, 80000)
      my_ybreaks <- c(0,20000,40000,60000,80000)
      my_ylabels <- c("0","20k","40k","60k","80k")
    }
    if (data_trace == "hpc") {
      my_ylim <- c(0, 100000)
      my_ybreaks <- c(0,25000,50000,75000,100000)
      my_ylabels <- c("0","25k","50k","75k","100k")
    }
    
    p <- ggplot(data=plot_data, aes(x=alpha, y=cost, fill=cost_type)) +
      geom_boxplot() + 
      xlab(feature) + 
      ylab("Cost") + 
      coord_cartesian(ylim=my_ylim) + 
      scale_y_continuous(breaks=my_ybreaks, labels=my_ylabels) + 
      theme_bw() + 
      theme(legend.title=element_blank()) +
      scale_fill_brewer(palette="Dark2", labels=c("com","mig")) + 
      theme(text=element_text(size=12))
    
    if (my_expl_strat == "comp_mig_hop_age" && data_trace == "p_fab") {
      p <- p + 
        theme(legend.justification = c(1.1, 1.1), legend.position = c(1, 1))
    } else if (my_expl_strat == "comp_mig_cc_age" && data_trace == "hpc") {
      p <- p + 
        theme(legend.justification = c(-0.1, 1.1), legend.position = c(0, 1))
    } else {
      p <- p +
        theme(legend.position = "none")
    }
    
    outfile <- paste(out_directory, feature, "_", my_expl_strat, "_", data_trace, ".pdf", sep="")
    ggsave(filename=outfile, plot=p, width = plotwidth_half, height = plotheight_half)
    
  }
}
