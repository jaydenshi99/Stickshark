#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cassert>

struct TestResult {
    std::string name;
    bool passed;
    std::string error;
};

class TestRunner {
private:
    std::vector<TestResult> results;
    int passed = 0;
    int failed = 0;

public:
    void run(const std::string& testName, bool (*testFunc)()) {
        std::cout << "Running: " << testName << std::endl;
        bool result = false;
        std::string error;
        
        try {
            result = testFunc();
        } catch (const std::exception& e) {
            error = e.what();
            result = false;
        } catch (...) {
            error = "Unknown exception";
            result = false;
        }
        
        if (result) {
            std::cout << "  PASSED" << std::endl;
            passed++;
        } else {
            std::cout << "  FAILED";
            if (!error.empty()) {
                std::cout << " (" << error << ")";
            }
            std::cout << std::endl;
            failed++;
        }
        
        results.push_back({testName, result, error});
    }
    
    void assert_true(bool condition, const std::string& message = "") {
        if (!condition) {
            throw std::runtime_error(message.empty() ? "Assertion failed" : message);
        }
    }
    
    void assert_false(bool condition, const std::string& message = "") {
        assert_true(!condition, message);
    }
    
    template<typename T>
    void assert_equal(const T& expected, const T& actual, const std::string& message = "") {
        if (expected != actual) {
            throw std::runtime_error(
                message.empty() 
                    ? "Expected != Actual" 
                    : message + " (Expected: " + std::to_string(expected) + ", Actual: " + std::to_string(actual) + ")"
            );
        }
    }
    
    void printSummary() {
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        std::cout << "Total:  " << (passed + failed) << std::endl;
        
        if (failed > 0) {
            std::cout << "\nFailed tests:" << std::endl;
            for (const auto& result : results) {
                if (!result.passed) {
                    std::cout << "  - " << result.name;
                    if (!result.error.empty()) {
                        std::cout << ": " << result.error;
                    }
                    std::cout << std::endl;
                }
            }
        }
    }
    
    int getExitCode() const {
        return failed > 0 ? 1 : 0;
    }
};

// Helper macros for easier test writing
#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            throw std::runtime_error("Assertion failed: " #condition); \
        } \
    } while(0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            throw std::runtime_error("Assertion failed: " #expected " == " #actual); \
        } \
    } while(0)

