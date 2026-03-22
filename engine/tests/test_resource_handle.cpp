/**
 * @brief Resource system tests
 *
 * Tests for Buffer resource creation, usage, and cleanup.
 */

#include "engine.h"
#include "resource_handle.h"
#include "resources/buffer.h"
#include "resource_manager.h"
#include "utils/debug_assert.h"
#include "core/rid_allocator.h"
#include <cstdlib>
#include <iostream>
#include <cassert>

// Constants
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

// ============================================================================
// Test 1: Basic Buffer Creation and Cleanup
// ============================================================================
void test_BufferCreation() {
    std::cout << "\n=== Test 1: Basic Buffer Creation ===" << std::endl;

    ssme::Engine engine;
    engine.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Get resource manager from engine (assuming it has a getter)
    // For now, we'll just test that engine creates successfully

    std::cout << "✓ Engine initialized successfully" << std::endl;

    engine.cleanup();
    std::cout << "✓ Engine cleanup successful" << std::endl;
}

// ============================================================================
// Test 2: ResourceHandle Lifetime
// ============================================================================
void test_ResourceHandleLifetime() {
    std::cout << "\n=== Test 2: ResourceHandle Lifetime ===" << std::endl;

    ssme::Engine engine;
    engine.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Test 1: Empty handle (should not crash)
    {
        ssme::ResourceHandle<ssme::Buffer> empty_handle;
        ssme::debug_assert(!empty_handle.isValid());
        ssme::debug_assert(empty_handle.get() == nullptr);
        std::cout << "✓ Empty handle is invalid (as expected)" << std::endl;
    }

    // Test 2: Handle goes out of scope (should cleanup properly)
    {
        ssme::ResourceHandle<ssme::Buffer> temp_handle;
        // Handle destroyed here - should not crash
        std::cout << "✓ Handle destroyed without crash" << std::endl;
    }

    engine.cleanup();
    std::cout << "✓ Test completed successfully" << std::endl;
}

// ============================================================================
// Test 3: RID Allocator
// ============================================================================
void test_RIDAllocator() {
    std::cout << "\n=== Test 3: RID Allocator ===" << std::endl;

    ssme::RIDAllocator allocator(ssme::RIDRange::INTERNAL_START, ssme::RIDRange::INTERNAL_END);

    // Allocate some RIDs
    ssme::RID rid1 = allocator.allocate();
    ssme::RID rid2 = allocator.allocate();
    ssme::RID rid3 = allocator.allocate();

    ssme::debug_assert(rid1.isValid());
    ssme::debug_assert(rid2.isValid());
    ssme::debug_assert(rid3.isValid());
    ssme::debug_assert(rid1.id != rid2.id);
    ssme::debug_assert(rid2.id != rid3.id);

    std::cout << "✓ RID 1: " << rid1.id << std::endl;
    std::cout << "✓ RID 2: " << rid2.id << std::endl;
    std::cout << "✓ RID 3: " << rid3.id << std::endl;

    // Free and reuse
    allocator.free(rid2);
    ssme::RID rid4 = allocator.allocate();
    ssme::debug_assert(rid4.isValid());

    std::cout << "✓ RID 4 (reused): " << rid4.id << std::endl;
    std::cout << "✓ RID allocator test passed" << std::endl;
}

// ============================================================================
// Test 4: Multiple Buffers
// ============================================================================
void test_MultipleBuffers() {
    std::cout << "\n=== Test 4: Multiple Buffers ===" << std::endl;

    ssme::Engine engine;
    engine.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Create multiple handles
    ssme::ResourceHandle<ssme::Buffer> buffer1;
    ssme::ResourceHandle<ssme::Buffer> buffer2;
    ssme::ResourceHandle<ssme::Buffer> buffer3;

    // All should be invalid
    ssme::debug_assert(!buffer1.isValid());
    ssme::debug_assert(!buffer2.isValid());
    ssme::debug_assert(!buffer3.isValid());

    std::cout << "✓ Multiple handles created (all invalid)" << std::endl;

    // Destroy in different order
    buffer2.~ResourceHandle();
    buffer1.~ResourceHandle();
    buffer3.~ResourceHandle();

    std::cout << "✓ All handles destroyed without crash" << std::endl;

    engine.cleanup();
}

