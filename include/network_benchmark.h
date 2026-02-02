#ifndef NETWORK_BENCHMARK_H
#define NETWORK_BENCHMARK_H

#include <cstdint>
#include <cstddef>
#include <string>

#ifdef __linux__
#include <sys/types.h>  // Provides ssize_t on POSIX systems
#else
// Fallback for non-POSIX systems (functions won't be used anyway)
typedef long ssize_t;
#endif

/**
 * Network Benchmarking Module
 * 
 * Performs basic network timing measurements using standard sockets.
 * Measures connection time, send/receive latency, and round-trip time.
 * 
 * Limitations:
 * - Requires network connectivity
 * - Requires a reachable target host/port
 * - Results depend on network conditions
 * - May fail if firewall blocks connections
 * 
 * Example usage:
 *   NetworkBenchmark benchmark;
 *   auto results = benchmark.run("127.0.0.1", 80, 1024);
 */
class NetworkBenchmark {
public:
    /**
     * Network timing statistics.
     */
    struct TimingStats {
        double connection_time_ms;
        double send_time_ms;
        double receive_time_ms;
        double round_trip_time_ms;
        double avg_connection_time_ms;
        double min_connection_time_ms;
        double max_connection_time_ms;
        bool connection_successful;
        bool data_exchange_successful;
    };

    /**
     * Results structure containing network benchmark metrics.
     */
    struct Results {
        std::string target_host;
        std::uint16_t target_port;
        std::size_t payload_size_bytes;
        std::size_t iterations;
        TimingStats timing;
        std::string error_message;
        bool benchmark_successful;
    };

    /**
     * Constructs a network benchmark instance.
     */
    NetworkBenchmark() noexcept;

    /**
     * Runs the network benchmark.
     * 
     * @param host Target hostname or IP address (e.g., "127.0.0.1" or "example.com")
     * @param port Target port number
     * @param payload_size_bytes Size of payload to send/receive (default: 1024)
     * @return Results structure with benchmark metrics
     */
    Results run(const std::string& host, std::uint16_t port, 
                std::size_t payload_size_bytes = 1024);

    /**
     * Runs the network benchmark with call-like loop behavior.
     * Performs repeated open/close cycles to simulate call behavior.
     * 
     * @param host Target hostname or IP address
     * @param port Target port number
     * @param iterations Number of connection cycles to perform
     * @param payload_size_bytes Size of payload to send/receive (default: 1024)
     * @return Results structure with benchmark metrics
     */
    Results run_call_loop(const std::string& host, std::uint16_t port,
                          std::size_t iterations,
                          std::size_t payload_size_bytes = 1024);

    /**
     * Prints network benchmark results in a clear table format.
     * 
     * @param results The benchmark results to print
     */
    static void print_results(const Results& results);

    /**
     * Compares network latency with memory timing.
     * 
     * @param network_results Network benchmark results
     * @param memory_latency_ns Memory benchmark average latency in nanoseconds
     */
    static void print_comparison(const Results& network_results, 
                                 double memory_latency_ns);

    /**
     * Compares network latency with CPU timing.
     * 
     * @param network_results Network benchmark results
     * @param cpu_time_per_op_ns CPU benchmark time per operation in nanoseconds
     */
    static void print_cpu_comparison(const Results& network_results,
                                     double cpu_time_per_op_ns);

private:
    /**
     * Creates a TCP socket and connects to the target.
     * 
     * @param host Target hostname or IP address
     * @param port Target port number
     * @param connection_time_ms Output parameter for connection time in milliseconds
     * @return Socket file descriptor, or -1 on error
     */
    int connect_to_host(const std::string& host, std::uint16_t port,
                       double& connection_time_ms) noexcept;

    /**
     * Sends data over a socket.
     * 
     * @param socket_fd Socket file descriptor
     * @param data Pointer to data to send
     * @param size Size of data in bytes
     * @param send_time_ms Output parameter for send time in milliseconds
     * @return Number of bytes sent, or -1 on error
     */
    ssize_t send_data(int socket_fd, const void* data, std::size_t size,
                      double& send_time_ms) noexcept;

    /**
     * Receives data from a socket.
     * 
     * @param socket_fd Socket file descriptor
     * @param buffer Buffer to receive data into
     * @param size Size of buffer in bytes
     * @param receive_time_ms Output parameter for receive time in milliseconds
     * @return Number of bytes received, or -1 on error
     */
    ssize_t receive_data(int socket_fd, void* buffer, std::size_t size,
                         double& receive_time_ms) noexcept;

    /**
     * Closes a socket.
     * 
     * @param socket_fd Socket file descriptor to close
     */
    void close_socket(int socket_fd) noexcept;

    /**
     * Performs a single connection cycle and measures timing.
     * 
     * @param host Target hostname or IP address
     * @param port Target port number
     * @param payload_size_bytes Size of payload to send/receive
     * @param connection_time_ms Output parameter for connection time
     * @return true if connection cycle was successful
     */
    bool single_connection_cycle(const std::string& host, std::uint16_t port,
                                 std::size_t payload_size_bytes,
                                 double& connection_time_ms) noexcept;
};

#endif // NETWORK_BENCHMARK_H

