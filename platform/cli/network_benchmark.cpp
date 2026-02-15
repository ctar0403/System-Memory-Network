/**
 * network_benchmark.cpp - Network benchmark implementation
 */

#include "network_benchmark.h"
#include "timer.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>
#include <cerrno>
#include <cstddef>
#include <limits>
#include <algorithm>

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#endif

NetworkBenchmark::NetworkBenchmark() noexcept {
}

NetworkBenchmark::Results NetworkBenchmark::run(
    const std::string& host,
    std::uint16_t port,
    std::size_t payload_size_bytes
) {
    Results results{};
    results.target_host = host;
    results.target_port = port;
    results.payload_size_bytes = payload_size_bytes;
    results.iterations = 1;
    results.benchmark_successful = false;
    results.timing.connection_successful = false;
    results.timing.data_exchange_successful = false;
    results.timing.connection_time_ms = 0.0;
    results.timing.send_time_ms = 0.0;
    results.timing.receive_time_ms = 0.0;
    results.timing.round_trip_time_ms = 0.0;
    results.timing.avg_connection_time_ms = 0.0;
    results.timing.min_connection_time_ms = 0.0;
    results.timing.max_connection_time_ms = 0.0;

#ifdef __linux__
    // Attempt to connect
    double connection_time_ms = 0.0;
    int socket_fd = connect_to_host(host, port, connection_time_ms);
    
    if (socket_fd < 0) {
        results.error_message = "Failed to establish connection";
        results.timing.connection_successful = false;
        return results;
    }
    
    results.timing.connection_time_ms = connection_time_ms;
    results.timing.avg_connection_time_ms = connection_time_ms;
    results.timing.min_connection_time_ms = connection_time_ms;
    results.timing.max_connection_time_ms = connection_time_ms;
    results.timing.connection_successful = true;

    // Prepare payload data
    std::vector<std::uint8_t> send_buffer(payload_size_bytes);
    std::vector<std::uint8_t> recv_buffer(payload_size_bytes);
    
    // Fill send buffer with a simple pattern
    for (std::size_t i = 0; i < payload_size_bytes; ++i) {
        send_buffer[i] = static_cast<std::uint8_t>(i & 0xFF);
    }

    // Measure round-trip time
    Timer round_trip_timer;
    round_trip_timer.start();

    // Send data
    double send_time_ms = 0.0;
    ssize_t bytes_sent = send_data(socket_fd, send_buffer.data(), 
                                    payload_size_bytes, send_time_ms);
    
    if (bytes_sent < 0 || static_cast<std::size_t>(bytes_sent) != payload_size_bytes) {
        results.error_message = "Failed to send data completely";
        close_socket(socket_fd);
        return results;
    }
    
    results.timing.send_time_ms = send_time_ms;

    // Receive data (if server echoes back)
    // Note: Most servers won't echo data, so this may fail - that's OK
    double receive_time_ms = 0.0;
    ssize_t bytes_received = receive_data(socket_fd, recv_buffer.data(),
                                          payload_size_bytes, receive_time_ms);
    
    double round_trip_time_ms = round_trip_timer.elapsed_milliseconds();
    
    results.timing.receive_time_ms = receive_time_ms;
    results.timing.round_trip_time_ms = round_trip_time_ms;
    
    // Consider benchmark successful if we can connect and send
    // Receiving data back is optional (depends on server behavior)
    if (bytes_received >= 0) {
        results.timing.data_exchange_successful = true;
        results.benchmark_successful = true;
    } else {
        // Send was successful, but receive failed (server may not echo)
        // Still consider this a partial success for timing measurements
        results.timing.data_exchange_successful = false;
        results.benchmark_successful = true;  // We got connection and send timing
        results.error_message = "Server did not echo data (this is normal for most servers)";
    }

    close_socket(socket_fd);
#else
    results.error_message = "Network benchmarking not supported on this platform";
    results.timing.connection_successful = false;
#endif

    return results;
}

