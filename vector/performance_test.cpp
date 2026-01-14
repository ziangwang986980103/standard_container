#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <iomanip>
#include "vector.h"

using namespace std::chrono;

// Helper class for timing
class Timer {
    high_resolution_clock::time_point start;
public:
    Timer() : start(high_resolution_clock::now()) {}
    
    double elapsed_ms() {
        auto end = high_resolution_clock::now();
        return duration_cast<microseconds>(end - start).count() / 1000.0;
    }
    
    void reset() {
        start = high_resolution_clock::now();
    }
};

// Test data structure
struct TestObject {
    int id;
    double value;
    std::string name;
    
    TestObject() : id(0), value(0.0), name("") {}
    TestObject(int i, double v) : id(i), value(v), name("Object_" + std::to_string(i)) {}
};

void print_header(const std::string& test_name) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << test_name << "\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << std::left << std::setw(30) << "Operation" 
              << std::setw(15) << "Custom (ms)" 
              << std::setw(15) << "std::vector (ms)"
              << "Ratio\n";
    std::cout << std::string(60, '-') << "\n";
}

void print_result(const std::string& op, double custom_time, double std_time) {
    double ratio = custom_time / std_time;
    std::cout << std::left << std::setw(30) << op
              << std::setw(15) << std::fixed << std::setprecision(2) << custom_time
              << std::setw(15) << std_time
              << std::setprecision(2) << ratio << "x\n";
}

// Test 1: Push back performance
void test_push_back_performance() {
    print_header("PUSH_BACK PERFORMANCE");
    const int N = 1000000;
    
    // Custom vector - int
    Timer t;
    {
        vector<int> v;
        for(int i = 0; i < N; ++i) {
            v.push_back(i);
        }
    }
    double custom_time = t.elapsed_ms();
    
    // std::vector - int
    t.reset();
    {
        std::vector<int> v;
        for(int i = 0; i < N; ++i) {
            v.push_back(i);
        }
    }
    double std_time = t.elapsed_ms();
    
    print_result("push_back (int, 1M elements)", custom_time, std_time);
    
    // Custom vector - TestObject
    t.reset();
    {
        vector<TestObject> v;
        for(int i = 0; i < N / 10; ++i) {
            v.push_back(TestObject(i, i * 1.5));
        }
    }
    custom_time = t.elapsed_ms();
    
    // std::vector - TestObject
    t.reset();
    {
        std::vector<TestObject> v;
        for(int i = 0; i < N / 10; ++i) {
            v.push_back(TestObject(i, i * 1.5));
        }
    }
    std_time = t.elapsed_ms();
    
    print_result("push_back (object, 100K)", custom_time, std_time);
}

// Test 2: Reserve and push back
void test_reserve_push_back() {
    print_header("RESERVE + PUSH_BACK PERFORMANCE");
    const int N = 1000000;
    
    // Custom vector
    Timer t;
    {
        vector<int> v;
        v.reserve(N);
        for(int i = 0; i < N; ++i) {
            v.push_back(i);
        }
    }
    double custom_time = t.elapsed_ms();
    
    // std::vector
    t.reset();
    {
        std::vector<int> v;
        v.reserve(N);
        for(int i = 0; i < N; ++i) {
            v.push_back(i);
        }
    }
    double std_time = t.elapsed_ms();
    
    print_result("reserve + push_back (1M)", custom_time, std_time);
}

// Test 3: Insert at beginning
void test_insert_beginning() {
    print_header("INSERT AT BEGINNING PERFORMANCE");
    const int N = 10000;
    
    // Custom vector
    Timer t;
    {
        vector<int> v;
        for(int i = 0; i < N; ++i) {
            v.insert(v.begin(), i);
        }
    }
    double custom_time = t.elapsed_ms();
    
    // std::vector
    t.reset();
    {
        std::vector<int> v;
        for(int i = 0; i < N; ++i) {
            v.insert(v.begin(), i);
        }
    }
    double std_time = t.elapsed_ms();
    
    print_result("insert at begin (10K)", custom_time, std_time);
}

// Test 4: Insert in middle
void test_insert_middle() {
    print_header("INSERT IN MIDDLE PERFORMANCE");
    const int N = 10000;
    
    // Custom vector
    Timer t;
    {
        vector<int> v;
        v.reserve(N);
        for(int i = 0; i < N; ++i) {
            v.push_back(i);
        }
        for(int i = 0; i < 1000; ++i) {
            v.insert(v.begin() + v.size() / 2, i);
        }
    }
    double custom_time = t.elapsed_ms();
    
    // std::vector
    t.reset();
    {
        std::vector<int> v;
        v.reserve(N);
        for(int i = 0; i < N; ++i) {
            v.push_back(i);
        }
        for(int i = 0; i < 1000; ++i) {
            v.insert(v.begin() + v.size() / 2, i);
        }
    }
    double std_time = t.elapsed_ms();
    
    print_result("insert middle (1K into 10K)", custom_time, std_time);
}

