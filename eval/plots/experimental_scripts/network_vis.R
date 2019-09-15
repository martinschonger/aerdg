require(network)
require(networkDynamic)
require(ndtv)


args = commandArgs(trailingOnly=TRUE)


base_net <- network.initialize(118, directed = FALSE)

#args[1] <- "C:/Users/Martin Schonger/Documents/edge_changes.txt"
edge_changes <- read.csv(args[1], header = TRUE, stringsAsFactors = FALSE)

edge_changes[, 2] <- edge_changes[, 2] + 1
edge_changes[, 3] <- edge_changes[, 3] + 1

edge_changes_mat <- data.matrix(edge_changes)

edge_changes_net <- networkDynamic(base.net = base_net, 
                                   edge.changes = edge_changes_mat)

compute.animation(edge_changes_net, animation.mode = 'kamadakawai', chain.direction = 'reverse')

pdf(tempfile(tmpdir = "C:/Users/Martin Schonger/source/repos/CREP/log", fileext = '.pdf'))
filmstrip(edge_changes_net, displaylabels=T)
dev.off()

render.d3movie(edge_changes_net, 
               displaylabels=T, 
               render.par = list(tween.frames = 5),
               filename = tempfile(tmpdir = "C:/Users/Martin Schonger/source/repos/CREP/log", fileext = '.html'), 
               launchBrowser = T)
