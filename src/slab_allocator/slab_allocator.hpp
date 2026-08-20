
/**
 * @file slab_allocator.hpp
 * @brief Header file for the SlabAllocator class.
 *
 * # Notes
 * > Why does a user want to use this allocator?
 * The slab allocator is especially good to allocate and deallocate objects with high frequency and
 * with O(1) time. This is opposed to the ArenaAllocator where you cant deallocate a single object
 * but you are constrained to deallocate all objects at once to have fast allocation in O(1) time.
 * The purpose is different, and different are the lifetimes of the objects you want to store:
 * - arena allocator: all objects have similar lifetimes
 * - slab allocator: each individual objects has its own lifetime
 * This is a huge difference to be considered.
 *
 * Anyway, the slab allocator i am going to implement will have a bunch of feature that the user
 * could rely on when he wants to use it, so that he can make assumptions on it and use it in the
 * best way he can based on his use case.
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
 * The slab is intended to be instantianted per-type. So i would imagine that i can instantiate a
 * slab for my type T. The approach that i am proposing does not allow to create a slab allocator
 * for a speci
 *
 *
 * ## The Slab
 * Basically a slub is a chunk of memory allocated through the backing storage.
 * It has a base pointer that allows you to refer to it and it has a size (how many bytes you can
 * allocate in it). The slab can be divided in slots, each of which stores a value of type T. Based
 * on the slab size and `sizeof(T)` you can compute how many slots the slab could hold: >
 * `slot_capacity = slab_size / sizeof(T)` > NOTE: C++ guarantees that `sizeof(T)` is a multiple of
 * `alignof(T)`
 *
 * ### Defining a preferred slab size
 * The user can specify the preferred slab size through a template parameter.
 * NOTE: the choice of a template parameter is not random. I choose this to aim for the lowest
 * overhead possible. During template instantiation at runtime all the math is done. But this could
 * bring to code bloat since SlabAllocator<size = 4095> is different from SlabAllocator<size =
 * 4096>. The user could align the size with the page size to avoid compiling the same class
 * unnecessarily (shortly i will explain why).
 *
 * This size represents the preferred size the user wants or expects the size to be.
 * However, since the minimum memory chunk you can allocate on using the SystemPageBacking has
 * the size of a memory page, the effective allocated chunk size will always be
 * a page size multiplier. So the two example above will effectively result both in
 * SlabAllocator<size = 4096> (considering a page size of 4KB).
 * So, to sum up, the user can pass a preferred size, but the actual allocated memory
 * is always aligned to the closest bigger multiplier of the page size.
 * NOTE: this behavior is caused by the workings on the mmap. If the backing policy
 * doesnt use mmap, this behavior could change (ex. with a static backing).
 *
 * ### Using ArenaBacking
 * Generally the user should try to avoid situations where slab allocation happens frequently.
 * This could have several drawbacks:
 * - system call overhead
 * - cache locality is harder since slab pages could be sparse in memory
 * To minimize sys call overhead and improve cache locality the user could allocate upfront a large
 * chunk of memory and let the slab allocator allocate from there. Hence, the ArenaBacking solution
 * comes in handy. However, and this should be obvious, the cons of this approach is that slabs
 * cannot be deallocated one by one but their lifetime is tied together. NOTE: when using the arena
 * backing policy, the user can setup the arena before passing it into the constructor. This
 * approach is actually interesting since the concept of BackingPolicy just expose functions to
 * allocate and deallocate and does not allow to tune the backing strategy at all (at least for
 * now). So what the user could do is to tune the backing strategy as he wantsa and then pass it to
 * the allocator. This is actually so grate and elegant! Yeah I should find a girl.
 *
 * ### Slab Management
 * To manage slabs the allocator needs to store some metadata for each of them.
 * This metadata is stored in what i will call a header from now on.
 * However another question that raise is: where do i take the memory to allocate
 * those headers? There are two main solutions that i have thought about:
 * - embedded headers -- allocated within the slab
 * - external headers -- allocated in a dedicated slab containing headers
 * I decided to use and implemented the embedded strategy first.
 * The user should be aware that, for each slab, a small amount of memory is dedicated
 * to store the header. So even though the user specify a preferred size the actual
 * usable size of the slab should take the header into consideration.
 *
 * NOTE: in a slab allocator based on mmap, slab size is a multiplier of the page size
 * and it uses a small amount of memory to allocate the header.
 *
 */

#pragma once
#include "../internal/backing/backing_storage.hpp"

#include <cstddef>

