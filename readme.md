Project Title

System Memory and Network Performance Measurement Engine (C/C++)

Project Goal

Build a C/C++ system-level measurement tool that benchmarks RAM read/write behavior and compares device-side processing time with real network latency. The tool is designed as a demo and analysis engine, not a hardware modification system.

Target Environment

OS: Unix-like (Linux / Android preferred)

Language: Standard C / C++ (portable, warning-clean)

Execution: User-space application

Build: Makefile or CMake

No kernel or bootloader modification

Core Features (Phase 1)
1. Memory Read/Write/Compare Engine

Allocate a configurable RAM buffer

Perform repeated read–write–read cycles (e.g. up to 10M iterations)

Validate data correctness byte-by-byte

Measure:

Total execution time

Average latency per iteration

Throughput (MB/s)

Store results in internal tables (arrays or structs)

2. Timing & Table Generation

Use high-resolution timers (monotonic clock)

Record:

Buffer size

Iteration count

Min / max / average timings

Output results in structured form (table or CSV-style text)

3. Resource Prioritization (Best Effort)

Increase process priority where permitted

Reduce scheduling noise (single-threaded baseline)

No claims of exclusive hardware control

Log environment constraints clearly

4. Network Loop & Call Timing Measurement

Implement a simple network loop:

Open connection

Send small payload

Receive response

Close connection

Measure:

Call open time (target ~0.2s observation)

Round-trip latency

Compare network delay vs CPU memory timing

Support WAN / Wi-Fi where available via standard APIs

5. Demo Output

Console-based output is sufficient

Show:

Memory benchmark results

Network timing results

Ratio of CPU vs network latency

Output must be readable and repeatable

Non-Goals (Explicit)

No CPU frequency changes

No physical RAM expansion

No pin-level hardware access

No bootloader or kernel modification

No compression-based memory tricks

Milestones (for AI Agent Execution)
Milestone 1 – Project Setup

Tasks

Create project structure

Add build system

Implement main entry point

Implement timer utility

Output

Compiling empty tool

Clean warnings

Milestone 2 – Memory Benchmark

Tasks

Allocate RAM buffer

Implement read/write/read loop

Add timing

Validate data integrity

Output

Timing results printed

Stable, repeatable measurements

Milestone 3 – Tables & Optimization

Tasks

Store results in structs

Add averaging and summary stats

Apply best-effort process prioritization

Output

Structured output tables

Cleaner timing results

Milestone 4 – Network Measurement

Tasks

Implement connection open/close loop

Measure latency and throughput

Compare with memory timings

Output

CPU vs network timing comparison

Milestone 5 – Final Demo & Cleanup

Tasks

Clean code

Add usage instructions

Final test run

Output

Demo-ready binary

Simple documentation

Acceptance Criteria

Compiles cleanly with strict warnings

Runs without crashes or leaks

Produces consistent timing data

Results are explainable and reproducible

Scope matches user-space OS limits

Notes for Code AI Agent

Prefer clarity over cleverness

Avoid undefined behavior

Log assumptions clearly

Keep math simple and verifiable