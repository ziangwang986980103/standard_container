#include "vector.h"
#include <iostream>
#include <cassert>
#include <stdexcept>

void test_constructor() {
    std::cout << "Testing constructors..." << std::endl;
    
    // Default constructor
    std::vector<int> v1;
    assert(v1.size() == 0);
    assert(v1.capacity() == 0);
    assert(v1.empty());
    
    // Constructor with count
    std::vector<int> v2(5);
    assert(v2.size() == 5);
    assert(v2.capacity() >= 5);
    
    // Constructor with count and value
    std::vector<int> v3(5, 42);
    assert(v3.size() == 5);
    for(size_t i = 0; i < v3.size(); ++i) {
        assert(v3[i] == 42);
    }
    
    std::cout << "✓ Constructors passed" << std::endl;
}

void test_push_back() {
    std::cout << "Testing push_back..." << std::endl;
    
    std::vector<int> v;
    v.push_back(1);
    assert(v.size() == 1);
    assert(v[0] == 1);
    
    v.push_back(2);
    v.push_back(3);
    assert(v.size() == 3);
    assert(v[0] == 1);
    assert(v[1] == 2);
    assert(v[2] == 3);
    
    // Test with rvalue
    int x = 99;
    v.push_back(std::move(x));
    assert(v[3] == 99);
    
    std::cout << "✓ push_back passed" << std::endl;
}

void test_pop_back() {
    std::cout << "Testing pop_back..." << std::endl;
    
    std::vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    
    v.pop_back();
    assert(v.size() == 2);
    assert(v[0] == 1);
    assert(v[1] == 2);
    
    v.pop_back();
    v.pop_back();
    assert(v.empty());
    
    std::cout << "✓ pop_back passed" << std::endl;
}

void test_insert() {
    std::cout << "Testing insert..." << std::endl;
    
    std::vector<int> v;
    v.push_back(1);
    v.push_back(3);
    
    // Insert in middle
    auto it = v.insert(v.begin() + 1, 2);
    assert(v.size() == 3);
    assert(v[0] == 1);
    assert(v[1] == 2);
    assert(v[2] == 3);
    assert(*it == 2);
    
    // Insert at beginning
    v.insert(v.begin(), 0);
    assert(v[0] == 0);
    assert(v.size() == 4);
    
    // Insert at end
    v.insert(v.end(), 4);
    assert(v[4] == 4);
    assert(v.size() == 5);
    
    std::cout << "✓ insert passed" << std::endl;
}

void test_erase() {
    std::cout << "Testing erase..." << std::endl;
    
    std::vector<int> v;
    for(int i = 0; i < 5; ++i) {
        v.push_back(i);
    }
    
    // Erase single element
    auto it = v.erase(v.begin() + 2);
    assert(v.size() == 4);
    assert(v[0] == 0);
    assert(v[1] == 1);
    assert(v[2] == 3);
    assert(v[3] == 4);
    assert(*it == 3);
    
    // Erase range
    v.erase(v.begin() + 1, v.begin() + 3);
    assert(v.size() == 2);
    assert(v[0] == 0);
    assert(v[1] == 4);
    
    std::cout << "✓ erase passed" << std::endl;
}

void test_resize() {
    std::cout << "Testing resize..." << std::endl;
    
    std::vector<int> v;
    v.push_back(1);
    v.push_back(2);
    
    // Resize larger
    v.resize(5);
    assert(v.size() == 5);
    assert(v[0] == 1);
    assert(v[1] == 2);
    assert(v[2] == 0);  // Default initialized
    
    // Resize larger with value
    v.resize(7, 99);
    assert(v.size() == 7);
    assert(v[5] == 99);
    assert(v[6] == 99);
    
    // Resize smaller
    v.resize(3);
    assert(v.size() == 3);
    assert(v[0] == 1);
    assert(v[1] == 2);
    
    std::cout << "✓ resize passed" << std::endl;
}

