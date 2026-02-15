# iOS Integration Guide

## Overview

This guide explains how to integrate the BenchmarkCore library into an iOS application for testing purposes.

## What This Demo Does

- **Memory Benchmarking**: Measures RAM read/write/verify performance
- **CPU Benchmarking**: Tests computational performance (integer, float, memory operations)
- **Timing Measurements**: High-resolution nanosecond-precision timing
- **Stability Testing**: Continuous mode for consistency analysis

## What This Demo Does NOT Do

❌ **Does NOT modify CPU frequency or hardware settings**
❌ **Does NOT increase physical RAM**
❌ **Does NOT access hardware pins or chips directly**
❌ **Does NOT modify bootloader or kernel**
❌ **Does NOT use private iOS APIs**
❌ **Does NOT require jailbreaking**
❌ **Does NOT guarantee performance improvements**
❌ **Does NOT perform background execution**

All measurements are performed in user space within iOS sandbox constraints.

## iOS Compatibility

### Supported Features
- ✅ Memory benchmarking (user-space only)
- ✅ CPU computational benchmarking
- ✅ High-resolution timing (std::chrono)
- ✅ Demo mode with safe defaults

### Not Supported on iOS
- ❌ Network benchmarking (requires raw sockets or special entitlements)
- ❌ Process priority adjustment (requires elevated privileges)
- ❌ Continuous background execution (iOS sandbox restrictions)

## Integration Steps

### Step 1: Add Core Library to Xcode Project

1. Open your Xcode project
2. Right-click on your project in the navigator
3. Select "Add Files to [Project]..."
4. Navigate to the `core/` directory
5. Select the following files:
   - `core/include/timer.h`
   - `core/include/memory_benchmark.h`
   - `core/include/cpu_benchmark.h`
   - `core/include/demo_config.h`
   - `core/src/timer.cpp`
   - `core/src/memory_benchmark.cpp`
   - `core/src/cpu_benchmark.cpp`
6. Ensure "Copy items if needed" is checked
7. Click "Add"

### Step 2: Configure Build Settings

1. Select your project in the navigator
2. Select your target
3. Go to "Build Settings"
4. Set "C++ Language Dialect" to "C++17"
5. Set "C++ Standard Library" to "libc++"

### Step 3: Create iOS Wrapper (Swift Example)

```swift
import Foundation

class BenchmarkRunner {
    // Run memory benchmark with demo defaults
    func runMemoryBenchmark() -> String {
        // Call C++ code through bridging header
        let benchmark = MemoryBenchmark()
        let results = benchmark.run(
            DemoConfig.DEMO_BUFFER_SIZE,
            DemoConfig.DEMO_ITERATIONS
        )
        
        return formatResults(results)
    }
    
    // Run CPU benchmark with demo defaults
    func runCPUBenchmark() -> String {
        let benchmark = CpuBenchmark()
        let results = benchmark.run(DemoConfig.DEMO_CPU_ITERATIONS)
        
        return formatResults(results)
    }
    
    private func formatResults(_ results: Any) -> String {
        // Format results for UI display
        // Convert C++ results to Swift string
        return "Benchmark completed"
    }
}
```

### Step 4: Create Bridging Header

Create a file named `YourProject-Bridging-Header.h`:

```objc
#ifndef YourProject_Bridging_Header_h
#define YourProject_Bridging_Header_h

#ifdef __cplusplus
extern "C" {
#endif

// Include C++ headers
#include "timer.h"
#include "memory_benchmark.h"
#include "cpu_benchmark.h"
#include "demo_config.h"

#ifdef __cplusplus
}
#endif

#endif
```

### Step 5: Create Objective-C++ Wrapper (Recommended)

Create `BenchmarkWrapper.mm`:

```objc
#import <Foundation/Foundation.h>
#include "memory_benchmark.h"
#include "cpu_benchmark.h"
#include "demo_config.h"

@interface BenchmarkWrapper : NSObject
+ (NSString *)runMemoryBenchmark;
+ (NSString *)runCPUBenchmark;
@end

@implementation BenchmarkWrapper

+ (NSString *)runMemoryBenchmark {
    MemoryBenchmark benchmark;
    auto results = benchmark.run(
        DemoConfig::DEMO_BUFFER_SIZE,
        DemoConfig::DEMO_ITERATIONS
    );
    
    NSString *output = [NSString stringWithFormat:
        @"Memory Benchmark Results:\n"
        @"Buffer Size: %zu bytes\n"
        @"Iterations: %zu\n"
        @"Avg Latency: %.2f ns\n"
        @"Throughput: %.2f MB/s\n"
        @"Verification: %@",
        results.buffer_size_bytes,
        results.iterations,
        results.timing.avg_latency_ns,
        results.throughput_mbps,
        results.verification_passed ? @"PASSED" : @"FAILED"
    ];
    
    return output;
}

+ (NSString *)runCPUBenchmark {
    CpuBenchmark benchmark;
    auto results = benchmark.run(DemoConfig::DEMO_CPU_ITERATIONS);
    
    NSString *output = [NSString stringWithFormat:
        @"CPU Benchmark Results:\n"
        @"Iterations: %zu\n"
        @"Time per Op: %.2f ns\n"
        @"Ops/Second: %.2f",
        results.iterations,
        results.timing.time_per_operation_ns,
        results.timing.operations_per_second
    ];
    
    return output;
}

@end
```

### Step 6: Use in SwiftUI View

```swift
import SwiftUI

struct BenchmarkView: View {
    @State private var results: String = ""
    @State private var isRunning: Bool = false
    
    var body: some View {
        VStack(spacing: 20) {
            Text("Performance Benchmark")
                .font(.title)
            
            Button("Run Memory Benchmark") {
                runBenchmark(type: .memory)
            }
            .disabled(isRunning)
            
            Button("Run CPU Benchmark") {
                runBenchmark(type: .cpu)
            }
            .disabled(isRunning)
            
            ScrollView {
                Text(results)
                    .font(.system(.body, design: .monospaced))
                    .padding()
            }
        }
        .padding()
    }
    
    enum BenchmarkType {
        case memory, cpu
    }
    
    func runBenchmark(type: BenchmarkType) {
        isRunning = true
        results = "Running benchmark..."
        
        DispatchQueue.global(qos: .userInitiated).async {
            let result: String
            switch type {
            case .memory:
                result = BenchmarkWrapper.runMemoryBenchmark()
            case .cpu:
                result = BenchmarkWrapper.runCPUBenchmark()
            }
            
            DispatchQueue.main.async {
                self.results = result
                self.isRunning = false
            }
        }
    }
}
```

## Demo Mode Usage

The `demo_config.h` header provides safe defaults for mobile devices:

```cpp
#include "demo_config.h"
#include "memory_benchmark.h"

// Use demo defaults
MemoryBenchmark benchmark;
auto results = benchmark.run(
    DemoConfig::DEMO_BUFFER_SIZE,    // 1 MB
    DemoConfig::DEMO_ITERATIONS       // 100 iterations
);
```

## Important Notes

### Performance Considerations
- Run benchmarks on a background thread to avoid blocking UI
- Use demo mode defaults to avoid excessive battery drain
- Benchmarks may take several seconds to complete
- Results will vary based on device model and iOS version

### iOS Sandbox Restrictions
- Cannot modify system settings
- Cannot access hardware directly
- Cannot run in background indefinitely
- Cannot use elevated privileges

### Testing Recommendations
- Test on physical devices (not just simulator)
- Test with device plugged in (battery considerations)
- Close other apps before benchmarking
- Run multiple times for consistency

## Troubleshooting

### Build Errors
- Ensure C++17 is enabled in build settings
- Check that all core files are added to target
- Verify bridging header path is correct

### Runtime Issues
- Reduce iteration counts if app becomes unresponsive
- Use demo mode defaults for initial testing
- Check memory warnings in Xcode console

## Example Output

```
Memory Benchmark Results:
Buffer Size: 1048576 bytes (1.00 MB)
Iterations: 100
Avg Latency: 1234.56 ns
Min Latency: 1100.23 ns
Max Latency: 1456.78 ns
Throughput: 2345.67 MB/s
Verification: PASSED
```

## Legal Disclaimer

This benchmark tool is for measurement and analysis purposes only. It does not:
- Modify hardware or system settings
- Bypass iOS security restrictions
- Require jailbreaking or special entitlements
- Guarantee any performance improvements

All measurements are performed within standard iOS sandbox constraints.
