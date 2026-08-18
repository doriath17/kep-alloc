#pragma once
#include <concepts>
#include <cstddef>
#include <type_traits>

namespace kep_alloc::internal {

template <typename T>
concept BackingPolicy = requires(T backing, size_t bytes, void* ptr) {
    {backing.allocate_chunk(bytes)} -> std::convertible_to<void*>;
    {backing.deallocate_chunk(ptr, bytes)} -> std::same_as<void>;
};

}