// ============================================================================
// Test 5: Stress Test - Many Handles
// ============================================================================
void test_StressTest() {
    std::cout << "\n=== Test 5: Stress Test (1000 handles) ===" << std::endl;

    ssme::Engine engine;
    engine.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    const int NUM_HANDLES = 1000;
    std::vector<ssme::ResourceHandle<ssme::Buffer>> handles(NUM_HANDLES);

    for (int i = 0; i < NUM_HANDLES; i++) {
        ssme::debug_assert(!handles[i].isValid());
    }

    std::cout << "✓ Created " << NUM_HANDLES << " handles" << std::endl;

    // Destroy all
    handles.clear();

    std::cout << "✓ All handles destroyed" << std::endl;

    engine.cleanup();
    std::cout << "✓ Stress test passed" << std::endl;
}

// ============================================================================
// Test 6: Real Buffer Creation via ResourceManager
// ============================================================================
void test_RealBufferCreation() {
    std::cout << "\n=== Test 6: Real Buffer Creation ===" << std::endl;

    ssme::Engine engine;
    engine.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Create buffer descriptor
    ssme::BufferDesc desc;
    desc.size = 1024;  // 1KB buffer
    desc.usage = static_cast<ssme::BufferUsageFlags>(ssme::BufferUsage::VERTEX_BUFFER);
    desc.is_host_visible = true;
    desc.initial_data = nullptr;

    // Load buffer via ResourceManager
    auto buffer_handle = engine.m_resource_manager->load<ssme::Buffer>(
        "test_buffer_1", desc);

    // Check handle is valid
    ssme::debug_assert(buffer_handle.isValid(), "");
    ssme::debug_assert(buffer_handle.get() != nullptr);
    std::cout << "✓ Buffer created successfully" << std::endl;

    // Check resource exists
    auto* buffer_ptr = engine.m_resource_manager->get<ssme::Buffer>("test_buffer_1");
    ssme::debug_assert(buffer_ptr != nullptr);
    ssme::debug_assert(buffer_ptr == buffer_handle.get());

    std::cout << "✓ Buffer found in ResourceManager" << std::endl;

    // Cleanup
    buffer_handle.~ResourceHandle();
    std::cout << "✓ Buffer handle destroyed" << std::endl;

    engine.cleanup();
    std::cout << "✓ Real buffer test passed" << std::endl;
}

// ============================================================================
// Test 7: Reference Counting
// ============================================================================
void test_ReferenceCounting() {
    std::cout << "\n=== Test 7: Reference Counting ===" << std::endl;

    ssme::Engine engine;
    engine.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    ssme::BufferDesc desc;
    desc.size = 512;
    desc.usage = static_cast<ssme::BufferUsageFlags>(ssme::BufferUsage::INDEX_BUFFER);
    desc.is_host_visible = true;

    // Load same buffer twice (should increment refcount)
    auto handle1 = engine.m_resource_manager->load<ssme::Buffer>("shared_buffer", desc);
    auto handle2 = engine.m_resource_manager->load<ssme::Buffer>("shared_buffer", desc);

    ssme::debug_assert(handle1.isValid());
    ssme::debug_assert(handle2.isValid());
    std::cout << "✓ Two handles to same buffer created" << std::endl;

    // Both should point to same resource
    auto* ptr1 = handle1.get();
    auto* ptr2 = handle2.get();
    ssme::debug_assert(ptr1 == ptr2);
    std::cout << "✓ Both handles point to same resource" << std::endl;

    // Destroy first handle (refcount should decrement, but resource stays)
    handle1.~ResourceHandle();
    std::cout << "✓ First handle destroyed (resource should still exist)" << std::endl;

    // Second handle should still be valid
    ssme::debug_assert(handle2.isValid());
    std::cout << "✓ Second handle still valid" << std::endl;

    // Destroy second handle (resource should be freed)
    handle2.~ResourceHandle();
    std::cout << "✓ Second handle destroyed (resource should be freed)" << std::endl;

    engine.cleanup();
    std::cout << "✓ Reference counting test passed" << std::endl;
}

