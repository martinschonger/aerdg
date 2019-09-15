/*
Debugging helper macros.
*/

#pragma once

#define DREACH(x) std::cerr << "[DEBUG] Reached point " << x << std::endl;
#define DLOG(x) std::cerr << #x << " = " << x << std::endl;

// Copyright (c) 2019 Martin Schonger