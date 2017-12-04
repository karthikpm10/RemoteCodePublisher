#pragma once
/////////////////////////////////////////////////////////////////////////
// Server.h - Demonstrates simple one-way HTTP style messaging    //
//                 and file transfer                                   //
//                                                                     //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2016           //
// Application: OOD Project #4                                         //
// Platform:    Visual Studio 2015, Dell XPS 8900, Windows 10 pro      //
/////////////////////////////////////////////////////////////////////////
/*
* This package implements a server that receives HTTP style messages and
* files from multiple concurrent clients 
Clients can perform the following operations
Upload files to the server
Publish files in the server
List the files in the server
Downlaod files in the server
Delete files in the server
*/
/*
Public Interface :
== == == == == == == == =
void execute(const size_t TimeBetweenMessages, const size_t NumMessages);
void startListener();
MsgServer(int port,std::string storagePath) :portNumber(port),repoPath(storagePath) {};
* Required Files:
*   Server.cpp, 
*   HttpMessage.h, HttpMessage.cpp
*   Cpp11-BlockingQueue.h
*   Sockets.h, Sockets.cpp
*   FileSystem.h, FileSystem.cpp
*   Logger.h, Logger.cpp
*   Utilities.h, Utilities.cpp
*Build Process :
*--------------
*   devenv Server.sln / debug rebuild
*
* Maintenance History :
*--------------------
ver 0.1 : 7nd March 2017
*
/*
* -
/*
* 
*/
#include <thread>
#include "../HttpMessage/HttpMessage.h"
#include "../Sockets/Sockets.h"
#include "../FileSystem/FileSystem.h"
#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"

#include "../Analyzer/Executive.h"
#include <string>
#include <iostream>
#include<stack>

using Show =Logging:: StaticLogger<1>;
using namespace Utilities;

/////////////////////////////////////////////////////////////////////
// ClientHandler class
/////////////////////////////////////////////////////////////////////
// - instances of this class are passed by reference to a SocketListener
// - when the listener returns from Accept with a socket it creates an
//   instance of this class to manage communication with the client.
// - You no longer need to be careful using data members of this class
//   because each client handler thread gets its own copy of this 
//   instance so you won't get unwanted sharing.
// - I changed the SocketListener semantics to pass
//   instances of this class by value for version 5.2.
// - that means that all ClientHandlers need copy semantics.
//
class ClientHandler
{
public:
	ClientHandler(Async::BlockingQueue<HttpMessage>& msgQ, std::string storagePath) : msgQ_(msgQ), repoPath(storagePath) {}
	void operator()(Socket socket);
private:
	std::string repoPath;
	bool connectionClosed_;
	HttpMessage readMessage(Socket& socket);
	bool readFile(const std::string& filename, size_t fileSize, Socket& socket);
	Async::BlockingQueue<HttpMessage>& msgQ_;
};



class MsgServer
{
public:
	using EndPoint = std::string;
	//void execute(const size_t TimeBetweenMessages, const size_t NumMessages);
	void startListener();
	MsgServer(int port,std::string storagePath) :portNumber(port),repoPath(storagePath) {
		std::cout << "*************REQUIREMENT 6 and 7: Server Communicates with client using sockets and HttpMessages using either synchronous request/response or asynchronous one-way messaging************\n";
	};
private:
	std::unordered_map<std::string, std::vector<std::string>> lazymap1;
	std::unordered_map<std::string, std::vector<std::string>> lazymap2;
	std::unordered_map<std::string, std::vector<std::string>> lazymap3;
	std::vector<std::string> dependentFiles;
	void startCodeAnalysis(std::string category);
	void initializeDependentFiles(std::string category,std::string fileName);
	void getDependentFiles(std::string file, std::unordered_map<std::string, std::vector<std::string>> map);
	int portNumber;
	std::string repoPath;
	void uploadFiles(std::string category, std::string toAddr,std::string filename);
	void sendPublishMessage(std::string toAddress,std::string category);
	//SocketConnecter& createSocket(std::string address);
	std::stack<std::string> split(const std::string s, char delim);
	void processMessage(HttpMessage msg);
	HttpMessage makeMessage(size_t n, const std::string& msgBody, const EndPoint& ep);
	void sendMessage(HttpMessage& msg, Socket& socket);
	bool sendFile(const std::string& fqname, Socket& socket,std::string toAddr,std::string category);
	void listFiles(std::string category, std::string toAddr);
	std::string MsgServer::buildFilesList(std::vector<std::string> fileList);
	void deleteFiles(std::string category, std::string toAddr);
	void sendUploadCompleteMessage(std::string category, std::string toAddr);
};

#ifdef DEBUG_MSGSERVER

int main()
{
	::SetConsoleTitle(L"HttpMessage Server - Runs Forever");

	Show::attach(&std::cout);
	Show::start();
	Show::title("\n  HttpMessage Server started");

	BlockingQueue<HttpMessage> msgQ;

	try
	{
		SocketSystem ss;
		SocketListener sl(8080, Socket::IP6);
		ClientHandler cp(msgQ);
		sl.start(cp);
		/*
		* Since this is a server the loop below never terminates.
		* We could easily change that by sending a distinguished
		* message for shutdown.
		*/
		while (true)
		{
			HttpMessage msg = msgQ.deQ();
			Show::write("\n\n  server recvd message with body contents:\n" + msg.bodyString());
		}
	}
	catch (std::exception& exc)
	{
		Show::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}

#endif // DEBUG

