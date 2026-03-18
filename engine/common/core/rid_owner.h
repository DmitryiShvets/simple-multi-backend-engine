#pragma once

#include "rid.h"
#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>
#include <cassert>

namespace ssme {

/**
 * @brief Godot-style RID owner with chunked storage and validator
 * 
 * Provides O(1) allocation, lookup, and deallocation with built-in
 * safety against dangling references through the validator mechanism.
 * 
 * Memory layout:
 *   - Chunked arrays for cache locality
 *   * Free list for O(1) reuse of freed slots
 *   * Validator prevents use-after-free bugs
 * 
 * @tparam T Resource type to own
 * @tparam THREAD_SAFE Enable mutex protection (default: true)
 * @tparam ELEMENTS_PER_CHUNK Elements per chunk (default: 65536 / sizeof(T))
 */
template<typename T, bool THREAD_SAFE = true, size_t ELEMENTS_PER_CHUNK = 0>
class RID_Owner {
public:
    static constexpr size_t DEFAULT_CHUNK_BYTES = 65536;
    static constexpr size_t DEFAULT_ELEMENTS = ELEMENTS_PER_CHUNK > 0 
        ? ELEMENTS_PER_CHUNK 
        : (DEFAULT_CHUNK_BYTES / sizeof(T));

    RID_Owner() 
        : m_elements_per_chunk(DEFAULT_ELEMENTS)
        , m_max_alloc(DEFAULT_ELEMENTS * MAX_CHUNKS)
        , m_alloc_count(0)
        , m_chunk_count(0)
        , m_validator_gen(1)
    {
        // Allocate first chunk
        allocateChunk();
    }

    ~RID_Owner() {
        clear();
        
        // Free all chunks
        for (auto* chunk : m_chunks) {
            delete[] chunk;
        }
        m_chunks.clear();
        
        // Free free lists
        for (auto* free_list : m_free_lists) {
            delete[] free_list;
        }
        m_free_lists.clear();
    }

    // Non-copyable, non-movable
    RID_Owner(const RID_Owner&) = delete;
    RID_Owner& operator=(const RID_Owner&) = delete;
    RID_Owner(RID_Owner&&) = delete;
    RID_Owner& operator=(RID_Owner&&) = delete;

    /**
     * @brief Allocate a new RID and construct resource in-place
     * @tparam Args Constructor argument types
     * @param args Constructor arguments
     * @return New RID with validator
     */
    template<typename... Args>
    RID allocate(Args&&... args) {
        LockGuard lock;
        
        // Check if we need a new chunk
        if (m_alloc_count == m_max_alloc) {
            if (!allocateChunk()) {
                return RID::INVALID;  // Out of memory
            }
        }
        
        // Get free index (from free list or sequential)
        uint32_t index = getFreeIndex();
        
        // Generate new validator
        uint32_t validator = generateValidator();
        
        // Mark as "allocated but uninitialized" (set high bit)
        Chunk& chunk = getChunk(index);
        chunk.validator = validator | UNINITIALIZED_BIT;
        
        // Construct resource in-place
        new (&chunk.data) T(std::forward<Args>(args)...);
        
        // Mark as initialized (clear high bit)
        chunk.validator = validator;
        
        m_alloc_count++;
        
        // Build RID: upper 32 bits = validator, lower 32 bits = index
        uint64_t rid_id = (static_cast<uint64_t>(validator) << 32) | index;
        return RID{rid_id};
    }

    /**
     * @brief Get resource by RID with validation
     * @param rid Resource ID
     * @return Pointer to resource, or nullptr if invalid
     */
    T* get(RID rid) const {
        if (!rid.isValid()) {
            return nullptr;
        }
        
        LockGuard lock;
        
        // Extract index and validator from RID
        uint32_t index = static_cast<uint32_t>(rid.id & 0xFFFFFFFF);
        uint32_t validator = static_cast<uint32_t>(rid.id >> 32);
        
        // Bounds check
        if (index >= m_alloc_count) {
            return nullptr;
        }
        
        // Get chunk
        uint32_t chunk_idx = index / m_elements_per_chunk;
        uint32_t elem_idx = index % m_elements_per_chunk;
        
        if (chunk_idx >= m_chunks.size()) {
            return nullptr;
        }
        
        Chunk& chunk = *m_chunks[chunk_idx];
        
        // Validate validator matches
        if ((chunk.validator & ~UNINITIALIZED_BIT) != validator) {
            return nullptr;  // Validator mismatch (freed or wrong RID)
        }
        
        // Check if initialized
        if (chunk.validator & UNINITIALIZED_BIT) {
            return nullptr;  // Still being constructed
        }
        
        return &chunk.data;
    }

    /**
     * @brief Free a resource by RID
     * @param rid Resource ID to free
     * @return true if freed, false if invalid
     */
    bool free(RID rid) {
        if (!rid.isValid()) {
            return false;
        }
        
        LockGuard lock;
        
        // Extract index and validator
        uint32_t index = static_cast<uint32_t>(rid.id & 0xFFFFFFFF);
        uint32_t validator = static_cast<uint32_t>(rid.id >> 32);
        
        // Bounds check
        if (index >= m_alloc_count) {
            return false;
        }
        
        // Get chunk
        uint32_t chunk_idx = index / m_elements_per_chunk;
        uint32_t elem_idx = index % m_elements_per_chunk;
        
        if (chunk_idx >= m_chunks.size()) {
            return false;
        }
        
        Chunk& chunk = *m_chunks[chunk_idx];
        
        // Validate
        if (chunk.validator != validator) {
            return false;  // Already freed or wrong RID
        }
        
        // Call destructor
        chunk.data.~T();
        
        // Mark as freed (invalid validator)
        chunk.validator = INVALID_VALIDATOR;
        
        // Return index to free list
        returnToFreeList(index);
        
        m_alloc_count--;
        return true;
    }

