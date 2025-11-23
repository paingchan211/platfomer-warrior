#pragma once

#include "Constants.h"

#include <iostream>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <mutex>

#if defined(__GNUG__)
#include <cxxabi.h>
#include <memory>
#include <cstdlib>
#endif

// Converts a mangled C++ type name into a human-readable form (GCC/Clang only)
inline std::string demangleTypeName(const char *name)
{
#if defined(__GNUG__)
    int status = 0;
    // Use RAII smart pointer to free memory returned by __cxa_demangle
    std::unique_ptr<char, decltype(&std::free)> demangled(
        abi::__cxa_demangle(name, nullptr, nullptr, &status),
        &std::free);
    if (status == 0 && demangled)
    {
        return demangled.get(); // Return demangled readable name
    }
#endif
    return name ? name : "UnknownType"; // Fallback if demangling not supported
}

// Logs how virtual dispatch resolves at runtime (e.g., Base* calling Derived::update)
// BasePtr = static pointer type, baseLabel = "Entity", methodLabel = "update"
template <typename BasePtr>
inline void logPolymorphicDispatch(const BasePtr *instance,
                                   const char *baseLabel,
                                   const char *methodLabel)
{
    // Skip logging if disabled or null instance
    if (!ENABLE_POLYMORPHISM_STDOUT || instance == nullptr)
    {
        return;
    }

    // Extract actual dynamic type name of the object
    const std::string derivedName = demangleTypeName(typeid(*instance).name());

    // Use provided labels or defaults
    const char *base = baseLabel ? baseLabel : "UnknownBase";
    const char *method = methodLabel ? methodLabel : "";

    // Create a unique key for tracking count (Base::Derived::method)
    const std::string key = std::string(base) + "::" + derivedName + "::" + method;

    // Static tracker shared across all calls
    struct DispatchTracker
    {
        std::unordered_map<std::string, int> counts; // Tracks prints per call site
        std::mutex mutex;                            // Protects multithreaded access
    };

    static DispatchTracker tracker;

    {
        std::lock_guard<std::mutex> lock(tracker.mutex); // Thread-safe increment
        int &count = tracker.counts[key];
        if (count >= POLYMORPHISM_LOG_LIMIT)
        {
            return; // Stop logging after limit reached
        }
        ++count; // Count this dispatch
    }

    // Print polymorphism dispatch result to stdout
    std::cout << "[Polymorphism] " << base << " pointer dispatched to "
              << derivedName << "::" << method << std::endl;
}
