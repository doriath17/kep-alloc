/**
 * @file kep_alloc.hpp
 * @brief Public interface for the kep_alloc library.
 */

#pragma once

// Note that including headers in this file will make them available to consumers
// since they become part of the public interface.
// For example, headers made to define implementation specific details should not be included here,
// but rather in the source files.
#include "../../src/internal/backing/backing_policy.hpp"
#include "../../src/arena_allocator/arena_allocator.hpp"