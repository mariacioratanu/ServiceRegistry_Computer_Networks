#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <sqlite3.h>
#include <time.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "logging.h"
#include "JsonParser.h"

/* Port used */
#define PORT 2908

typedef struct thData {
    int idThread; // thread id
    int cl;       // client descriptor
    char username[51];
} thData;

class Service{
    public:
        Service(std::string name, std::string ip, int port, std::string tex){
            this->name = name;
            this->ip = ip;
            this->port = port;
            this->tex = tex;
        }
        std::string name;
        std::string ip;
        int port;
        std::string tex; // time of expiration
};

class User{
    public:
        User(std::string username, std::string password, std::string role){
            this->username = username;
            this->password = password;
            this->role = role;
        }
        std::string username;
        std::string password;
        std::string role; // admin or user
};

template <class T, class K>
class Repository {
public:
    virtual void add(T obj) = 0;
    virtual void remove(K identity) = 0;
    virtual void update(T obj) = 0;
    virtual T getById(std::string id) = 0;
    virtual std::vector<T> getAll() = 0;

    // Singleton instance accessor
    static Repository<T, K>* getInstance() {
        static Repository<T, K>* instance = nullptr;
        if (!instance) {
            instance = new Repository<T, K>();
        }
        return instance;
    }

protected:
    Repository() {} // Protected constructor to prevent instantiation
};

class UserRepository : public Repository<User, std::string> {
public:
    static UserRepository* getInstance() {
        static UserRepository instance; // Ensures single instance
        return &instance;
    }

    void add(User obj) override {
        log_message("Adding user to database");
        if (!db) {
            log_message("Database not initialized");
            return;
        }

        const char* sqlInsert = "INSERT INTO users (username, password, role) VALUES (?, ?, ?);";
        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, nullptr) != SQLITE_OK) {
            log_message("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
            return;
        }

        sqlite3_bind_text(stmt, 1, obj.username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, obj.password.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, obj.role.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            log_message("Failed to execute statement: " + std::string(sqlite3_errmsg(db)));
        } else {
            log_message("User added successfully");
        }

        sqlite3_finalize(stmt);
    }

    // Implement other required methods as needed
    void remove(std::string id) override {
        // Code for removing a user
    }

    void update(User obj) override {
        // Code for updating a user
    }

    User getById(std::string id) override {
        // Code for retrieving a user by ID
        return User("", "", "");
    }

    std::vector<User> getAll() override {
        // Code for retrieving all users
        return std::vector<User>();
    }

private:
    UserRepository() {
        // Open the database connection
        if (sqlite3_open("service_registry.db", &db) != SQLITE_OK) {
            log_message("Cannot open database: " + std::string(sqlite3_errmsg(db)));
            db = nullptr;
        } else {
            log_message("Database opened successfully");

            // Create the users table if it doesn't exist
            const char* sqlCreateTable = "CREATE TABLE IF NOT EXISTS users ("
                                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                         "username TEXT NOT NULL UNIQUE,"
                                         "password TEXT NOT NULL,"
                                         "role TEXT NOT NULL);";

            char* errMsg = nullptr;
            if (sqlite3_exec(db, sqlCreateTable, nullptr, nullptr, &errMsg) != SQLITE_OK) {
                log_message("SQL error: " + std::string(errMsg));
                sqlite3_free(errMsg);
            } else {
                log_message("Table 'users' created or already exists");
            }
        }
    }

    ~UserRepository() {
        if (db) {
            sqlite3_close(db);
            db = nullptr;
            log_message("Database connection closed");
        }
    }

    sqlite3* db;

    // Delete copy constructor and assignment operator to enforce singleton
    UserRepository(const UserRepository&) = delete;
    UserRepository& operator=(const UserRepository&) = delete;
};

class Request {
public:
    std::string sender_ip;
    int sender_port;
    std::string command;
};

class Response {
public:
    std::string status;
    std::string message;
};

class UserHandler {
public:
    // Static method to access the singleton instance
    static UserHandler* getInstance() {
        static UserHandler instance;
        return &instance;
    }

    Response addUser(User user) {
        userRepository->add(user);
        return Response{ "ok", "User added successfully" };
    }

    void removeUser(std::string id) {
        userRepository->remove(id);
    }

    void updateUser(User user) {
        userRepository->update(user);
    }

    User getUserById(std::string id) {
        return userRepository->getById(id);
    }

    std::vector<User> getAllUsers() {
        return userRepository->getAll();
    }

private:
    UserHandler() : userRepository(UserRepository::getInstance()) {} // Private constructor

