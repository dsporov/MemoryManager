#pragma once

#include <stdexcept>

#define interface struct

typedef std::invalid_argument NullPointerException;
typedef std::invalid_argument IllegalArgumentException;
typedef std::bad_alloc OutOfMemoryException;
typedef std::exception CorruptedMemoryException;
typedef std::logic_error InternalError;