// Test 5: Random access
void test_random_access() {
    print_header("RANDOM ACCESS PERFORMANCE");
    const int N = 1000000;
    const int ACCESSES = 10000000;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, N - 1);
    
    // Prepare random indices
    std::vector<int> indices;
    indices.reserve(ACCESSES);
    for(int i = 0; i < ACCESSES; ++i) {
        indices.push_back(dis(gen));
    }
    
    // Custom vector
    vector<int> v_custom;
    v_custom.reserve(N);
    for(int i = 0; i < N; ++i) {
        v_custom.push_back(i);
    }
    
    Timer t;
    long long sum = 0;
    for(int idx : indices) {
        sum += v_custom[idx];
    }
    double custom_time = t.elapsed_ms();
    
    // std::vector
    std::vector<int> v_std;
    v_std.reserve(N);
    for(int i = 0; i < N; ++i) {
        v_std.push_back(i);
    }
    
    t.reset();
    sum = 0;
    for(int idx : indices) {
        sum += v_std[idx];
    }
    double std_time = t.elapsed_ms();
    
    print_result("random access (10M ops)", custom_time, std_time);
}

// Test 6: Erase from end
void test_erase_end() {
    print_header("ERASE FROM END PERFORMANCE");
    const int N = 100000;
    
    // Custom vector
    vector<int> v_custom;
    for(int i = 0; i < N; ++i) {
        v_custom.push_back(i);
    }
    
    Timer t;
    while(!v_custom.empty()) {
        v_custom.pop_back();
    }
    double custom_time = t.elapsed_ms();
    
    // std::vector
    std::vector<int> v_std;
    for(int i = 0; i < N; ++i) {
        v_std.push_back(i);
    }
    
    t.reset();
    while(!v_std.empty()) {
        v_std.pop_back();
    }
    double std_time = t.elapsed_ms();
    
    print_result("pop_back (100K)", custom_time, std_time);
}

// Test 7: Copy constructor
void test_copy_constructor() {
    print_header("COPY CONSTRUCTOR PERFORMANCE");
    const int N = 1000000;
    
    // Custom vector
    vector<int> v_custom_orig;
    v_custom_orig.reserve(N);
    for(int i = 0; i < N; ++i) {
        v_custom_orig.push_back(i);
    }
    
    Timer t;
    vector<int> v_custom_copy = v_custom_orig;
    double custom_time = t.elapsed_ms();
    
    // std::vector
    std::vector<int> v_std_orig;
    v_std_orig.reserve(N);
    for(int i = 0; i < N; ++i) {
        v_std_orig.push_back(i);
    }
    
    t.reset();
    std::vector<int> v_std_copy = v_std_orig;
    double std_time = t.elapsed_ms();
    
    print_result("copy (1M elements)", custom_time, std_time);
}

// Test 8: Move constructor
void test_move_constructor() {
    print_header("MOVE CONSTRUCTOR PERFORMANCE");
    const int N = 1000000;
    
    // Custom vector
    vector<int> v_custom_orig;
    v_custom_orig.reserve(N);
    for(int i = 0; i < N; ++i) {
        v_custom_orig.push_back(i);
    }
    
    Timer t;
    vector<int> v_custom_moved = std::move(v_custom_orig);
    double custom_time = t.elapsed_ms();
    
    // std::vector
    std::vector<int> v_std_orig;
    v_std_orig.reserve(N);
    for(int i = 0; i < N; ++i) {
        v_std_orig.push_back(i);
    }
    
    t.reset();
    std::vector<int> v_std_moved = std::move(v_std_orig);
    double std_time = t.elapsed_ms();
    
    print_result("move (1M elements)", custom_time, std_time);
}

// Test 9: Iteration
void test_iteration() {
    print_header("ITERATION PERFORMANCE");
    const int N = 10000000;
    
    // Custom vector
    vector<int> v_custom;
    v_custom.reserve(N);
    for(int i = 0; i < N; ++i) {
        v_custom.push_back(i);
    }
    
    Timer t;
    long long sum = 0;
    for(const auto& val : v_custom) {
        sum += val;
    }
    double custom_time = t.elapsed_ms();
    
    // std::vector
    std::vector<int> v_std;
    v_std.reserve(N);
    for(int i = 0; i < N; ++i) {
        v_std.push_back(i);
    }
    
    t.reset();
    sum = 0;
    for(const auto& val : v_std) {
        sum += val;
    }
    double std_time = t.elapsed_ms();
    
    print_result("range-for iteration (10M)", custom_time, std_time);
}

// Test 10: Emplace back
void test_emplace_back() {
    print_header("EMPLACE_BACK PERFORMANCE");
    const int N = 100000;
    
    // Custom vector
    Timer t;
    {
        vector<TestObject> v;
        for(int i = 0; i < N; ++i) {
            v.emplace_back(i, i * 1.5);
        }
    }
    double custom_time = t.elapsed_ms();
    
    // std::vector
    t.reset();
    {
        std::vector<TestObject> v;
        for(int i = 0; i < N; ++i) {
            v.emplace_back(i, i * 1.5);
        }
    }
    double std_time = t.elapsed_ms();
    
    print_result("emplace_back (100K objects)", custom_time, std_time);
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║     VECTOR PERFORMANCE COMPARISON: Custom vs std::vector   ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    
    test_push_back_performance();
    test_reserve_push_back();
    test_insert_beginning();
    test_insert_middle();
    test_random_access();
    test_erase_end();
    test_copy_constructor();
    test_move_constructor();
    test_iteration();
    test_emplace_back();
    
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Performance testing complete!\n";
    std::cout << "Note: Ratio < 1.0 means custom vector is faster\n";
    std::cout << "      Ratio > 1.0 means std::vector is faster\n";
    std::cout << std::string(60, '=') << "\n\n";
    
    return 0;
}