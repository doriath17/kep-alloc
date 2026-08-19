
/**
 * @file slab_allocator.hpp
 * @brief Header file for the SlabAllocator class.
 * 
 * # Notes
 * > Why does a user want to use this allocator?
 * The slab allocator is especially good to allocate and deallocate objects with high frequency and with O(1) time.
 * This is opposed to the ArenaAllocator where you cant deallocate a single object but you are constrained to 
 * deallocate all objects at once to have fast allocation in O(1) time. 
 * The purpose is different, and different are the lifetimes of the objects you want to store: 
 * - arena allocator: all objects have similar lifetimes
 * - slab allocator: each individual objects has its own lifetime
 * This is a huge difference to be considered. 
 * 
 * Anyway, the slab allocator i am going to implement will have a bunch of feature that the user could rely on 
 * when he wants to use it, so that he can make assumptions on it and use it in the best way he can based on his use case. 
 * 
 * ## Template Definition
 * The slab allocator is a template and takes some template parameters:
 * - backing storage: to define how the memory is requested. 
 * - a type T: this is particularly important since from this type you can infer:
 *  - the size of T
 *  - alignment requirements of T
 * - a preferred slab size: so the user can express his preference on how big should a slab be. 
 * 
 * ### The Slot Size & ByteSlot
 * Usually the slab allocator is intended to be allocated for a type T and so its 
 * slots would be of size equals to `sizeof(T)`. However the user could also want to
 * instantiate a slab allocator based on a specific slot size rather than a type. 
 * In this case the main suggested approach is to use the dedicated type `ByteSlot<N>`,
 * which is defined type made just for this scenario and allow to write something like:
 * - `SlabAllocator<ByteSlot<4>, ...>` (slab allocator with 4 bytes slots)
 * 
 * Alternatively, the user could use: 
 * - `SlabAllocator<std::array<std::byte, 4>>`
 * - 4-bytes: `SlabAllocator<std::uint32_t>`
 * - 8-bytes: `SlabAllocator<std::uint64_t>`
 * - 16-bytes: `SlabAllocator<std::uint128_t>` (if supported)
 * 
 * The slab is intended to be instantianted per-type. So i would imagine that i can instantiate a slab for my type T. 
 * The approach that i am proposing does not allow to create a slab allocator for a speci
 * 
 * 
 * ## The Slab
 * Basically a slub is a chunk of memory allocated through the backing storage. 
 * It has a base pointer that allows you to refer to it and it has a size (how many bytes you can allocate in it).
 * The slab can be divided in slots, each of which stores a value of type T. 
 * Based on the slab size and `sizeof(T)` you can compute how many slots the slab could hold:
 * > `slot_capacity = slab_size / sizeof(T)` 
 * > NOTE: C++ guarantees that `sizeof(T)` is a multiple of `alignof(T)`
 * 
 * ### How is the slab allocated? 
 * The user can specify the preferred slab size through a template parameter.
 * NOTE: the choice of a template parameter is not random. I choose this to aim for the lowest overhead possible. 
 * During template instantiation at runtime all the math is done. But this could bring to code bloat since
 * SlabAllocator<size = 4095> is different from SlabAllocator<size = 4096>. The user could align the size 
 * with the page size to avoid compiling the same class unnecessarily (shortly i will explain why).
 * 
 * This size represents the preferred size the user wants or expects the size to be. 
 * However, since the minimum memory chunk you can allocate on using the SystemPageBacking has 
 * the size of a memory page, i dediced that the slab allocator will align the size parameter up to a page size multiplier. 
 * So the two example above will effectively result both in SlabAllocator<size = 4096> (considering a page size of 4KB).  
 * 
 * ### Using ArenaBacking
 * Generally the user should try to avoid situations where slab allocation happens frequently.
 * This could have several drawbacks:
 * - system call overhead
 * - cache locality is harder since slab pages could be sparse in memory
 * To minimize sys call overhead and improve cache locality the user could allocate upfront a large chunk of memory and let
 * the slab allocator allocate from there. Hence, the ArenaBacking solution comes in handy. 
 * However, and this should be obvious, the cons of this approach is that slabs cannot be deallocated one by one but their lifetime
 * is tied together. 
 * NOTE: when using the arena backing policy, the user can setup the arena before passing it into the constructor. 
 * This approach is actually interesting since the concept of BackingPolicy just expose functions to allocate and deallocate and
 * does not allow to tune the backing strategy at all (at least for now). So what the user could do is to tune the backing strategy 
 * as he wantsa and then pass it to the allocator. This is actually so grate and elegant! Yeah I should find a girl. 
 *  
 * 
 * 
 */

#pragma once

namespace kep_alloc {

}