library(tools)
library(ggplot2)
library(tidyr)
library(dplyr)
library(colorspace)
library(configr)

args = vector()

directory <- "C:/Users/Martin Schonger/source/repos/crep_eval/"
log_directory <- paste(directory, "log/", sep = "")


#timestamps <- c("20190715-200551","20190715-201732","20190717-145500","20190717-145659","20190717-160048","20190717-155945","20190717-175026","20190717-181441")
#my_labels <- c("opt","nopt","opt + bucket","nopt + bucket","opt + fibonacci","nopt + fibonacci","opt_v2","opt_v3")
#timestamps <- c(timestamps, "20190718-095402","20190718-094548","20190718-092205","20190718-091339")
#my_labels <- c(my_labels, "opt2","nopt2","nopt3","tmp")

#timestamps <- c("20190718-125526","20190718-133243","20190718-124840","20190718-123556")
#my_labels <- c("none","only charikar optimized","basic","full")

#timestamps <- c("20190718-151327","20190718-152158","20190718-153741","20190718-154938")
#my_labels <- c("basic","basic","full","full")

#timestamps <- c("20190718-124840","20190720-130621")
#my_labels <- c("a","b")

timestamp <- "20190723-141139"
file <- paste(log_directory, timestamp, "__main_log.csv", sep = "")
main_log <- read.csv(file)# %>% 
#  select(c(alpha,cluster_size,num_clusters,com_cost_dyn,com_cost_stat,num_deletes_dyn,num_merges_dyn,total_time_dyn,total_time_stat))

timestamps <- levels(main_log$timestamp)
my_labels <- lapply(main_log$alpha, as.character)

idx <- 1
for (ts in timestamps) {
  args[idx] <- paste(log_directory, ts, "__time_measurement.csv", sep = "")
  idx <- idx + 1
}

rawdata <- list()
for (filename in args) {
  tmp <- read.csv(filename)
  rawdata <- append(rawdata, list(tmp))
}

#TODO
#config <- read.config(file = paste(log_directory, "20190717-181441", "__config.txt", sep = ""))

n <- nrow(rawdata[[1]])
timedata <- data.frame("tstamp" = 1:n, stringsAsFactors = FALSE)
for (i in seq_along(rawdata)) {
  timedata[paste("x",i,sep = "")] <- rawdata[[i]][1:n,2] #TODO
}

#timedata <- data.frame("tstamp" = 1:n, "timeA" = rawdataA[,2], "timeB" = rawdataB[,2], stringsAsFactors = FALSE)
#str(timedata)

#ggplot(data = timedata, aes(x = tstamp, y = time)) +
#  geom_line(color = "#00AFBB", size = 2)

select_col_names <- colnames(timedata)

proc_data <- timedata %>%
  select(select_col_names) %>%
  gather(key = "variable", value = "value", -tstamp)

min_x <- 0
max_x <- n
min_y <- 0
max_y <- 7.5
min_y2 <- min_y
max_graph_size <- max(rawdata[[1]][,1], na.rm = TRUE)
max_y2 <- ceiling(max_graph_size / 500) * 500

gtime <- 1:n
gvalue <- rawdata[[1]][,1]
proc_data_graphsize <- data.frame("tstamp" = gtime, "gsize" = gvalue, stringsAsFactors = FALSE)

max_gsize <- max(proc_data_graphsize[,2], na.rm = TRUE)

scale_factor <- max_y / max_y2

x_breaks <- seq(from = 0, to = n, by = 20)

first_breaks <- seq(from = 0, to = max_y, by = 0.5)
sec_breaks <- seq(from = 0, to = max_y2, by = 350)

#sec_scaled_breaks <- sec_breaks * scale_factor

p <- ggplot(data = proc_data, aes(x = tstamp)) +
  geom_line(mapping = aes(y = value, color = variable)) +
  scale_x_continuous(breaks = x_breaks) +
  scale_y_continuous(name = "time[s]", breaks = first_breaks, sec.axis = sec_axis(trans = ~. / scale_factor, name = "graph size [#]", breaks = sec_breaks)) +
  #scale_colour_manual(name = "config", labels = c("opt","no opt","no opt + bucket q."), values = c("#00AFBB", "#E7B800", "red")) +
  #scale_color_viridis_d(name = "config", labels = c("opt","no opt","no opt + bucket q."), option = "D", end = 0.98) +
  scale_color_manual(name = "config", labels = my_labels, values = qualitative_hcl(length(rawdata), "Dynamic")) +
  #xlim(min_x, max_x) +
  #ylim(min_y, max_y) +
  xlab("batch [#]") +
  ylab("time [s]") +
  ggtitle(paste("Required time per batch [", args[1], "]", sep = "")) +
  coord_cartesian(xlim = c(min_x, max_x), ylim = c(min_y, max_y)) +
  theme_light() +
  theme(plot.title = element_text(hjust = 0.5))


p <- p +
  geom_line(data = proc_data_graphsize, aes(x = tstamp, y = (gsize / max_y2) * max_y), linetype = 2)


#TODO maybe add scale for event_complexity
#p <- p +
#  geom_line(data = events_for_plot, aes(x = tstamp, y = (sum.num_nodes_base / max(events_for_plot[,2]) * 4)))


#p <- p +
#  geom_vline(data = proc_data_graphsize, aes(xintercept = tstamp)) +
#  geom_text(data = proc_data_graphsize, mapping = aes(label = gsize, y = 0), angle = 60, hjust = 0)


cur_timestamp <- format(Sys.time(), "%Y%m%d-%H%M%S")

ggsave(paste(directory, "plot/", cur_timestamp, "__time_plot.pdf", sep = ""), plot = p, width = 35, height = 20, units = "cm", dpi = "retina")
