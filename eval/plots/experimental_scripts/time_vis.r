library(tools)
library(ggplot2)

args = commandArgs(trailingOnly=TRUE)

timestamp <- "20190719-172958"
args[1] <- paste("C:/Users/Martin Schonger/source/repos/crep_eval/log/", timestamp, "__time_measurement.csv", sep = "")

if (length(args)==0) {
  stop("At least one argument must be supplied: 'path_to_file'", call.=FALSE)
} else if (length(args)==1) {
  #default output file
  args[2] <- paste(dirname(args[1]), "/", tools::file_path_sans_ext(basename(args[1])), "__plot.pdf", sep = "")
}

tmp <- read.csv(args[1])

scale_factor <- (max(tmp[1], na.rm = TRUE) / max(tmp[2], na.rm = TRUE))

plotty <- ggplot(tmp, aes(x = 1:dim(tmp)[1])) +
  labs(x = "batch", y = "num_nodes") +
  geom_point(aes(y = time * scale_factor, color = "avg_time_per_batch [s]")) +
  geom_point(aes(y = graph_size, color = "graph_size")) +
  scale_y_continuous(sec.axis = sec_axis(~./scale_factor))

#save plots to pdf
pdf(args[2])

print(plotty)

dev.off()
