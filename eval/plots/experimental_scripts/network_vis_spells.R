library(network)
library(networkDynamic)
library(ndtv)
library(viridis)
library(intergraph)
library(threejs)
library(htmlwidgets)


args = commandArgs(trailingOnly=TRUE)

timestamp <- "20190719-172646"
args[1] <- paste("C:/Users/Martin Schonger/source/repos/CREP/log/", timestamp, "__vertex_spells.csv", sep = "")
args[2] <- paste("C:/Users/Martin Schonger/source/repos/CREP/log/", timestamp, "__edge_spells.csv", sep = "")
args[3] <- paste("C:/Users/Martin Schonger/source/repos/CREP/log/", timestamp, "__cluster_dynamics.csv", sep = "")


vertex_spells <- read.csv(args[1], header = TRUE, stringsAsFactors = FALSE)
vertex_spells[, 3] <- vertex_spells[, 3] + 1
vertex_spells[, 4] <- vertex_spells[, 4] + 1
vertex_spells[, 5] <- vertex_spells[, 5] + 1
vertex_spells_mat <- data.matrix(vertex_spells)
num_v <- length(table(vertex_spells$vertex.id))

edge_changes <- read.csv(args[2], header = TRUE, stringsAsFactors = FALSE)
edge_changes[, 3] <- edge_changes[, 3] + 1
edge_changes[, 4] <- edge_changes[, 4] + 1
edge_changes_mat <- data.matrix(edge_changes)

cluster_spells <- read.csv(args[3], header = TRUE, stringsAsFactors = FALSE)
cluster_spells[, 3] <- cluster_spells[, 3] + 1
cluster_spells_mat <- data.matrix(cluster_spells)
num_clusters <- length(table(cluster_spells$vertex.id))


base_net <- network.initialize(num_v, directed = FALSE)

edge_changes_net <- networkDynamic(base.net = base_net, 
                                   vertex.spells = vertex_spells_mat,
                                   edge.spells = edge_changes_mat,
                                   #vertex.pid = c('vertex.id'),
                                   create.TEAs = TRUE,
                                   vertex.TEA.names = c('cluster','component_root','component_size','is_component_root'),
                                   edge.TEA.names = c('weight'))


set.network.attribute(edge_changes_net, 'vertex.pid', 'vertex.names')

base_net_cluster <- network.initialize(num_clusters, directed = FALSE)
cluster_net <- networkDynamic(base.net = base_net_cluster, 
                                   vertex.spells = cluster_spells_mat,
                                   create.TEAs = TRUE,
                                   vertex.TEA.names = c('cluster_size'))


slicy_par <- list(start = 490, end = 500, interval = 1, aggregate.dur = 1, rule = 'latest')

compute.animation(edge_changes_net, 
                  animation.mode = 'kamadakawai', 
                  #default.dist = 16,
                  #chain.direction = 'reverse',
                  weight.attr = 'weight',
                  weight.dist = FALSE,
                  slice.par = slicy_par
                  )

#pdf(tempfile(tmpdir = "C:/Users/Martin Schonger/source/repos/CREP/log", fileext = '.pdf'))
#filmstrip(edge_changes_net, displaylabels=F, slice.par = slicy_par)
#dev.off()

#filmstrip(edge_changes_net, displaylabels=F, mfrow = c(1,1))

#color_tmp <- grDevices::colors()[grep('gr(a|e)y', grDevices::colors(), invert = T)]
#color_tmp_sample <- sample(color_tmp, num_clusters)

num_viridis <- ceiling(num_clusters / 2)
num_magma <- num_clusters - num_viridis

color_tmp_sample <- c(rev(viridis(num_viridis, begin = 0.3)), magma(2*num_magma, begin = 0.3)[1:num_magma])

#pie(rep(1,num_clusters), col=color_tmp_sample)

render.d3movie(edge_changes_net, 
               label = 
                 function(slice){
                   ifelse(slice%v%'is_component_root' == 1, ifelse(slice%v%'component_size' > 1, get.vertex.pid(edge_changes_net, slice%v%'vertex.names'), ""), "")
                 },
               displaylabels = T,
               #vertex.cex = 0.9,
               vertex.cex = 
                 function(slice, onset, terminus){
                   ifelse(slice%v%'is_component_root', 0.4 * sqrt(slice%v%'component_size'), 0.4)
                 },
                 #function(slice, onset, terminus){
                 # ifelse(slice%v%'is_component_root', 0.4 * sqrt(get.vertex.attribute.active(cluster_net, onset = onset, terminus = terminus, rule = 'latest', 'cluster_size')[slice%v%'cluster']), 0.4)
                 #},
               vertex.col = 
                 function(slice){paste(color_tmp_sample[slice%v%'cluster'])},
               edge.col = "darkgrey",
               #edge.col = 
                # function(slice) {
                #   ifelse(get.vertex.attribute(slice, 'component_root')[get.vertex.id(edge_changes_net, slice%e%'tail.vertex.id')] == get.vertex.attribute(slice, 'component_root')[get.vertex.id(edge_changes_net, slice%e%'head.vertex.id')], "darkgrey", "darkgrey")
                # },
               vertex.border = "lightgrey",
               edge.lwd = 'weight',
               edge.tooltip = 
                 function(slice){paste('weight:',slice%e%'weight')}, 
               vertex.tooltip = 
                 function(slice, onset, terminus){paste('id:',get.vertex.pid(edge_changes_net, slice%v%'vertex.names'),'<br>',
                                       'cluster:',slice%v%'cluster','<br>',
                                       'component_size:',slice%v%'component_size','<br>',
                                       'is_component_root:',slice%v%'is_component_root','<br>',
                                       'cluster_size:',get.vertex.attribute.active(cluster_net, onset = onset, terminus = terminus, rule = 'latest', 'cluster_size')[slice%v%'cluster'])},
               render.par = list(tween.frames = 8),
               filename = tempfile(pattern = paste(timestamp,"__animation__",sep = ""), tmpdir = "C:/Users/Martin Schonger/source/repos/CREP/log", fileext = '.html'), 
               launchBrowser = T)


net_495 <- network.collapse(edge_changes_net, at = 495)
igraph_495 <- asIgraph(net_495)

graph_attr(igraph_495, "layout") <- NULL
gjs <- graphjs(igraph_495, main="t=495", bg="white", showLabels=F, stroke=F, 
               curvature=0.1, attraction=0.9, repulsion=0.8, opacity=0.5, vertex.size = 0.5, vertex.color = c(viridis(gorder(igraph_495), begin = 0.3)))
print(gjs)
saveWidget(gjs, file="Media-Network-gjs.html")
browseURL("Media-Network-gjs.html")


