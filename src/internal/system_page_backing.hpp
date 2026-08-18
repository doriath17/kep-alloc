#pragma once

namespace kep_alloc::internal {

/**
 * @brief Default strategy to acquire memory from the OS.
 * 
 * ## Notes
 * What i want to achieve is to define the main front-end allocators with templates
 * and instantiating the template with a specific memory allocation strategy. 
 * 
 * For example: 
 * @code{.cpp}
 * template <typename T, typename Backing = internal::SystemPageBacking>
 * class MyAllocator {...}
 * @endcode
 * 
 * In this way the allocators can be instantiated by the user with much more flexibility. 
 * The user can instantiate the allocator based on its needs. 
 * 
 * The SystemPageBacking is the simplest and more direct way to request memory from the 
 * operative system. It just use `mmap` and `munmap` to allocate and deallocate memory. 
 * But the user could just use a more suitable memory backing system for its use case. 
 * 
 * Obviosly this simple implementation has some disadvantages, depending on the allocator 
 * that uses it. For example if a slab allocator have to create a thousand slabs, 
 * if is uses this class to request memory to the os, it will need to request memory
 * one thousand times. Instead a much better approach could be to use another backing 
 * system to provide one large area. 
 */
class SystemPageBacking {

};

}