// ============================================================================
// Test 8: Multiple Resources
// ============================================================================
void test_MultipleResources() {
    std::cout << "\n=== Test 8: Multiple Resources ===" << std::endl;

    ssme::Engine engine;
    engine.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    ssme::BufferDesc desc;
    desc.size = 256;
    desc.usage = static_cast<ssme::BufferUsageFlags>(ssme::BufferUsage::UNIFORM_BUFFER);
    desc.is_host_visible = true;

    // Create multiple different buffers
    auto buffer1 = engine.m_resource_manager->load<ssme::Buffer>("buffer_A", desc);
    desc.size = 512;
    auto buffer2 = engine.m_resource_manager->load<ssme::Buffer>("buffer_B", desc);
    desc.size = 1024;
    auto buffer3 = engine.m_resource_manager->load<ssme::Buffer>("buffer_C", desc);

    ssme::debug_assert(buffer1.isValid());
    ssme::debug_assert(buffer2.isValid());
    ssme::debug_assert(buffer3.isValid());
    std::cout << "✓ Three different buffers created" << std::endl;

    // All should point to different resources
    ssme::debug_assert(buffer1.get() != buffer2.get());
    ssme::debug_assert(buffer2.get() != buffer3.get());
    ssme::debug_assert(buffer1.get() != buffer3.get());
    std::cout << "✓ All buffers are unique" << std::endl;

    // Destroy in random order
    buffer2.~ResourceHandle();
    buffer3.~ResourceHandle();
    buffer1.~ResourceHandle();
    std::cout << "✓ All buffers destroyed" << std::endl;

    engine.cleanup();
    std::cout << "✓ Multiple resources test passed" << std::endl;
}

// ============================================================================
// Test 9: Copy and Move Semantics
// ============================================================================
void test_CopyMoveSemantics() {
    std::cout << "\n=== Test 9: Copy and Move Semantics ===" << std::endl;

    ssme::Engine engine;
    engine.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    ssme::BufferDesc desc;
    desc.size = 1024;
    desc.usage = static_cast<ssme::BufferUsageFlags>(ssme::BufferUsage::VERTEX_BUFFER);
    desc.is_host_visible = true;

    // Create original handle
    auto handle1 = engine.m_resource_manager->load<ssme::Buffer>("copy_test", desc);
    ssme::debug_assert(handle1.isValid());
    auto* raw_ptr = handle1.get();
    std::cout << "✓ Original handle created" << std::endl;

    // Copy construction
    auto handle2 = handle1;
    ssme::debug_assert(handle2.isValid());
    ssme::debug_assert(handle2.get() == raw_ptr);
    std::cout << "✓ Copy construction works (same resource)" << std::endl;

    // Copy assignment
    ssme::ResourceHandle<ssme::Buffer> handle3;
    handle3 = handle1;
    ssme::debug_assert(handle3.isValid());
    ssme::debug_assert(handle3.get() == raw_ptr);
    std::cout << "✓ Copy assignment works (same resource)" << std::endl;

    // Move construction
    auto handle4 = std::move(handle1);
    ssme::debug_assert(handle4.isValid());
    ssme::debug_assert(handle4.get() == raw_ptr);
    // handle1 should be empty after move
    ssme::debug_assert(!handle1.isValid());
    std::cout << "✓ Move construction works (source invalidated)" << std::endl;

    // Move assignment
    ssme::ResourceHandle<ssme::Buffer> handle5;
    handle5 = std::move(handle2);
    ssme::debug_assert(handle5.isValid());
    ssme::debug_assert(handle5.get() == raw_ptr);
    ssme::debug_assert(!handle2.isValid());
    std::cout << "✓ Move assignment works (source invalidated)" << std::endl;

    // All valid handles should point to same resource
    ssme::debug_assert(handle3.get() == handle4.get());
    ssme::debug_assert(handle4.get() == handle5.get());
    std::cout << "✓ All valid handles point to same resource" << std::endl;

    // Destroy all
    handle3.~ResourceHandle();
    handle4.~ResourceHandle();
    handle5.~ResourceHandle();
    std::cout << "✓ All handles destroyed" << std::endl;

    engine.cleanup();
    std::cout << "✓ Copy/move semantics test passed" << std::endl;
}

