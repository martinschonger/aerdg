// CREP.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>


#include "include_boost.h"
#include "../lib/rapidjson/document.h"
#include "include_mongodb.h"

#include "typedefs.h"
#include "data_source.h"
#include "data_mongodb.h"
#include "data_json.h"
#include "logger.h"
#include "dynamic_partitioning.h"

#if defined(__GNUC__)
#include "static_partitioning.h"
#endif

// Copyright (c) 2019 Martin Schonger