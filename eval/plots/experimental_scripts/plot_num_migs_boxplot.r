library(tools)
library(ggplot2)
library(tidyr)
library(dplyr)
library(colorspace)

directory <- "C:/Users/Martin Schonger/source/repos/crep_eval/"
log_directory <- paste(directory, "log/", sep = "")

main_timestamp <- "20190820-114139"
main_timestamp <- "20190820-213851"
specific_log_dir <- paste(log_directory, main_timestamp, "/", sep = "")

filenames <- list.files(specific_log_dir, pattern = "*num_migs.csv")

summary_filename <- paste(specific_log_dir, main_timestamp, "__summary.csv", sep = "")
summary <- read.csv(summary_filename)
summary$expl_strat_specifics <- as.character(summary$expl_strat_specifics)
summary$part_strat_specifics <- as.character(summary$part_strat_specifics)

get_labels_from_summary <- function(summary_df) {
  res <- as.character(summary_df$uid)
  res
}

my_labels <- get_labels_from_summary(summary)

#my_labels <- filenames

args = vector()
idx <- 1
for (ts in filenames) {
  args[idx] <- paste(specific_log_dir, ts, sep = "")
  idx <- idx + 1
}

rawdata <- list()
for (filename in args) {
  header = read.csv(filename, skip = 0, header = F, nrows = 1, as.is = T)
  tmp = read.csv(filename, skip = 2, header = F)
  colnames(tmp)= header
  rawdata <- append(rawdata, list(tmp))
}

n <- nrow(rawdata[[1]])
timedata <- data.frame("tstamp" = 1:n, stringsAsFactors = FALSE)
for (i in seq_along(rawdata)) {
  timedata[my_labels[i]] <- cumsum(rawdata[[i]][1:n,2]) #TODO
}

select_col_names <- colnames(timedata)

proc_data <- timedata %>%
  select(select_col_names) %>%
  gather(key = "variable", value = "value", -tstamp)

title <- paste("Migrations per vertex [", main_timestamp, "]", sep = "")

p <- ggplot(data = proc_data, aes(x = variable, y = value)) + 
  geom_boxplot() + 
  ggtitle(title) +
  xlab("uid") +
  ylab("migrations [#]") +
  coord_flip()

cur_timestamp <- format(Sys.time(), "%Y%m%d-%H%M%S")
ggsave(paste(directory, "plot/", main_timestamp, "__num_migs___", cur_timestamp, ".pdf", sep = ""), plot = p, dpi = "retina")
