#pragma once

#include <iostream>
#include <vector>
#include <algorithm>

#include "typedefs.h"
#include "include_mongodb.h"
#include "comp_graph.h"
#include "data_mongodb.h"
#if defined(__GNUC__)
#include "static_partitioning.h"
#endif

#include "debug.h"

// Copyright (c) 2019 Martin Schonger