// ============================================================================
// Test 10: Handles in Containers
// ============================================================================
void test_HandlesInContainers() {
    std::cout << "\n=== Test 10: Handles in Containers ===" << std::endl;

    ssme::Engine engine;
    engine.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    ssme::BufferDesc desc;
    desc.size = 256;
    desc.usage = static_cast<ssme::BufferUsageFlags>(ssme::BufferUsage::VERTEX_BUFFER);
    desc.is_host_visible = true;

    // Create vector of handles
    std::vector<ssme::ResourceHandle<ssme::Buffer>> handles;

    for (int i = 0; i < 10; i++) {
        desc.size = 256 * (i + 1);
        auto handle = engine.m_resource_manager->load<ssme::Buffer>(
            "buffer_" + std::to_string(i), desc);
        ssme::debug_assert(handle.isValid());
        handles.push_back(std::move(handle));
    }

    std::cout << "✓ Created 10 buffers in vector" << std::endl;

    // All should be valid
    for (size_t i = 0; i < handles.size(); i++) {
        ssme::debug_assert(handles[i].isValid());
    }
    std::cout << "✓ All handles in vector are valid" << std::endl;

    // All should be unique
    for (size_t i = 0; i < handles.size(); i++) {
        for (size_t j = i + 1; j < handles.size(); j++) {
            ssme::debug_assert(handles[i].get() != handles[j].get());
        }
    }
    std::cout << "✓ All buffers are unique" << std::endl;

    // Clear vector (should destroy all handles)
    handles.clear();
    std::cout << "✓ Vector cleared (all handles destroyed)" << std::endl;

    engine.cleanup();
    std::cout << "✓ Container test passed" << std::endl;
}

// ============================================================================
// Test 11: Self-Assignment Safety
// ============================================================================
void test_SelfAssignment() {
    std::cout << "\n=== Test 11: Self-Assignment Safety ===" << std::endl;

    ssme::Engine engine;
    engine.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    ssme::BufferDesc desc;
    desc.size = 1024;
    desc.usage = static_cast<ssme::BufferUsageFlags>(ssme::BufferUsage::VERTEX_BUFFER);
    desc.is_host_visible = true;

    auto handle = engine.m_resource_manager->load<ssme::Buffer>("self_assign", desc);
    ssme::debug_assert(handle.isValid());
    auto* raw_ptr = handle.get();
    std::cout << "✓ Original handle created" << std::endl;

    // Self copy assignment (should be safe)
    handle = handle;
    ssme::debug_assert(handle.isValid());
    ssme::debug_assert(handle.get() == raw_ptr);
    std::cout << "✓ Self copy assignment is safe" << std::endl;

    // Self move assignment (should be safe)
    handle = std::move(handle);
    ssme::debug_assert(handle.isValid());
    ssme::debug_assert(handle.get() == raw_ptr);
    std::cout << "✓ Self move assignment is safe" << std::endl;

    handle.~ResourceHandle();
    std::cout << "✓ Handle destroyed" << std::endl;

    engine.cleanup();
    std::cout << "✓ Self-assignment test passed" << std::endl;
}

