// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#define NOMINMAX
#include <windows.h>

/* std */
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <iterator>
#include <unordered_map>
#include <algorithm>
#include <unordered_set>
#include <thread>
#include <fstream>
#include <mutex>

/* boost */
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/optional/optional.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/chrono.hpp>
#include <boost/filesystem.hpp>
#include <boost\bind.hpp>


#include <boost/log/core.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/trivial.hpp>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>


// TODO: reference additional headers your program requires here
