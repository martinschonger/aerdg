require(tools)
require(tidyr)
require(dplyr)

crep_read_rawdata <- function(feature, X, dir)
{
  uids <- as.character(X$uid)
  
  filenames <- paste(uids, "__", feature, ".csv", sep="")
  full_filenames <- paste(dir, filenames, sep = "")
  
  rawdata <- list()
  for (fn in full_filenames) {
    header = read.csv(fn, skip = 0, header = F, nrows = 1, as.is = T)
    uid = read.csv(fn, skip = 1, header = F, nrows = 1, as.is = T)
    tmp = read.csv(fn, skip = 2, header = F)
    colnames(tmp)= header
    rawdata[[as.character(uid)]] <- tmp
  }
  
  rawdata
}

crep_fill_missing <- function(X, sequence)
{
  res <- X
  
  for (i in seq_along(res)) {
    tmp <- data.frame(sequence)
    colnames(tmp) <- names(res[[i]])[1]
    tmp <- merge(tmp, res[[i]], all.x = TRUE)
    tmp[is.na(tmp)] <- 0
    res[[i]] <- tmp
  }
  
  res
}

crep_combine <- function(X, col = 2, accum = FALSE)
{
  res <- X[[1]][1]
  
  for (i in seq_along(X)) {
    if (accum) {
      res[names(X)[i]] <- cumsum(X[[i]][col])
    } else {
      res[names(X)[i]] <- X[[i]][col]
    }
  }
  
  res
}

crep_reshape <- function(X)
{
  res <- X %>%
    gather(key = "key", value = "value", -1)
  
  res$key <- res$key %>% factor
  
  res$key <- fct_inorder(res$key, ordered = NA)
  
  res
}

crep_basic_plot <- function(X, max_y, label_x, label_y, legend_title, title, subtitle = "", caption = "", no_legend = FALSE)
{
  n <- as.numeric(table(X$key)[1])
  my_labels <- names(table(X$key))
  
  min_x <- 0
  max_x <- n
  min_y <- 0
  max_y <- max_y
  
  x_breaks <- seq(from = min_x, to = max_x, by = n/10)
  x_labels <- x_breaks/10000
  y_breaks <- seq(from = min_y, to = max_y, by = max_y/10)
  
  title <- title
  
  points_for_readability <- X %>% slice(seq(n/20, n(), by = n/10))
  
  p <- ggplot(data = X, aes_string(x = names(X)[1])) +
    geom_line(mapping = aes(y = value, color = key)) +
    geom_point(data = points_for_readability, mapping = aes_string(x = names(X)[1], y = names(X)[3], color = names(X)[2] , shape = names(X)[2])) +
    scale_x_continuous(breaks = x_breaks, labels = x_labels) +
    scale_y_continuous(breaks = y_breaks) +
    scale_color_manual(name = legend_title, labels = my_labels, values = qualitative_hcl(length(rawdata), "Dynamic")) +
    coord_cartesian(xlim = c(min_x, max_x), ylim = c(min_y, max_y)) +
    theme_light(base_size = 12) +
    #theme(plot.title = element_text(hjust = 0.5),
    #      plot.subtitle = element_text(hjust = 0.5)) +
    scale_shape_manual(name = legend_title, values=seq(0,length(my_labels)), guide = "legend") +
    labs(title = title,
         subtitle = subtitle,
         caption = str_wrap(caption, width = 60),
         x = label_x,
         y = label_y)
  
  if (no_legend) {
    p <- p + theme(legend.position = "none")
  } else {
    p <- p + theme(legend.justification = c(0, 1), legend.position = c(0, 1))
  }
  
  p
}
