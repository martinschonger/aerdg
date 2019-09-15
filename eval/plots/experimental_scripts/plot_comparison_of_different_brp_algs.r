library(tools)
library(ggplot2)
library(tidyr)
library(dplyr)
library(colorspace)

directory <- "C:/Users/Martin Schonger/source/repos/crep_eval/"
log_directory <- paste(directory, "log/", sep = "")

#20190812-000022
#20190820-000640
main_timestamp <- "20190820-114139"
specific_log_dir <- paste(log_directory, main_timestamp, "/", sep = "")

filenames <- list.files(specific_log_dir, pattern = "*time.csv")

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

min_x <- 0
max_x <- n
min_y <- 0
max_y <- 15000

x_breaks <- seq(from = min_x, to = max_x, by = 100000)

first_breaks <- seq(from = min_y, to = max_y, by = 500)

title <- paste("Required time per request [", summary_filename, "]", sep = "")

p <- ggplot(data = proc_data, aes(x = tstamp)) +
  geom_line(mapping = aes(y = value, color = variable)) +
  scale_x_continuous(breaks = x_breaks) +
  scale_y_continuous(name = "time[s]", breaks = first_breaks) +
  scale_color_manual(name = "config", labels = my_labels, values = qualitative_hcl(length(rawdata), "Dynamic")) +
  xlab("batch [#]") +
  ylab("time [s]") +
  ggtitle(title) +
  coord_cartesian(xlim = c(min_x, max_x), ylim = c(min_y, max_y)) +
  theme_light() +
  theme(plot.title = element_text(hjust = 0.5))

cur_timestamp <- format(Sys.time(), "%Y%m%d-%H%M%S")

ggsave(paste(directory, "plot/", cur_timestamp, "__time_plot.pdf", sep = ""), plot = p, width = 35, height = 20, units = "cm", dpi = "retina")
