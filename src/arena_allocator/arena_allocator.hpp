/**
 * @file arena_allocator.hpp
 * @brief Header file for the ArenaAllocator class.
 *
 * # Notes
 * The arena allocator allocates a large memory chunk when it is created and allocates in that chunk
 * by simply updating an allocation offset (representing the offset from the start, at the initial
state it will be 0).
 * It is optimal to allocate memory for objects that have the same lifetime so that they can fastly
be allocated
 * O(1) and they can be deallocated all at once O(1).
 *
 * ## Use Cases
 * To understand the allocator better and how the user will use it and what he expects from it, here
there are
 * three use cases.
 *
 * ### Game Loop: per-frame allocations
 * In game engines thousand of temporary objects (like particles) are allocated and discarded every
frame.
 * Usage sequence:
 * @code{.cpp}
// 1. Setup phase (allocated once upfront)
SystemPageBacking backing;
ArenaAllocator arena(1024 * 1024 * 2, backing); // 2MB Arena

while (game_is_running) {
    // 2. Reset offset to 0 at start of frame (instant bulk deallocation)
    arena.reset();

    // 3. Systems allocate scratch memory rapidly during frame update
    auto* particles = static_cast<Particle*>(arena.allocate(sizeof(Particle) * 500,
alignof(Particle))); auto* debug_text = static_cast<char*>(arena.allocate(128, alignof(char)));

    process_particles(particles, 500);
    render_debug_overlay(debug_text);

    // No individual deallocations needed! Everything discarded via arena.reset()
}
 * @endcode
 *
 * ### Scoped / Marker-Based Allocations (Savepoints)
 * Sometimes a function (like when parsing something) needs a temporary memory context to work with,
but the higher level
 * functions needs to retain their allocations. This is like the program function call stack:
 *
 * @code{.cpp}
void process_scene_node(ArenaAllocator& arena) {
    // 1. Take a snapshot of the current allocation offset
    auto marker = arena.get_marker();

    // 2. Allocate temporary working memory needed only within this function
    auto* temp_buffer = static_cast<float*>(arena.allocate(1024 * sizeof(float), alignof(float)));
    calculate_intermediate_transforms(temp_buffer);

    // 3. Allocate persistent result that outlives temporary work

    // 4. Rewind cursor back to snapshot address (frees temp_buffer)
    arena.rewind_to(marker);
}
 * @endcode
 *
 * ### Parsing / Serialization
 * When parsing a JSON, thousand of small nodes are allocated and their lifetimes it tied together:
 * they are created during parsing, used during execution/compilation, destroyed all at once when
the task finishes.
 *
 * @code{.cpp}
void compile_source_file(const std::string& source_code) {
    // 1. Task-scoped arena
    SystemPageBacking backing;
    ArenaAllocator parser_arena(64 * 1024, backing);

    // 2. Build AST nodes using arena memory
    ASTNode* root = parse_tokens(source_code, parser_arena);

    // 3. Process AST
    execute_ast(root);

    // 4. End of scope: parser_arena destructor fires, freeing all AST nodes simultaneously
}
 * @endcode
 *
 * ## Takeaways
 * - the user should be able to define the alignment of the data he wants to allocate on the arena
 * this is important to guarantee the correct working of the allocated object since the cpu and the
hardware
 * expects to work with aligned memory pointers. See the alignment section
 *
 *
 *
 * # Alignment
 * def. a variable is said to be n-byte aligned if its starting memory address is a multiple of n.
 * > Why is it important?
 * if an object is not aligned in memory it could span across two cache lines or two words.
 * This is suboptimal since the cpu then needs to do some operation to correctly handle that object,
 * like shifting its bit to make it aligned. So to avoid this, objects should be aligned.
 * Furthermore on some system unaligned objects could cause alignment faluts.
 * > How to align an object in memory?
 * Each object or value stored in memory has an alignment requirement.
 * - for primitive built-in types it usually corresponds to their size
 *      - a char has alignment = 1
 *      - a 4 byte int needs an address divisible by 4 (it is 4byte aligned)
 *      - a 8 byte double needs an address divisible by 8 (alignment = 8, it is 8byte aligned)
 *
 * - for compound types like structs or classes the situation is quite different and it mainly
depends on the members
 * alignment reqs. and specifically by the member with the largest alignment requirement. For
example:
 * @code{.cpp}
struct Example {
    int32_t a;  // 4 bytes (alignment 4)
    char b;     // 1 byte  (alignment 1)
};
 * @endcode
 * Here the memeber with the largerst alignment req. is the integer with 4-bytes alignment. So the
entire struct
 * has an alignment req. of 4-bytes.
 * Note that in this case the total size of the structure is 8byte (not 5, because you need to
consider the padding)
 * and the requirement for the alignment is 4byte. This means that the alignment req. does not
always correspond to the size.
 *
 * ## Useful for the alignment
 * - `alignof(T)` -> to obtain the alignment requirement of the type T
 * - `std::max_align_t` -> it is a POD (plain old data) type whose alignment requirement is at least
as strict as
 * that of every scalar type in C++.
 * On most 64-bit x86/ARM platforms it is equal to 16bytes matching types like `long double`.
 * On 32-bit platforms it is typically 8bytes.
 * It is commonly used to provide a default value for when it comes to allocate something and you
dont have an
 * alignment requirement specified. So, almost like a convention, the allocator should use
`alignof(std::max_align_t)`
 * to align the given type. This guarantees that any standard C++ type can be safely placed at that
returned address.
 * By the way, if you think of it, `std::max_align_t` represent the largest alignment req. a compond
type member can have,
 * so at the end of day, it represents the max alignment any standard type could have (it is like
maximising it).
 *
 */

