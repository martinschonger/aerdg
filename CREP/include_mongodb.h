#pragma once

#if defined(__GNUC__) || defined(__GNUG__)

//#define PATH_LIB_BSONCXX /usr/local/include/bsoncxx/v_noabi/bsoncxx/
//#define PATH_LIB_MONGOCXX /usr/local/include/mongocxx/v_noabi/mongocxx/

#include "/usr/local/include/bsoncxx/v_noabi/bsoncxx/builder/stream/document.hpp"
#include "/usr/local/include/bsoncxx/v_noabi/bsoncxx/json.hpp"
#include "/usr/local/include/bsoncxx/v_noabi/bsoncxx/builder/basic/kvp.hpp"
#include "/usr/local/include/bsoncxx/v_noabi/bsoncxx/stdx/string_view.hpp"
#include "/usr/local/include/mongocxx/v_noabi/mongocxx/client.hpp"
#include "/usr/local/include/mongocxx/v_noabi/mongocxx/instance.hpp"
#include "/usr/local/include/mongocxx/v_noabi/mongocxx/options/find.hpp"
#include "/usr/local/include/mongocxx/v_noabi/mongocxx/exception/exception.hpp"

#elif defined(_MSC_VER)

//#define PATH_LIB_BSONCXX C:/mongo-cxx-driver/src/bsoncxx/
//#define PATH_LIB_MONGOCXX C:/mongo-cxx-driver/src/mongocxx/

#endif

// Copyright (c) 2019 Martin Schonger