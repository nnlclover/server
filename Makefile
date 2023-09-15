all:
	g++ main.cpp -lsqlite3 -o main
	g++ server.cpp -o server
	g++ client.cpp -o client
