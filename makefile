all: 	http_server.cpp
	g++ -o http_server http_server.cpp -std=c++11 -Wall -pedantic -pthread -lboost_system
	g++ -o console.cgi console.cpp -std=c++11 -Wall -pedantic -pthread -lboost_system

