#include "vector.h"
#include <iostream>
#include <cassert>
#include <stdexcept>
#include <string>
#include <memory>
// #include <chrono>

// ============================================================================
// STRESS TESTS
// ============================================================================

void test_large_vector() {
    std::cout << "Testing large std::vector..." << std::endl;
    
    std::vector<int> v;
    const size_t large_size = 100000;
    
    // Push many elements
    for(size_t i = 0; i < large_size; ++i) {
        v.push_back(i);
    }
    
    assert(v.size() == large_size);
    
    // Verify all elements
    for(size_t i = 0; i < large_size; ++i) {
        assert(v[i] == static_cast<int>(i));
    }
    
    std::cout << "✓ large std::vector passed" << std::endl;
}

void test_many_reallocations() {
    std::cout << "Testing many reallocations..." << std::endl;
    
    std::vector<int> v;
    
    // Force many reallocations
    for(int i = 0; i < 1000; ++i) {
        v.push_back(i);
    }
    
    assert(v.size() == 1000);
    
    // Verify data integrity after reallocations
    for(int i = 0; i < 1000; ++i) {
        assert(v[i] == i);
    }
    
    std::cout << "✓ many reallocations passed" << std::endl;
}

void test_interleaved_operations() {
    std::cout << "Testing interleaved operations..." << std::endl;
    
    std::vector<int> v;
    
    v.push_back(1);
    v.push_back(2);
    v.insert(v.begin() + 1, 10);
    assert(v[1] == 10);
    
    v.pop_back();
    v.push_back(3);
    v.erase(v.begin());
    
    assert(v.size() == 2);
    assert(v[0] == 10);
    assert(v[1] == 3);
    
    v.resize(5, 99);
    assert(v.size() == 5);
    assert(v[4] == 99);
    
    v.clear();
    assert(v.empty());
    
    std::cout << "✓ interleaved operations passed" << std::endl;
}

// ============================================================================
// EDGE CASES
// ============================================================================

void test_empty_vector_operations() {
    std::cout << "Testing empty std::vector operations..." << std::endl;
    
    std::vector<int> v;
    
    // Clear on empty
    v.clear();
    assert(v.empty());
    
    // Erase empty range
    v.push_back(1);
    auto it = v.erase(v.begin(), v.begin());
    assert(it == v.begin());
    assert(v.size() == 1);
    
    // Pop on empty (after clearing)
    v.pop_back();
    v.pop_back();  // Should handle gracefully
    assert(v.empty());
    
    std::cout << "✓ empty std::vector operations passed" << std::endl;
}

void test_single_element() {
    std::cout << "Testing single element operations..." << std::endl;
    
    std::vector<int> v;
    v.push_back(42);
    
    assert(v.front() == 42);
    assert(v.back() == 42);
    assert(v.at(0) == 42);
    
    // Erase single element
    v.erase(v.begin());
    assert(v.empty());
    
    // Insert into empty
    v.insert(v.begin(), 99);
    assert(v[0] == 99);
    
    std::cout << "✓ single element operations passed" << std::endl;
}

void test_resize_edge_cases() {
    std::cout << "Testing resize edge cases..." << std::endl;
    
    std::vector<int> v;
    
    // Resize from 0 to 0
    v.resize(0);
    assert(v.empty());
    
    // Resize to same size
    v.push_back(1);
    v.push_back(2);
    v.resize(2);
    assert(v.size() == 2);
    assert(v[0] == 1);
    assert(v[1] == 2);
    
    // Resize to 0
    v.resize(0);
    assert(v.empty());
    
    // Resize from 0 to large
    v.resize(100);
    assert(v.size() == 100);
    
    std::cout << "✓ resize edge cases passed" << std::endl;
}

void test_insert_edge_cases() {
    std::cout << "Testing insert edge cases..." << std::endl;
    
    std::vector<int> v;
    
    // Insert into empty at begin
    auto it = v.insert(v.begin(), 1);
    assert(*it == 1);
    assert(v.size() == 1);
    
    // Insert at end (like push_back)
    it = v.insert(v.end(), 2);
    assert(*it == 2);
    assert(v.size() == 2);
    assert(v[1] == 2);
    
    // Insert at beginning
    it = v.insert(v.begin(), 0);
    assert(*it == 0);
    assert(v[0] == 0);
    assert(v[1] == 1);
    assert(v[2] == 2);
    
    // Insert with reallocation
    v.clear();
    v.reserve(2);
    v.push_back(1);
    v.push_back(2);
    // This should trigger reallocation
    it = v.insert(v.begin() + 1, 99);
    assert(*it == 99);
    assert(v[1] == 99);
    
    std::cout << "✓ insert edge cases passed" << std::endl;
}