namespace kep_alloc {

template <size_t N> class ByteSlot {
    // TODO: improve the alignment logic to avoid unnecessary padding
    alignas(std::max_align_t) std::byte data[N];
};

/**
 * # Embedded slots free list (intrusive list)
 * The slab maintains a pointer to a list of slots. This list
 * is embedded in the free slots themselves: these slots are not
 * actually used so the free space is used to store a pointer to
 * the next free slot (the intrusive pointer)
 * NOTE: this decision actually impose another constraint the user
 * should be aware of: the minimum slot size a slab can have is 8-bytes
 * since a slot must be able to store a pointer.
 *
 * # Lazy initialization vs Eager initialization
 * To initialize the intrusive list, i can use an eager or a lazy strategy.
 * The eager strategy will iterate over each uninitialized slot and
 * initialize it. This is clearly a O(n) operation done only once,
 * when the slab is allocated.
 * The lazy approach, instead, will store a `m_next_uninitialized_slot`
 * member in the header to allow a seconf path of allocation:
 * - main path: use the `m_free_list_head`
 * - bump path: use the `m_next_uninitialized_slot` to allocate from the
 * next uninitialized slot and update the bump index.
 * This approach will avoid the O(n) cost during initialization with a
 * simple check on the free list head pointer:
 * `if (m_free_list_head != nullptr) { // use initialized slot }`
 * From what i understood, this check cost is virtually zero since cpus
 * nowadays can predict if the branch is always true if it follows a
 * certain pattern and it doesnt flip unpredictably from true to false to true.
 * And this is my case where this condition eventually will become always
 * true when each slot has been initialized.
 *
 * So this is why a lazy initialization of the intrusive list seems like
 * the most effective choice.
 *
 * # Header content
 * - m_free_list_head
 * - m_next_uninitialized_slot
 * - m_allocated_slots
 * - m_slot_capacity (constexpr)
 *
 * These pointer allow to implement the slab lists
 * - m_next
 * - m_prev
 *
 */
template <typename T, size_t slab_size> class Slab {
  public:
    /**
     * NOTE:
     * - the base_ptr must be a valid pointer and it must be the
     * initial address of an allocated memory chunk of slab_size bytes.
     *
     * @param base_ptr the base ptr of the allocated slab
     */
    Slab(void* base_ptr) noexcept {
        size_t raw_first_slot_ptr = reinterpret_cast<size_t>(base_ptr) + sizeof(Slab);

        m_next_uninitialized_slot =
            reinterpret_cast<void*>(internal::check_and_align(raw_first_slot_ptr, alignof(T)));
    }

    void set_next(void* next) noexcept { m_next = next; }
    void set_prev(void* prev) noexcept { m_prev = prev; }

    /**
     * @return a pointer to the allocated slot or nullptr if the slab has
     * no more memory.
     */
    [[nodiscard]] void* allocate() noexcept {
        if (m_allocated_slots == capacity) [[unlikely]] {
            return nullptr;
        }

        void* available_slot = nullptr;
        if (m_free_list_head != nullptr) [[likely]] {
            available_slot = m_free_list_head;
            void** next_ptr = static_cast<void**>(m_free_list_head);
            m_free_list_head = *next_ptr;
        } else [[unlikely]] {
            available_slot = m_next_uninitialized_slot;
            m_next_uninitialized_slot = reinterpret_cast<void*>(
                reinterpret_cast<size_t>(m_next_uninitialized_slot) + sizeof(T));
        }

        m_allocated_slots++;
        return available_slot;
    }

    void deallocate(void* ptr) noexcept {
        if (ptr == nullptr) [[unlikely]] {
            return;
        }

        const auto raw_ptr = reinterpret_cast<std::uintptr_t>(ptr);
        const bool in_range = (raw_ptr >= m_first_slot_ptr) && (raw_ptr <= last_slot_ptr());

        if (!in_range || !internal::is_aligned(raw_ptr, alignof(T))) [[unlikely]] {
            return;
        }

        // store the current head inside the slot
        void** slot_ptr = static_cast<void**>(ptr);
        *slot_ptr = m_free_list_head;

        // push the slot at the list head
        m_free_list_head = ptr;
    }

    inline std::uintptr_t last_slot_ptr() const noexcept {
        return m_first_slot_ptr + usable_bytes - sizeof(T);
    }

  private:
    void* m_free_list_head = nullptr;
    void* m_next_uninitialized_slot = nullptr;

    void* m_next = nullptr;
    void* m_prev = nullptr;

    size_t m_allocated_slots = 0;
    std::uintptr_t m_first_slot_ptr = 0;

    static constexpr size_t usable_bytes = slab_size > sizeof(Slab) ? slab_size - sizeof(Slab) : 0;
    static constexpr size_t slot_size = std::max(sizeof(T), sizeof(void*));
    static constexpr size_t capacity = usable_bytes / slot_size;
};

template <typename T, internal::BackingStorage backing_storage = internal::SystemPageBackingStorage,
          size_t pref_slab_size>
class SlabAllocator {};

} // namespace kep_alloc