NetworkBenchmark::Results NetworkBenchmark::run_call_loop(
    const std::string& host,
    std::uint16_t port,
    std::size_t iterations,
    std::size_t payload_size_bytes
) {
    Results results{};
    results.target_host = host;
    results.target_port = port;
    results.payload_size_bytes = payload_size_bytes;
    results.iterations = iterations;
    results.benchmark_successful = false;
    results.timing.connection_successful = false;
    results.timing.data_exchange_successful = false;
    results.timing.connection_time_ms = 0.0;
    results.timing.send_time_ms = 0.0;
    results.timing.receive_time_ms = 0.0;
    results.timing.round_trip_time_ms = 0.0;
    results.timing.avg_connection_time_ms = 0.0;
    results.timing.min_connection_time_ms = 0.0;
    results.timing.max_connection_time_ms = 0.0;

#ifdef __linux__
    std::vector<double> connection_times;
    connection_times.reserve(iterations);
    
    std::size_t successful_connections = 0;
    double total_connection_time = 0.0;
    double min_time = std::numeric_limits<double>::max();
    double max_time = 0.0;
    
    std::cout << "Performing " << iterations << " connection cycles...\n";
    
    for (std::size_t i = 0; i < iterations; ++i) {
        double cycle_time = 0.0;
        bool success = single_connection_cycle(host, port, payload_size_bytes, cycle_time);
        
        if (success) {
            successful_connections++;
            connection_times.push_back(cycle_time);
            total_connection_time += cycle_time;
            min_time = std::min(min_time, cycle_time);
            max_time = std::max(max_time, cycle_time);
        }
        
        // Progress indicator for long runs
        if (iterations > 10 && (i + 1) % (iterations / 10) == 0) {
            std::cout << "  Progress: " << ((i + 1) * 100 / iterations) << "%\n";
        }
    }
    
    if (successful_connections > 0) {
        results.timing.connection_successful = true;
        results.timing.data_exchange_successful = true;
        results.benchmark_successful = true;
        
        results.timing.avg_connection_time_ms = total_connection_time / successful_connections;
        results.timing.min_connection_time_ms = min_time;
        results.timing.max_connection_time_ms = max_time;
        results.timing.connection_time_ms = results.timing.avg_connection_time_ms;
        results.timing.round_trip_time_ms = results.timing.avg_connection_time_ms;
        
        std::cout << "Completed " << successful_connections << "/" << iterations 
                  << " connection cycles successfully.\n";
    } else {
        results.error_message = "All connection attempts failed";
        std::cout << "All connection attempts failed.\n";
    }
#else
    results.error_message = "Network benchmarking not supported on this platform";
    results.timing.connection_successful = false;
#endif

    return results;
}

