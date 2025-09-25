# High-Performance Electronic Trading Ecosystem in C++

### 1. About

This report details the design and implementation of a complete, low-latency electronic trading ecosystem developed in modern C++. The project simulates the core infrastructure of a financial exchange and a corresponding algorithmic trading client, focusing on high-throughput, low-latency performance. The system comprises an exchange with a matching engine, order gateway, and market data publisher, alongside a client capable of executing market making and liquidity taking strategies. The implementation emphasizes advanced C++ techniques, low-level performance optimizations, and a robust, multi-threaded architecture to model the demanding requirements of real-world high-frequency trading (HFT) systems.

### 2. Project Objectives

The primary goal of this project was to gain practical, hands-on experience in building performance-critical systems. Key objectives included:
* To architect a complex, multi-component system with decoupled services running in parallel.
* To implement advanced C++ techniques for minimizing latency and jitter, specifically focusing on memory management and concurrent programming.
* To develop a deep understanding of low-level network programming using the POSIX socket API and the `epoll` mechanism for high-performance I/O.
* To model and implement the fundamental logic of a financial exchange, including a limit order book, matching algorithm, and data dissemination protocols.

### 3. System Architecture

The ecosystem is divided into two primary applications that run concurrently: the Exchange and the Trading Client. Each application is composed of several independent, thread-safe components that communicate via high-speed, lock-free queues.

#### 3.1 The Exchange
The exchange application is multi-threaded, with each core component running on its own thread to maximize parallelism and throughput.
* **Order Gateway Server:** Acts as the entry point for clients. It manages reliable TCP connections, receives client order requests, and sends back private order responses and execution reports to the specific client who owns the order.
* **Matching Engine:** The core of the exchange, containing the central limit order book. It processes validated requests to add, cancel, or match orders based on a strict Price/Time Priority (FIFO) algorithm.
* **Market Data Publisher:** Responsible for disseminating public market activity. It broadcasts anonymous order book updates and trade events to all subscribed clients via an efficient UDP multicast stream.

#### 3.2 The Trading Client
The client application is also multi-threaded, designed to react to market data and execute trading strategies with minimal delay.
* **Market Data Consumer:** Subscribes to the exchange's UDP multicast feeds. It is responsible for decoding the binary market data protocol and handling the snapshot/incremental logic to maintain an accurate local order book, including recovery from packet loss.
* **Order Gateway Client:** Manages the TCP connection to the exchange's Order Gateway. It encodes strategy-generated orders into the binary protocol for sending and decodes private responses from the exchange.
* **Trading Engine:** The "brain" of the client. It consumes market data, runs trading algorithms, manages risk, and tracks positions and P&L.

#### 3.3 Inter-Component Communication
All internal communication between the threads listed above (e.g., from the Order Gateway to the Matching Engine) is handled by custom-built, **lock-free Single-Producer, Single-Consumer (SPSC) queues**. This design avoids the high overhead and contention of traditional mutex-based synchronization, which is critical for maintaining low latency.

### 4. Core Low-Latency Techniques Implemented

This project moved beyond standard library abstractions to implement several foundational low-latency techniques from scratch:

* **Custom Memory Management:** A **Memory Pool** class was developed to pre-allocate memory at startup. All performance-critical objects (like orders and price levels) are served from this pool.
* **Lock-Free Concurrency:** A cache-aligned **SPSC Lock-Free Queue** was implemented using `std::atomic` and correct C++ memory ordering.
* **High-Performance Networking:**
    * A C++ wrapper around the **POSIX Socket API** was built to manage network connections.
* **Asynchronous Design:** A dedicated, low-priority thread and a lock-free queue are used for the **Logging Framework**.

### 5. Key Components & Functionality

* **Matching Algorithm:** The matching engine implements a strict **Price/Time Priority (FIFO)** algorithm, the most common type used in real financial markets. Orders are first prioritized by the best price (highest bid, lowest ask) and then by their arrival time.
* **FIFO Sequencer:** To ensure fairness, the Order Gateway contains a sequencer that sorts all incoming requests from all clients by their precise kernel receive timestamp before forwarding them to the Matching Engine.
* **Market Data Protocol:** A custom binary protocol was designed for market data, featuring an efficient **incremental feed** for live updates and a separate **snapshot feed** for recovery and late joins.
* **Trading Algorithms:** The client-side Trading Engine was built to support two fundamental strategy types: a **Market Maker** that provides passive liquidity and a **Liquidity Taker** that aggressively crosses the spread.

### 6. Technology Stack

* **Language:** Modern C++ (C++17/20)
* **Compiler:** GCC
* **Build System:** CMake & Ninja
* **Operating System:** Linux
* **Core APIs:** POSIX Sockets, Pthreads, `epoll`
