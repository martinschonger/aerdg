/*
Header file for run_tests.cpp.
*/

#pragma once

#include <memory>
#include <iostream>
#include <cstdint>
#include <filesystem>

#include "data_source.h"
#include "data_mongodb.h"
#include "data_mongo_offline.h"
#include "data_generated.h"

#if defined(__GNUC__)
#include "static_partitioning.h"
#endif

#include "part_simple.h"

//#include "brp_hops.h"
//#include "brp_heu.h"
//#include "brp_crep.h"
//#include "comp_mig_full.h"
//#include "comp_mig_hop.h"
//#include "comp_mig_greedy.h"
//#include "comp_mig_cc.h"
//#include "comp_mig_dummy.h"
#include "brp_crep_age.h"
#include "comp_mig_age.h"
#include "comp_mig_hop_age.h"
#include "comp_mig_greedy_age.h"
#include "comp_mig_cc_age.h"
#include "utility.h"
#include "logger2.h"

// Copyright (c) 2019 Martin Schonger