void NetworkBenchmark::print_results(const Results& results) {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  Network Benchmark Results\n";
    std::cout << "========================================\n";
    std::cout << "\n";

    std::cout << "Configuration:\n";
    std::cout << "  " << std::left << std::setw(25) << "Target Host:" 
              << results.target_host << "\n";
    std::cout << "  " << std::left << std::setw(25) << "Target Port:" 
              << results.target_port << "\n";
    std::cout << "  " << std::left << std::setw(25) << "Payload Size:";
    if (results.payload_size_bytes < 1024) {
        std::cout << results.payload_size_bytes << " bytes\n";
    } else {
        std::cout << std::fixed << std::setprecision(2) 
                  << (results.payload_size_bytes / 1024.0) << " KB\n";
    }
    if (results.iterations > 1) {
        std::cout << "  " << std::left << std::setw(25) << "Iterations:"
                  << results.iterations << "\n";
    }
    std::cout << "\n";

    std::cout << "Connection Status:\n";
    std::cout << "  " << std::left << std::setw(25) << "Connection:"
              << (results.timing.connection_successful ? "SUCCESS" : "FAILED") << "\n";
    std::cout << "  " << std::left << std::setw(25) << "Data Exchange:"
              << (results.timing.data_exchange_successful ? "SUCCESS" : "FAILED") << "\n";
    
    if (!results.error_message.empty()) {
        std::cout << "  " << std::left << std::setw(25) << "Error:"
                  << results.error_message << "\n";
    }
    std::cout << "\n";

    if (results.timing.connection_successful) {
        std::cout << "Timing Statistics:\n";
        if (results.iterations > 1) {
            std::cout << "  " << std::left << std::setw(25) << "Avg Connection Time:"
                      << std::fixed << std::setprecision(3) 
                      << results.timing.avg_connection_time_ms << " ms\n";
            std::cout << "  " << std::left << std::setw(25) << "Min Connection Time:"
                      << std::fixed << std::setprecision(3) 
                      << results.timing.min_connection_time_ms << " ms\n";
            std::cout << "  " << std::left << std::setw(25) << "Max Connection Time:"
                      << std::fixed << std::setprecision(3) 
                      << results.timing.max_connection_time_ms << " ms\n";
        } else {
            std::cout << "  " << std::left << std::setw(25) << "Connection Time:"
                      << std::fixed << std::setprecision(3) 
                      << results.timing.connection_time_ms << " ms\n";
        }
        
        if (results.benchmark_successful) {
            std::cout << "  " << std::left << std::setw(25) << "Send Time:"
                      << std::fixed << std::setprecision(3) 
                      << results.timing.send_time_ms << " ms\n";
            
            if (results.timing.data_exchange_successful) {
                std::cout << "  " << std::left << std::setw(25) << "Receive Time:"
                          << std::fixed << std::setprecision(3) 
                          << results.timing.receive_time_ms << " ms\n";
                std::cout << "  " << std::left << std::setw(25) << "Round-Trip Time:"
                          << std::fixed << std::setprecision(3) 
                          << results.timing.round_trip_time_ms << " ms\n";
            } else {
                std::cout << "  " << std::left << std::setw(25) << "Receive Time:"
                          << "N/A (server did not echo)\n";
                std::cout << "  " << std::left << std::setw(25) << "Round-Trip Time:"
                          << std::fixed << std::setprecision(3) 
                          << results.timing.round_trip_time_ms << " ms (send only)\n";
            }
        }
        std::cout << "\n";
    }

    // Print limitations
    std::cout << "Limitations:\n";
    std::cout << "  - Network timing depends on network conditions\n";
    std::cout << "  - Results vary with network load and latency\n";
    std::cout << "  - Requires network connectivity and reachable target\n";
    std::cout << "  - Firewall rules may block connections\n";
    std::cout << "  - Target server must accept connections\n";
    std::cout << "\n";
}

void NetworkBenchmark::print_comparison(
    const Results& network_results,
    double memory_latency_ns
) {
    if (!network_results.benchmark_successful || memory_latency_ns <= 0.0) {
        return;
    }

    std::cout << "========================================\n";
    std::cout << "  Network vs Memory Latency Comparison\n";
    std::cout << "========================================\n";
    std::cout << "\n";

    double memory_latency_ms = memory_latency_ns / 1'000'000.0;
    double network_rtt_ms = network_results.timing.round_trip_time_ms;
    
    std::cout << "Latency Comparison:\n";
    std::cout << "  " << std::left << std::setw(30) << "Memory Latency:"
              << std::fixed << std::setprecision(3) 
              << memory_latency_ms << " ms\n";
    
    if (network_results.timing.data_exchange_successful && network_rtt_ms > 0.0) {
        std::cout << "  " << std::left << std::setw(30) << "Network Round-Trip:"
                  << std::fixed << std::setprecision(3) 
                  << network_rtt_ms << " ms\n";
        
        if (memory_latency_ms > 0.0) {
            double ratio = network_rtt_ms / memory_latency_ms;
            std::cout << "  " << std::left << std::setw(30) << "Network/Memory Ratio:"
                      << std::fixed << std::setprecision(2) 
                      << ratio << "x\n";
            
            if (ratio > 1000.0) {
                std::cout << "  Note: Network latency is " 
                          << std::fixed << std::setprecision(1) 
                          << (ratio / 1000.0) << "K times slower than memory\n";
            } else if (ratio > 1.0) {
                std::cout << "  Note: Network latency is " 
                          << std::fixed << std::setprecision(1) 
                          << ratio << " times slower than memory\n";
            }
        }
    } else {
        // Use connection + send time as approximation
        double network_total_ms = network_results.timing.connection_time_ms + 
                                  network_results.timing.send_time_ms;
        std::cout << "  " << std::left << std::setw(30) << "Network (Connection+Send):"
                  << std::fixed << std::setprecision(3) 
                  << network_total_ms << " ms\n";
        std::cout << "  Note: Server did not echo data, using connection+send time\n";
        
        if (network_total_ms > 0.0 && memory_latency_ms > 0.0) {
            double ratio = network_total_ms / memory_latency_ms;
            std::cout << "  " << std::left << std::setw(30) << "Network/Memory Ratio:"
                      << std::fixed << std::setprecision(2) 
                      << ratio << "x\n";
        }
    }
    std::cout << "\n";
}