    UserRepository* userRepository;

    // Delete copy constructor and assignment operator to enforce singleton
    UserHandler(const UserHandler&) = delete;
    UserHandler& operator=(const UserHandler&) = delete;
};

class Controller {
public:
    Controller() : userHandler(UserHandler::getInstance()) {}

    static Response processRequest(const std::string& request) {
        JsonParser parser;
        parser.parse(request);

        std::string command = parser["command"];
        std::string object = parser["object"];

        if (object == "user") {
            if (command == "register") {
                std::string username = parser["username"];
                std::string password = parser["password"];
                std::string role = parser["role"];

                User newUser(username, password, role);

                return UserHandler::getInstance()->addUser(newUser);
            } else if (command == "delete") {
                // Handle delete command
            }
        }
        return Response{ "error", "Invalid command" };
    }

private:
    UserHandler* userHandler; // Singleton instance of UserHandler
};

class LinuxAPIServerCommunication {
public:
    // Wrapper function for reading from a socket
    static ssize_t safe_read(int socket, char *buffer, size_t size) {
        ssize_t bytesRead = read(socket, buffer, size);
        if (bytesRead < 0) {
            log_message("Error reading from socket: " + std::string(strerror(errno)));
        }
        return bytesRead;
    }

    // Wrapper function for writing to a socket
    static bool safe_write(int socket, const std::string &message) {
        ssize_t bytesWritten = write(socket, message.c_str(), message.size());
        if (bytesWritten < 0) {
            log_message("Error writing to socket: " + std::string(strerror(errno)));
            return false;
        }
        return true;
    }

    static void *treat(void *arg) {
        thData tdL = *((thData *)arg);
        log_message(std::string("Thread ") + std::to_string(tdL.idThread) + " started and waiting for client socket " + std::to_string(tdL.cl));

        pthread_detach(pthread_self());
        LinuxAPIServerCommunication::respond((struct thData *)arg);

        log_message(std::string("Thread ending socket: ") + std::to_string(tdL.cl));

        close(tdL.cl);
        return NULL;
    }

    static void respond(void *arg) {
        thData tdL = *((thData *)arg);
        char buffer[1024];

        while (1) {
            Response response;
            ssize_t bytesRead = safe_read(tdL.cl, buffer, sizeof(buffer) - 1);
            if (bytesRead <= 0) {
                log_message(std::string("Thread ") + std::to_string(tdL.idThread) + ": Error or disconnection at read()");
                break;
            }
            buffer[bytesRead] = '\0';

            log_message(std::string("Thread ") + std::to_string(tdL.idThread) + " received: " + buffer);

            if (strcmp(buffer, "quit\n") == 0) {
                log_message(std::string("Thread ") + std::to_string(tdL.idThread) + " received quit command");
                break;
            } else {
                response = Controller::processRequest(buffer);
            }

            // Build JSON response
            std::ostringstream responseStream;
            responseStream << "{ \"status\": \"" << response.status << "\", \"message\": \"" << response.message << "\" }";
            std::string responseString = responseStream.str();

            if (!safe_write(tdL.cl, responseString)) {
                log_message(std::string("Thread ") + std::to_string(tdL.idThread) + ": Error at write()");
                break;
            }

            log_message(std::string("Thread ") + std::to_string(tdL.idThread) + " sent: " + responseString);
        }
        close(tdL.cl);
    }
};

class Server {
public:
    static int run() {
        log_message("Server starting");
        struct sockaddr_in server, from;
        int sd, i = 0;
        pthread_t th[100];

        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            log_message("Error at socket()");
            return errno;
        }

        int on = 1;
        setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

        bzero(&server, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = htonl(INADDR_ANY);
        server.sin_port = htons(PORT);

        if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
            log_message("Error at bind()");
            return errno;
        }

        if (listen(sd, 2) == -1) {
            log_message("Error at listen()");
            return errno;
        }

        log_message("Server listening for connections...");
        while (1) {
            int client;
            thData *td;
            socklen_t length = sizeof(from);

            if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0) {
                log_message("Error at accept()");
                continue;
            }

            log_message(std::string("Server accepted client socket: ") + std::to_string(client));

            td = (struct thData *)malloc(sizeof(struct thData));
            td->idThread = i++;
            td->cl = client;

            pthread_create(&th[i], NULL, &LinuxAPIServerCommunication::treat, td);
        }
        close(sd);
        return 0;
    }
};

int main() {
    return Server::run();
}