void test_erase_edge_cases() {
    std::cout << "Testing erase edge cases..." << std::endl;
    
    std::vector<int> v;
    for(int i = 0; i < 5; ++i) {
        v.push_back(i);
    }
    
    // Erase last element
    auto it = v.erase(v.begin() + 4);
    assert(it == v.end());
    assert(v.size() == 4);
    
    // Erase first element
    it = v.erase(v.begin());
    assert(*it == 1);
    assert(v[0] == 1);
    
    // Erase range to end
    it = v.erase(v.begin() + 1, v.end());
    assert(it == v.end());
    assert(v.size() == 1);
    
    // Erase empty range
    it = v.erase(v.begin(), v.begin());
    assert(v.size() == 1);
    
    std::cout << "✓ erase edge cases passed" << std::endl;
}

// ============================================================================
// CUSTOM TYPES
// ============================================================================

struct NonTrivial {
    int* data;
    static int construct_count;
    static int destruct_count;
    static int copy_count;
    static int move_count;
    
    NonTrivial() : data(new int(0)) {
        construct_count++;
    }
    
    explicit NonTrivial(int val) : data(new int(val)) {
        construct_count++;
    }
    
    ~NonTrivial() {
        delete data;
        destruct_count++;
    }
    
    NonTrivial(const NonTrivial& other) : data(new int(*other.data)) {
        construct_count++;
        copy_count++;
    }
    
    NonTrivial(NonTrivial&& other) noexcept : data(other.data) {
        other.data = nullptr;
        construct_count++;
        move_count++;
    }
    
    NonTrivial& operator=(const NonTrivial& other) {
        if(this != &other) {
            delete data;
            data = new int(*other.data);
            copy_count++;
        }
        return *this;
    }
    
    NonTrivial& operator=(NonTrivial&& other) noexcept {
        if(this != &other) {
            delete data;
            data = other.data;
            other.data = nullptr;
            move_count++;
        }
        return *this;
    }
    
    static void reset_counters() {
        construct_count = 0;
        destruct_count = 0;
        copy_count = 0;
        move_count = 0;
    }
};

int NonTrivial::construct_count = 0;
int NonTrivial::destruct_count = 0;
int NonTrivial::copy_count = 0;
int NonTrivial::move_count = 0;

void test_non_trivial_type() {
    std::cout << "Testing non-trivial type..." << std::endl;
    
    NonTrivial::reset_counters();
    std::cout << "Before test - Constructions: " << NonTrivial::construct_count 
          << ", Destructions: " << NonTrivial::destruct_count << std::endl;
    
    {
        std::vector<NonTrivial> v;
        v.push_back(NonTrivial(1));
        v.push_back(NonTrivial(2));
        v.push_back(NonTrivial(3));
        
        assert(v.size() == 3);
        assert(*v[0].data == 1);
        assert(*v[1].data == 2);
        assert(*v[2].data == 3);
        
        v.pop_back();
        assert(v.size() == 2);
    }
    
    // Check that all objects were properly destroyed
    
    
    std::cout << "  Constructions: " << NonTrivial::construct_count << std::endl;
    std::cout << "  Destructions: " << NonTrivial::destruct_count << std::endl;
    std::cout << "  Copies: " << NonTrivial::copy_count << std::endl;
    std::cout << "  Moves: " << NonTrivial::move_count << std::endl;

    assert(NonTrivial::construct_count == NonTrivial::destruct_count);
    
    std::cout << "✓ non-trivial type passed" << std::endl;
}

void test_string_vector() {
    std::cout << "Testing string std::vector..." << std::endl;
    
    std::vector<std::string> v;
    v.push_back("hello");
    v.push_back("world");
    v.push_back("test");
    
    assert(v.size() == 3);
    assert(v[0] == "hello");
    assert(v[1] == "world");
    assert(v[2] == "test");
    
    v.insert(v.begin() + 1, "beautiful");
    assert(v[1] == "beautiful");
    assert(v.size() == 4);
    
    v.erase(v.begin());
    assert(v[0] == "beautiful");
    
    v.resize(10, "default");
    assert(v.size() == 10);
    assert(v[9] == "default");
    
    std::cout << "✓ string std::vector passed" << std::endl;
}

// ============================================================================
// EXCEPTION SAFETY
// ============================================================================

struct ThrowOnCopy {
    int value;
    static int throw_after;
    static int copy_count;
    
    ThrowOnCopy(int v = 0) : value(v) {}
    
