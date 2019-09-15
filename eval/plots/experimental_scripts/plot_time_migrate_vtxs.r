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

filenames <- list.files(specific_log_dir, pattern = "*time_migrate.csv")

summary_filename <- paste(main_timestamp, "__summary.csv", sep = "")
summary <- read.csv(paste(specific_log_dir, summary_filename, sep = ""))
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

num_requests <- 1000000

rawdata <- list()
for (filename in args) {
  header = read.csv(filename, skip = 0, header = F, nrows = 1, as.is = T)
  tmp = read.csv(filename, skip = 2, header = F)
  colnames(tmp)= header
  
  tmp_filled <- data.frame(time = seq(num_requests), duration = numeric(num_requests))
  tmp_filled$duration[tmp$request] <- tmp$time
  
  rawdata <- append(rawdata, list(tmp_filled))
}

n <- num_requests#nrow(rawdata[[1]])
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
max_y <- 1#good value for n=10000: 1e-03

x_breaks <- seq(from = min_x, to = max_x, by = n/10)

y_breaks <- seq(from = min_y, to = max_y, by = max_y/10)

title <- paste("Cumulative migration time [", main_timestamp, "]", sep = "")

points_for_readability <- proc_data %>% slice(seq(n/20, n(), by = n/10))

p <- ggplot(data = proc_data, aes(x = tstamp)) +
  geom_line(mapping = aes(y = value, color = variable)) +
  geom_point(data = points_for_readability, mapping = aes(x = tstamp, y = value, color = variable , shape = variable)) +
  scale_x_continuous(breaks = x_breaks) +
  scale_y_continuous(name = "time [s]", breaks = y_breaks) +
  scale_color_manual(name = "uid", labels = my_labels, values = qualitative_hcl(length(rawdata), "Dynamic")) +
  xlab("request [#]") +
  #ylab("time [s]") +
  ggtitle(title) +
  coord_cartesian(xlim = c(min_x, max_x), ylim = c(min_y, max_y)) +
  theme_light() +
  theme(plot.title = element_text(hjust = 0.5)) +
  scale_shape_manual(values=seq(0,length(my_labels)))

cur_timestamp <- format(Sys.time(), "%Y%m%d-%H%M%S")
ggsave(paste(directory, "plot/", main_timestamp, "__time_migrate___", cur_timestamp, ".pdf", sep = ""), plot = p, dpi = "retina")