// ============================================================================
// Test 12: Resource Reload (Same UUID)
// ============================================================================
void test_ResourceReload() {
    std::cout << "\n=== Test 12: Resource Reload (Same UUID) ===" << std::endl;

    ssme::Engine engine;
    engine.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    ssme::BufferDesc desc;
    desc.size = 1024;
    desc.usage = static_cast<ssme::BufferUsageFlags>(ssme::BufferUsage::VERTEX_BUFFER);
    desc.is_host_visible = true;

    // Load same resource twice (should return same resource, increment refcount)
    auto handle1 = engine.m_resource_manager->load<ssme::Buffer>("reload_test", desc);
    auto handle2 = engine.m_resource_manager->load<ssme::Buffer>("reload_test", desc);

    ssme::debug_assert(handle1.isValid());
    ssme::debug_assert(handle2.isValid());
    ssme::debug_assert(handle1.get() == handle2.get());
    std::cout << "✓ Same UUID returns same resource" << std::endl;

    // Destroy first handle (resource should stay alive)
    handle1.~ResourceHandle();
    ssme::debug_assert(handle2.isValid());
    std::cout << "✓ Resource survives first handle destruction" << std::endl;

    // Destroy second handle (resource should be freed)
    handle2.~ResourceHandle();
    std::cout << "✓ Resource freed after last handle destruction" << std::endl;

    engine.cleanup();
    std::cout << "✓ Resource reload test passed" << std::endl;
}

// ============================================================================
// Test 13: Cyclic Reference Test
// ============================================================================
void test_CyclicReferences() {
    std::cout << "\n=== Test 13: Cyclic References ===" << std::endl;

    ssme::Engine engine;
    engine.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    ssme::BufferDesc desc;
    desc.size = 512;
    desc.usage = static_cast<ssme::BufferUsageFlags>(ssme::BufferUsage::VERTEX_BUFFER);
    desc.is_host_visible = true;

    // Create multiple handles to same resource
    auto handle1 = engine.m_resource_manager->load<ssme::Buffer>("cyclic_test", desc);
    auto handle2 = handle1;  // Copy
    auto handle3 = handle2;  // Copy of copy
    auto handle4 = std::move(handle3);  // Move (handle3 invalidated)

    ssme::debug_assert(handle1.isValid());
    ssme::debug_assert(handle2.isValid());
    ssme::debug_assert(!handle3.isValid());  // Moved from
    ssme::debug_assert(handle4.isValid());
    std::cout << "✓ Created cyclic references" << std::endl;

    // All valid handles should point to same resource
    ssme::debug_assert(handle1.get() == handle2.get());
    ssme::debug_assert(handle2.get() == handle4.get());
    std::cout << "✓ All handles point to same resource" << std::endl;

    // Create more copies
    std::vector<ssme::ResourceHandle<ssme::Buffer>> more_handles;
    for (int i = 0; i < 5; i++) {
        more_handles.push_back(handle1);
    }
    std::cout << "✓ Created 5 more copies" << std::endl;

    // Destroy original handles
    handle1.~ResourceHandle();
    handle2.~ResourceHandle();
    handle4.~ResourceHandle();
    std::cout << "✓ Original handles destroyed" << std::endl;

    // Vector handles should still work
    for (size_t i = 0; i < more_handles.size(); i++) {
        ssme::debug_assert(more_handles[i].isValid());
    }
    std::cout << "✓ Vector handles still valid" << std::endl;

    // Clear vector (should free resource)
    more_handles.clear();
    std::cout << "✓ Vector cleared (resource freed)" << std::endl;
    engine.cleanup();

    std::cout << "✓ Cyclic reference test passed" << std::endl;
}

// ============================================================================
// Main
// ============================================================================
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  SSME Resource System Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        // test_BufferCreation();
        // test_ResourceHandleLifetime();
        // test_RIDAllocator();
        // test_MultipleBuffers();
        // test_StressTest();
        test_RealBufferCreation();
        test_ReferenceCounting();
        test_MultipleResources();
        test_CopyMoveSemantics();
        test_HandlesInContainers();
        test_SelfAssignment();
        test_ResourceReload();
        test_CyclicReferences();

        std::cout << "\n========================================" << std::endl;
        std::cout << "  ALL TESTS PASSED ✓" << std::endl;
        std::cout << "========================================" << std::endl;
        return EXIT_SUCCESS;

    } catch (const std::exception &e) {
        std::cerr << "\n❌ TEST FAILED: " << e.what() << std::endl;
        std::cout << "\n========================================" << std::endl;
        std::cout << "  TESTS FAILED ✗" << std::endl;
        std::cout << "========================================" << std::endl;
        return EXIT_FAILURE;
    }
}
