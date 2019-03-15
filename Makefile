CXX=g++
CXXOPTIMIZE=
CXXFLAGS= -g -pthread -std=c++11 $(CXXOPTIMIZE)
USERID=123456789
SOURCES_CLIENT=client.cpp Header.cpp Packet.cpp utils.cpp
SOURCES_SERVER=server.cpp Header.cpp Packet.cpp utils.cpp
OBJECTS_CLIENT = $(SOURCES_CLIENT:.cpp=.o)
OBJECTS_SERVER = $(SOURCES_SERVER:.cpp=.o)



all: server client

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

client: $(OBJECTS_CLIENT)
	$(CXX) $(CXXFLAGS) $(OBJECTS_CLIENT) -o client

server: $(OBJECTS_SERVER)
	$(CXX) $(CXXFLAGS) $(OBJECTS_SERVER) -o server

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM server client *.tar.gz

dist: tarball
tarball: clean
	tar -cvzf /tmp/$(USERID).tar.gz --exclude=./.vagrant . && mv /tmp/$(USERID).tar.gz .