void NetworkBenchmark::print_cpu_comparison(
    const Results& network_results,
    double cpu_time_per_op_ns
) {
    if (!network_results.benchmark_successful || cpu_time_per_op_ns <= 0.0) {
        return;
    }

    std::cout << "========================================\n";
    std::cout << "  Network vs CPU Performance Comparison\n";
    std::cout << "========================================\n";
    std::cout << "\n";

    double cpu_time_ms = cpu_time_per_op_ns / 1'000'000.0;
    double network_time_ms = network_results.timing.avg_connection_time_ms > 0.0 ?
                             network_results.timing.avg_connection_time_ms :
                             network_results.timing.connection_time_ms;
    
    std::cout << "Performance Comparison:\n";
    std::cout << "  " << std::left << std::setw(30) << "CPU Time per Operation:"
              << std::fixed << std::setprecision(6) 
              << cpu_time_ms << " ms\n";
    
    std::cout << "  " << std::left << std::setw(30) << "Network Connection Time:"
              << std::fixed << std::setprecision(3) 
              << network_time_ms << " ms\n";
    
    if (cpu_time_ms > 0.0) {
        double ratio = network_time_ms / cpu_time_ms;
        std::cout << "  " << std::left << std::setw(30) << "Network/CPU Ratio:"
                  << std::fixed << std::setprecision(2) 
                  << ratio << "x\n";
        
        if (ratio > 1000.0) {
            std::cout << "  Note: Network operations are " 
                      << std::fixed << std::setprecision(1) 
                      << (ratio / 1000.0) << "K times slower than CPU operations\n";
        } else if (ratio > 1.0) {
            std::cout << "  Note: Network operations are " 
                      << std::fixed << std::setprecision(1) 
                      << ratio << " times slower than CPU operations\n";
        }
    }
    
    std::cout << "\n";
}

int NetworkBenchmark::connect_to_host(
    const std::string& host,
    std::uint16_t port,
    double& connection_time_ms
) noexcept {
#ifdef __linux__
    // Resolve hostname
    struct addrinfo hints{};
    struct addrinfo* result = nullptr;
    
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;  // IPv4 only for simplicity
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int status = getaddrinfo(host.c_str(), nullptr, &hints, &result);
    if (status != 0) {
        return -1;
    }

    // Create socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd < 0) {
        freeaddrinfo(result);
        return -1;
    }

    // Set socket timeout for connection
    struct timeval timeout{};
    timeout.tv_sec = 5;  // 5 second timeout
    timeout.tv_usec = 0;
    setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    // Prepare address structure
    struct sockaddr_in server_addr{};
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Copy resolved address
    if (result->ai_family == AF_INET) {
        struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(result->ai_addr);
        server_addr.sin_addr = addr->sin_addr;
    } else {
        freeaddrinfo(result);
        close_socket(socket_fd);
        return -1;
    }

    freeaddrinfo(result);

    // Measure connection time
    Timer connection_timer;
    connection_timer.start();

    // Attempt connection (blocking)
    int connect_result = connect(socket_fd, 
                                  reinterpret_cast<struct sockaddr*>(&server_addr),
                                  sizeof(server_addr));
    
    connection_time_ms = connection_timer.elapsed_milliseconds();

    if (connect_result < 0) {
        close_socket(socket_fd);
        return -1;
    }

    return socket_fd;
