#pragma once

#include <exception>

#define interface struct

typedef std::exception NullPointerException;
typedef std::exception IllegalArgumentException;
typedef std::exception OutOfMemoryException;
typedef std::exception CorruptedMemoryException;