void test_at() {
    std::cout << "Testing at..." << std::endl;
    
    std::vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    
    assert(v.at(0) == 1);
    assert(v.at(2) == 3);
    
    // Test bounds checking
    bool threw = false;
    try {
        v.at(10);
    } catch(const std::out_of_range&) {
        threw = true;
    }
    assert(threw);
    
    std::cout << "✓ at passed" << std::endl;
}

void test_clear() {
    std::cout << "Testing clear..." << std::endl;
    
    std::vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    
    size_t old_cap = v.capacity();
    v.clear();
    
    assert(v.size() == 0);
    assert(v.empty());
    assert(v.capacity() == old_cap);  // Capacity unchanged
    
    std::cout << "✓ clear passed" << std::endl;
}

void test_reserve() {
    std::cout << "Testing reserve..." << std::endl;
    
    std::vector<int> v;
    v.reserve(100);
    
    assert(v.capacity() >= 100);
    assert(v.size() == 0);
    
    // Add elements - should not reallocate
    for(int i = 0; i < 50; ++i) {
        v.push_back(i);
    }
    assert(v.capacity() >= 100);
    
    std::cout << "✓ reserve passed" << std::endl;
}

void test_emplace_back() {
    std::cout << "Testing emplace_back..." << std::endl;
    
    std::vector<std::pair<int, int>> v;
    v.emplace_back(1, 2);
    
    assert(v.size() == 1);
    assert(v[0].first == 1);
    assert(v[0].second == 2);
    
    v.emplace_back(3, 4);
    assert(v.size() == 2);
    
    std::cout << "✓ emplace_back passed" << std::endl;
}

void test_copy_operations() {
    std::cout << "Testing copy operations..." << std::endl;
    
    std::vector<int> v1;
    v1.push_back(1);
    v1.push_back(2);
    v1.push_back(3);
    
    // Copy constructor
    std::vector<int> v2(v1);
    assert(v2.size() == 3);
    assert(v2[0] == 1);
    assert(v2[1] == 2);
    assert(v2[2] == 3);
    
    // Copy assignment
    std::vector<int> v3;
    v3 = v1;
    assert(v3.size() == 3);
    assert(v3[0] == 1);
    
    std::cout << "✓ copy operations passed" << std::endl;
}

void test_move_operations() {
    std::cout << "Testing move operations..." << std::endl;
    
    std::vector<int> v1;
    v1.push_back(1);
    v1.push_back(2);
    v1.push_back(3);
    
    // Move constructor
    std::vector<int> v2(std::move(v1));
    assert(v2.size() == 3);
    assert(v2[0] == 1);
    // v1 is now in moved-from state
    
    // Move assignment
    std::vector<int> v3;
    v3 = std::move(v2);
    assert(v3.size() == 3);
    assert(v3[0] == 1);
    
    std::cout << "✓ move operations passed" << std::endl;
}

void test_iterators() {
    std::cout << "Testing iterators..." << std::endl;
    
    std::vector<int> v;
    for(int i = 0; i < 5; ++i) {
        v.push_back(i);
    }
    
    // Forward iteration
    int expected = 0;
    for(auto it = v.begin(); it != v.end(); ++it) {
        assert(*it == expected++);
    }
    
    // Range-based for
    expected = 0;
    for(auto& val : v) {
        assert(val == expected++);
    }
    
    std::cout << "✓ iterators passed" << std::endl;
}

void test_front_back() {
    std::cout << "Testing front/back..." << std::endl;
    
    std::vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    
    assert(v.front() == 1);
    assert(v.back() == 3);
    
    v.front() = 10;
    v.back() = 30;
    
    assert(v[0] == 10);
    assert(v[2] == 30);
    
    std::cout << "✓ front/back passed" << std::endl;
}

int main() {
    std::cout << "Running std::vector unit tests...\n" << std::endl;
    
    try {
        test_constructor();
        test_push_back();
        test_pop_back();
        test_insert();
        test_erase();
        test_resize();
        test_at();
        test_clear();
        test_reserve();
        test_emplace_back();
        test_copy_operations();
        test_move_operations();
        test_iterators();
        test_front_back();
        
        std::cout << "\n✅ All tests passed!" << std::endl;
        return 0;
    } catch(const std::exception& e) {
        std::cout << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}