#pragma once

#include "../internal/backing/backing_policy.hpp"
#include "../internal/utils.hpp"

#include <cstddef>
#include <new>
#include <stdexcept>

namespace kep_alloc {

template <internal::BackingPolicy backing_policy = internal::SystemPageBacking>
class ArenaAllocator {
  public:
    ArenaAllocator(backing_policy& bp, size_t init_size) : m_backing(bp) {
        // This is the upfront allocation
        m_size = internal::normalize_size(init_size);
        m_base_ptr = m_backing.allocate_chunk(m_size);
        if (m_base_ptr == nullptr) {
            throw std::bad_alloc();
        }
        m_alloc_ptr = m_base_ptr;
    }

    ~ArenaAllocator() { m_backing.deallocate_chunk(m_base_ptr, m_size); }

    // TODO: implement copy and move
    ArenaAllocator(const ArenaAllocator& source) {
        auto new_arena = ArenaAllocator(source.m_backing, source.m_size);
        memcpy(new_arena.m_base_ptr, source.m_base_ptr, source.get_marker());
        new_arena.rewind_to(source.get_marker());
    }

    ArenaAllocator(ArenaAllocator&& source) noexcept {
        m_backing = source.m_backing;
        m_size = source.m_size;
        m_base_ptr = source.m_base_ptr;
        m_alloc_ptr = source.m_alloc_ptr;
        source.m_base_ptr = nullptr;
        source.m_alloc_ptr = nullptr;
        source.m_size = 0;
    }

    /**
     * @brief Allocates a memory chunk on the arena
     *
     * @param size The size in bytes of the memory chunk to allocate
     * @param alignment The alignment requirement for the object allocating memory to
     * @return the pointer to the initial address of the allocated memory chunk
     */
    [[nodiscard]] void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        size_t raw_address = reinterpret_cast<size_t>(m_alloc_ptr);
        size_t aligned_address = raw_address;
        if (!internal::is_aligned(aligned_address, alignment)) {
            aligned_address = internal::align(aligned_address, alignment);
        }

        if (aligned_address + size > reinterpret_cast<size_t>(m_base_ptr) + m_size) [[unlikely]] {
            return nullptr; // Not enough space in the arena
        }

        m_alloc_ptr = reinterpret_cast<void*>(aligned_address + size);
        return m_alloc_ptr;
    }

    /**
     * @brief Deallocates a memory chunk on the arena (no-op for arena allocator)
     *
     * This function is a no-op for the arena allocator since individual deallocations are not
     * supported.
     *
     * @param ptr The pointer to the memory chunk to deallocate
     */
    void deallocate(void* ptr) const noexcept {}

    /**
     * @brief Resets the arena allocator to its initial state, effectively deallocating all
     * allocated memory.
     */
    void reset() { m_alloc_ptr = m_base_ptr; }

    [[nodiscard]] inline size_t get_marker() const noexcept {
        return reinterpret_cast<size_t>(m_alloc_ptr) - reinterpret_cast<size_t>(m_base_ptr);
    }

    inline void rewind_to(size_t marker) {
        if (marker > m_size) [[unlikely]] {
            throw std::invalid_argument("Marker is out of bounds");
        }

        m_alloc_ptr = reinterpret_cast<void*>(reinterpret_cast<size_t>(m_base_ptr) + marker);
    }

  private:
    backing_policy& m_backing; /// the backing system used to allocate the arena memory
    size_t m_size;             /// the total size of the arena in bytes

    void* m_base_ptr = nullptr; /// the base pointer to the allocated arena memory
    // Note that the first available raw byte is given by: `m_base_ptr + m_offset`
    void* m_alloc_ptr = nullptr; /// the pointer to the first available byte in the arena
};

} // namespace kep_alloc