    ThrowOnCopy(const ThrowOnCopy& other) : value(other.value) {
        copy_count++;
        if(copy_count >= throw_after && throw_after > 0) {
            throw std::runtime_error("Copy failed");
        }
    }
    
    ThrowOnCopy(ThrowOnCopy&&) noexcept = default;
    ThrowOnCopy& operator=(const ThrowOnCopy&) = default;
    ThrowOnCopy& operator=(ThrowOnCopy&&) noexcept = default;
    
    static void reset() {
        copy_count = 0;
        throw_after = -1;
    }
};

int ThrowOnCopy::throw_after = -1;
int ThrowOnCopy::copy_count = 0;

void test_exception_safety_resize() {
    std::cout << "Testing exception safety in resize..." << std::endl;
    
    ThrowOnCopy::reset();
    ThrowOnCopy::throw_after = 3;
    
    std::vector<ThrowOnCopy> v;
    v.push_back(ThrowOnCopy(1));
    v.push_back(ThrowOnCopy(2));
    
    size_t old_size = v.size();
    
    try {
        v.resize(10, ThrowOnCopy(99));
        assert(false);  // Should not reach here
    } catch(const std::runtime_error&) {
        // Exception was thrown, std::vector should be unchanged
        assert(v.size() == old_size);
        assert(v[0].value == 1);
        assert(v[1].value == 2);
    }
    
    std::cout << "✓ exception safety in resize passed" << std::endl;
}

// ============================================================================
// COMPARISON WITH STD::VECTOR
// ============================================================================

// void test_comparison_with_std_vector() {
//     std::cout << "Testing comparison with std::std::vector..." << std::endl;
    
//     std::vector<int> my_vec;
//     std::std::vector<int> std_vec;
    
//     // Perform same operations
//     for(int i = 0; i < 100; ++i) {
//         my_vec.push_back(i);
//         std_vec.push_back(i);
//     }
    
//     assert(my_vec.size() == std_vec.size());
    
//     for(size_t i = 0; i < my_vec.size(); ++i) {
//         assert(my_vec[i] == std_vec[i]);
//     }
    
//     my_vec.pop_back();
//     std_vec.pop_back();
//     assert(my_vec.size() == std_vec.size());
    
//     my_vec.insert(my_vec.begin() + 10, 999);
//     std_vec.insert(std_vec.begin() + 10, 999);
//     assert(my_vec[10] == std_vec[10]);
    
//     std::cout << "✓ comparison with std::std::vector passed" << std::endl;
// }

// ============================================================================
// PERFORMANCE TESTS (Optional - just for info)
// ============================================================================

// void test_reserve_performance() {
//     std::cout << "Testing reserve performance benefit..." << std::endl;
    
//     // Without reserve
//     auto start = std::chrono::high_resolution_clock::now();
//     {
//         std::vector<int> v;
//         for(int i = 0; i < 10000; ++i) {
//             v.push_back(i);
//         }
//     }
//     auto end = std::chrono::high_resolution_clock::now();
//     auto without_reserve = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
//     // With reserve
//     start = std::chrono::high_resolution_clock::now();
//     {
//         std::vector<int> v;
//         v.reserve(10000);
//         for(int i = 0; i < 10000; ++i) {
//             v.push_back(i);
//         }
//     }
//     end = std::chrono::high_resolution_clock::now();
//     auto with_reserve = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
//     std::cout << "  Without reserve: " << without_reserve << " µs" << std::endl;
//     std::cout << "  With reserve: " << with_reserve << " µs" << std::endl;
//     std::cout << "  Speedup: " << (double)without_reserve / with_reserve << "x" << std::endl;
    
//     std::cout << "✓ reserve performance test completed" << std::endl;
// }

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Advanced Vector Unit Tests" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    try {
        // Stress tests
        test_large_vector();
        test_many_reallocations();
        test_interleaved_operations();
        
        // Edge cases
        test_empty_vector_operations();
        test_single_element();
        test_resize_edge_cases();
        test_insert_edge_cases();
        test_erase_edge_cases();
        
        // Custom types
        test_non_trivial_type();
        test_string_vector();
        
        // Exception safety
        test_exception_safety_resize();
        
        // Comparison
        // test_comparison_with_std_vector();
        
        // Performance (optional)
        // test_reserve_performance();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "✅ All advanced tests passed!" << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
        
    } catch(const std::exception& e) {
        std::cout << "\n========================================" << std::endl;
        std::cout << "❌ Test failed with exception: " << e.what() << std::endl;
        std::cout << "========================================" << std::endl;
        return 1;
    }
}