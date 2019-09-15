library(dplyr)
library(ggplot2)


directory <- "C:/Users/Martin Schonger/source/repos/crep_eval/log/"

timestamp <- "20190720-130621"
events <- read.csv(paste(directory, timestamp, "__events.csv", sep = "")) %>% 
  select(c(time, num_nodes_base))

breaks <- seq(0,100000,1000)

events_proc <- events %>% 
  mutate(interval = cut(time,
                        breaks, 
                        include.lowest = TRUE, 
                        right = TRUE)) %>%
  group_by(interval) %>% 
  summarise(sum.num_nodes_base = sum(num_nodes_base))

events_proc[90:100,]

breaks_plot <- seq(0,100000,2500)
ploty <- ggplot(data = events_proc, aes(x = interval, y = sum.num_nodes_base)) + 
  geom_line(group = 1) + 
  theme_minimal()
ploty
ggsave(filename = paste(directory, timestamp, "__events_plot.pdf", sep = ""), plot = ploty)

events_for_plot <- data.frame("tstamp" = 1:100, "num_nodes_base" = events_proc[,2])
