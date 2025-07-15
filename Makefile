CXX = g++
CXXFLAGS = -Wall -pthread
LDFLAGS = -lsqlite3  # Link SQLite library
TARGETS = client server

all: $(TARGETS)

client: client.cpp logging.cpp
	$(CXX) $(CXXFLAGS) -o client client.cpp logging.cpp JsonParser.cpp $(LDFLAGS)

server: server.cpp logging.cpp
	$(CXX) $(CXXFLAGS) -o server server.cpp logging.cpp JsonParser.cpp $(LDFLAGS)

clean:
	rm -f $(TARGETS)
