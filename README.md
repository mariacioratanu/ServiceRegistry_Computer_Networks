# 🧭 Service Registry – Distributed Service Discovery in C/C++
Built on Linux in modern C++, the project implements a multithreaded Service Registry for the Computer Networks course: a TCP/JSON server that authenticates clients, persists data in SQLite, and automatically registers, renews, and expires distributed services.

A multithreaded TCP server that provides service registration and discovery across distributed systems. Built to emulate container orchestration behaviors (e.g. Kubernetes), this registry offers dynamic service lookup, automatic expiration, JSON communication, authentication, and full operational logging.

## ⚙️ Tech Stack

- **Language**: C/C++
- **Platform**: Linux / UNIX
- **Concurrency**: 'pthread' (POSIX threads)
- **Networking**: TCP/IP with JSON-based protocol
- **Database**: SQLite (repository pattern)
- **Logging**: Text-based file logging with timestamps

## 🎯 Core Features

- ✅ *Service Registration*: Add a new service with name, IP, port, and expiration time (TEx)
- 🔄 *Service Renewal*: Services can auto-renew to remain active
- 🔍 *Service Lookup*: Retrieve the IP and port of a named service
- 📜 *Service Listing*: Return a list of all currently active services
- 🔐 *Authentication*: Only registered and logged-in users can register or update services
- 🧾 *Logging*: All client-server interactions are logged with timestamp, IP, port, and action
- 🧠 *Service Lifecycle Threads*: Each registered service has its own thread for expiration monitoring

## 🧠 Architecture

The application follows a *modular, concurrent client-server architecture* designed to support distributed service discovery with high reliability and extensibility. Key architectural principles include:

- *TCP-based communication* – Ensures reliable, ordered message exchange between clients and the central registry using a custom JSON-based protocol.
- *Multithreaded server* – Each client connection is handled in a separate POSIX thread (pthread), enabling concurrent processing of multiple requests.
- *Dedicated service lifecycle threads* – For each registered service, a background thread monitors the expiration time (TEx) and either renews or deletes the service accordingly.
- *Authentication layer* – All critical operations (register, update, delete) require prior login, with credentials validated via SQLite.
- *Repository pattern* – Abstracts database access for users and services through UserRepository and ServiceRepository, ensuring separation of concerns and maintainable SQL logic.
- *Structured logging system* – Every request is recorded in server.log with timestamp, IP, port, and action type. Each service also has its own dedicated log file.
- *SQLite-backed persistence* – All user credentials and registered services are stored in a normalized SQLite schema, supporting transactional operations and efficient lookups.
