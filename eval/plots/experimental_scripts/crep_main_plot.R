library(tools)
library(ggplot2)
library(tidyr)
library(dplyr)
library(colorspace)
library(gridExtra)
library(tidyverse)

source("C:/Users/Martin Schonger/source/repos/CREP/doc/crep_functions.R")

directory <- "C:/Users/Martin Schonger/source/repos/crep_eval/"
log_directory <- paste(directory, "log/", sep = "")

main_timestamp <- "20190823-125627"
main_timestamp <- "20200823-122533"
main_timestamp <- "20190825-204955"
main_timestamp <- "20190825-205045"
main_timestamp <- "20190825-205424"
main_timestamp <- "20190826-121457"
main_timestamp <- "20190826-131940"
main_timestamp <- "20190826-125508"
main_timestamp <- "20190826-125744"
main_timestamp <- "20190826-132013"
main_timestamp <- "20190826-125935"
main_timestamp <- "20190830-165518"
main_timestamp <- "20190830-182730"
specific_log_dir <- paste(log_directory, main_timestamp, "/", sep = "")

summary_filename <- paste(main_timestamp, "__summary.csv", sep = "")
summary <- read.csv(paste(specific_log_dir, summary_filename, sep = ""))
summary$expl_strat_specifics <- as.character(summary$expl_strat_specifics)
summary$part_strat_specifics <- as.character(summary$part_strat_specifics)

X <- summary

feature <- "com_cost"
myplots <- list()
alphas <- c(20, 50, 100)
alphas <- c(10, 50, 100)
alphas <- c(4, 6, 20, 50, 100)

for (i in seq_along(alphas)) {
  Y <- X %>%
    filter(alpha == alphas[i], !base_alg %in% c("static","adaptive")) %>%
    select(names(X))
  
  rawdata <- crep_read_rawdata(feature, Y, specific_log_dir)
  
  filled <- crep_fill_missing(rawdata, seq(from = 1, to = 2000000))
  
  combined <- crep_combine(filled, col = 2, accum = TRUE)
  
  reshaped <- crep_reshape(combined)
  
  
  tmp <- summary
  tmp$uid <- as.character(tmp$uid)
  tmp <- tmp[tmp$uid %in% names(table(reshaped$key)), ]
  
  caption <- ""
  
  for (j in seq(nrow(tmp))) {
    keys <- names(tmp)
    vals <- as.character(tmp[j,])
    a <- cbind(keys, vals)
    a <- paste(a[,1], a[,2], sep = '=')
    a <- paste(a, collapse = ', ')
    
    if (nchar(caption) == 0) {
      caption <- a
    } else {
      caption <- paste(caption, a, sep = "; ")
    }
  }
  
  
  p <- crep_basic_plot(reshaped, 2000000, "request [1e4]", "cost", "uid", feature, paste("alpha = ", alphas[i], sep=""), caption, no_legend = (i != 1))
  
  
  Z <- X %>% filter(alpha == alphas[i], base_alg == "static")
  for (j in seq(nrow(Z))) {
    p <- p +
      geom_hline(yintercept = Z$com_cost[j])
  }
  
  
  myplots[[i]] <- p
}


cur_timestamp <- format(Sys.time(), "%Y%m%d-%H%M%S")
#pdf(paste(directory, "plot/", cur_timestamp, "__alphas.pdf", sep=""), width = 7*length(alphas), height = 7)
png(paste(directory, "plot/", cur_timestamp, "__alphas.jpeg", sep=""), 
    width = 10*length(alphas), 
    height = 10 + 3*2, units = 'cm', 
    res = 300)
grid.arrange(grobs = myplots, nrow = 1)
dev.off()