    /**
     * @brief Check if a RID is valid and owned
     * @param rid Resource ID
     * @return true if valid
     */
    bool isValid(RID rid) const {
        return get(rid) != nullptr;
    }

    /**
     * @brief Get number of live allocations
     */
    uint32_t getLiveCount() const {
        LockGuard lock;
        return m_alloc_count;
    }

    /**
     * @brief Clear all resources (call destructors, reset state)
     */
    void clear() {
        LockGuard lock;
        
        // Call destructors for all live resources
        for (size_t chunk_idx = 0; chunk_idx < m_chunks.size(); chunk_idx++) {
            for (size_t elem_idx = 0; elem_idx < m_elements_per_chunk; elem_idx++) {
                uint32_t index = chunk_idx * m_elements_per_chunk + elem_idx;
                if (index < m_alloc_count) {
                    Chunk& chunk = *m_chunks[chunk_idx];
                    if (chunk.validator != INVALID_VALIDATOR) {
                        chunk.data.~T();
                    }
                }
            }
        }
        
        // Reset state
        m_alloc_count = 0;
        m_chunk_count = 0;
        m_validator_gen = 1;
        
        // Clear free lists
        for (auto* free_list : m_free_lists) {
            delete[] free_list;
        }
        m_free_lists.clear();
        
        // Keep first chunk, remove others
        if (m_chunks.size() > 1) {
            for (size_t i = 1; i < m_chunks.size(); i++) {
                delete[] m_chunks[i];
            }
            m_chunks.resize(1);
            m_chunk_count = 1;
        }
    }

    /**
     * @brief Iterate over all live resources
     * @tparam Func Function type (void(T&))
     * @param func Function to call for each resource
     */
    template<typename Func>
    void forEach(Func&& func) {
        LockGuard lock;
        
        for (size_t chunk_idx = 0; chunk_idx < m_chunks.size(); chunk_idx++) {
            for (size_t elem_idx = 0; elem_idx < m_elements_per_chunk; elem_idx++) {
                uint32_t index = chunk_idx * m_elements_per_chunk + elem_idx;
                if (index < m_alloc_count) {
                    Chunk& chunk = *m_chunks[chunk_idx];
                    if (chunk.validator != INVALID_VALIDATOR) {
                        func(chunk.data);
                    }
                }
            }
        }
    }

private:
    // ========================================================================
    // Internal Structures
    // ========================================================================
    
    /**
     * @brief Chunk containing resource data and validator
     */
    struct Chunk {
        T data;              // Resource data (inline)
        uint32_t validator;  // Validation ID
    };
    
    static constexpr uint32_t INVALID_VALIDATOR = 0xFFFFFFFF;
    static constexpr uint32_t UNINITIALIZED_BIT = 0x80000000;
    static constexpr size_t MAX_CHUNKS = 256;  // Max 256 chunks
    
    // ========================================================================
    // Helper Methods
    // ========================================================================
    
    bool allocateChunk() {
        if (m_chunk_count >= MAX_CHUNKS) {
            return false;  // Max chunks reached
        }
        
        // Allocate new chunk
        Chunk* chunk = new Chunk[m_elements_per_chunk];
        
        // Initialize validators
        for (size_t i = 0; i < m_elements_per_chunk; i++) {
            chunk[i].validator = INVALID_VALIDATOR;
        }
        
        m_chunks.push_back(chunk);
        m_chunk_count++;
        
        // Allocate free list for this chunk
        uint32_t* free_list = new uint32_t[m_elements_per_chunk];
        m_free_lists.push_back(free_list);
        
        return true;
    }
    
    uint32_t getFreeIndex() {
        // Try to get from free list first
        if (m_free_count > 0) {
            m_free_count--;
            uint32_t chunk_idx = (m_alloc_count + m_free_count) / m_elements_per_chunk;
            return m_free_lists[chunk_idx][m_free_count];
        }
        
        // Sequential allocation
        return m_alloc_count;
    }
    
    void returnToFreeList(uint32_t index) {
        uint32_t chunk_idx = index / m_elements_per_chunk;
        uint32_t* free_list = m_free_lists[chunk_idx];
        free_list[m_free_count++] = index;
    }
    
    uint32_t generateValidator() {
        // Generate unique validator (modulo to fit in 31 bits)
        uint32_t validator = 1 + (m_validator_gen++ % 0x7FFFFFFF);
        return validator;
    }
    
    Chunk& getChunk(uint32_t index) {
        uint32_t chunk_idx = index / m_elements_per_chunk;
        uint32_t elem_idx = index % m_elements_per_chunk;
        return m_chunks[chunk_idx][elem_idx];
    }
    
    // ========================================================================
    // Thread Safety
    // ========================================================================
    
    struct LockGuard {
        LockGuard() {}
#ifdef THREAD_SAFE
        LockGuard() : lock_(m_mutex) {}
        mutable std::mutex m_mutex;
#endif
    };
    
    // ========================================================================
    // Member Variables
    // ========================================================================
    
    std::vector<Chunk*> m_chunks;           // Array of chunk pointers
    std::vector<uint32_t*> m_free_lists;    // Free list per chunk
    size_t m_elements_per_chunk;            // Elements per chunk
    uint32_t m_max_alloc;                   // Maximum allocations
    std::atomic<uint32_t> m_alloc_count;    // Current allocations
    std::atomic<uint32_t> m_chunk_count;    // Number of chunks
    std::atomic<uint32_t> m_validator_gen;  // Validator generator
    std::atomic<uint32_t> m_free_count{0};  // Free list size
};

} // namespace ssme
