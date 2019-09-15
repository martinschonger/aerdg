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

feature <- "hops"

summary <- read.csv(paste(data_directory, feature, ".csv", sep = ""))

preproc <- summary %>%
  select("expl_strat","add_param","db","time_total","com_cost","mig_cost")

preproc$add_param <- as.factor(preproc$add_param)

for (data_trace in c("p_fab", "hpc")) {
  
  plot_data <- preproc %>%
    select(-c("com_cost","mig_cost")) %>%
    filter(db == data_trace, expl_strat != "")
  
  if (data_trace == "p_fab") {
    my_ylim <- c(0, 20)
  }
  if (data_trace == "hpc") {
    my_ylim <- c(0, 300)
  }
  
  p <- ggplot(data=plot_data, aes(x=add_param, y=time_total)) +
    geom_boxplot() + 
    xlab(feature) + 
    ylab("Time [s]") + 
    coord_cartesian(ylim=my_ylim) + 
    theme_bw() + 
    theme(legend.title=element_blank()) +
    theme(text=element_text(size=12))

  outfile <- paste(out_directory, feature, "_runtime_", data_trace, ".pdf", sep="")
  ggsave(filename=outfile, plot=p, width = plotwidth_half, height = plotheight_half)
  
}