#else
    (void)host;
    (void)port;
    (void)connection_time_ms;
    return -1;
#endif
}

ssize_t NetworkBenchmark::send_data(
    int socket_fd,
    const void* data,
    std::size_t size,
    double& send_time_ms
) noexcept {
#ifdef __linux__
    Timer send_timer;
    send_timer.start();

    ssize_t total_sent = 0;
    const std::uint8_t* data_ptr = static_cast<const std::uint8_t*>(data);
    
    while (total_sent < static_cast<ssize_t>(size)) {
        ssize_t bytes_sent = send(socket_fd, 
                                      data_ptr + total_sent,
                                      size - total_sent,
                                      0);
        
        if (bytes_sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            send_time_ms = send_timer.elapsed_milliseconds();
            return -1;
        }
        
        total_sent += bytes_sent;
    }

    send_time_ms = send_timer.elapsed_milliseconds();
    return total_sent;
#else
    (void)socket_fd;
    (void)data;
    (void)size;
    (void)send_time_ms;
    return -1;
#endif
}

ssize_t NetworkBenchmark::receive_data(
    int socket_fd,
    void* buffer,
    std::size_t size,
    double& receive_time_ms
) noexcept {
#ifdef __linux__
    Timer receive_timer;
    receive_timer.start();

    ssize_t total_received = 0;
    std::uint8_t* buffer_ptr = static_cast<std::uint8_t*>(buffer);
    
    // Set a timeout for receive
    struct timeval timeout{};
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    while (total_received < static_cast<ssize_t>(size)) {
        ssize_t bytes_received = recv(socket_fd,
                                           buffer_ptr + total_received,
                                           size - total_received,
                                           0);
        
        if (bytes_received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                continue;
            }
            receive_time_ms = receive_timer.elapsed_milliseconds();
            return -1;
        }
        
        if (bytes_received == 0) {
            // Connection closed
            break;
        }
        
        total_received += bytes_received;
    }

    receive_time_ms = receive_timer.elapsed_milliseconds();
    return total_received;
#else
    (void)socket_fd;
    (void)buffer;
    (void)size;
    (void)receive_time_ms;
    return -1;
#endif
}

void NetworkBenchmark::close_socket(int socket_fd) noexcept {
#ifdef __linux__
    if (socket_fd >= 0) {
        close(socket_fd);
    }
#else
    (void)socket_fd;
#endif
}

bool NetworkBenchmark::single_connection_cycle(
    const std::string& host,
    std::uint16_t port,
    std::size_t payload_size_bytes,
    double& connection_time_ms
) noexcept {
#ifdef __linux__
    Timer cycle_timer;
    cycle_timer.start();
    
    double connect_time = 0.0;
    int socket_fd = connect_to_host(host, port, connect_time);
    
    if (socket_fd < 0) {
        connection_time_ms = 0.0;
        return false;
    }
    
    // Send small payload
    std::vector<std::uint8_t> send_buffer(payload_size_bytes, 0xAA);
    double send_time = 0.0;
    ssize_t bytes_sent = send_data(socket_fd, send_buffer.data(), 
                                   payload_size_bytes, send_time);
    
    close_socket(socket_fd);
    
    connection_time_ms = cycle_timer.elapsed_milliseconds();
    
    return (bytes_sent > 0);
#else
    (void)host;
    (void)port;
    (void)payload_size_bytes;
    connection_time_ms = 0.0;
    return false;
#endif
}

