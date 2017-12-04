#pragma once
/////////////////////////////////////////////////////////////////////////
// CLient.h - Demonstrates simple one-way HTTP style messaging    //
//                 and file transfer                                   //
//                                                                     //
// Karthik Palepally Muniyappa, CSE687 - Object Oriented Design, Spring 2017//
// Application: OOD Project #4                                         //
// Platform:    Visual Studio 2015, Dell XPS 8900, Windows 10 pro      //
/////////////////////////////////////////////////////////////////////////
/*This package Implements functionality to connect with a remote server and erform the folowing operataions
Upload files to the server
Publish files in the server
List the files in the server
Downlaod files in the server
Delete files in the server

Public Interface :
== == == == == == == == =
std::string uploadFiles(std::string files, int category);
std::string deleteFiles(int category);
std::string publishFiles(int category);
std::string listFiles(int category);
std::string downloadFiles(int category, std::string fileName);
*
* Build Process :
*--------------
*   devenv Client.sln / debug rebuild
*
* Maintenance History :
*--------------------
ver 0.1 : 7nd March 2017
*
/*
* -
*/

/*
* Required Files:
*   Client.cpp,
*   HttpMessage.h, HttpMessage.cpp
*   Cpp11-BlockingQueue.h
*   Sockets.h, Sockets.cpp
*   FileSystem.h, FileSystem.cpp
*   Logger.h, Logger.cpp
*   Utilities.h, Utilities.cpp
*/
/*

*/
//#include "../HttpMessage/HttpMessage.h"
//#include "../Sockets/Sockets.h"
//#include "FileSystem.h"
//#include "../Logger/Logger.h"
//#include "../HttpMessage/Utilities.h"
//#include <string>
//#include <iostream>
#include <thread>
#include "../HttpMessage/HttpMessage.h"
#include "../Sockets/Sockets.h"
#include "FileSystem.h"
#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
#include "../Logger/Logger.h"
#include "../HttpMessage/Utilities.h"
#include <string>
#include<stack>
#include <iostream>
#include<Windows.h>
#include <windows.h>
#include <ShellAPI.h>

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
	bool connectionClosed_;
	std::string repoPath;
	HttpMessage readMessage(Socket& socket);
	bool readFile(const std::string& filename, size_t fileSize, Socket& socket);
	Async::BlockingQueue<HttpMessage>& msgQ_;
};
//----< this defines processing to frame messages >------------------

class MsgClient
{
public:
	using EndPoint = std::string;
	//void execute(const size_t TimeBetweenMessages, const size_t NumMessages);
	void startListener(int);
	MsgClient(int port, std::string storagePath) :portNumber(port), repoPath(storagePath) {
		std::cout << "*************REQUIREMENT 9: Running an Automated test suit to demonstrate Requirements************\n";
		std::cout << "*************REQUIREMENT 6 and 7: Client Communicates with server using sockets and HttpMessages using either synchronous request/response or asynchronous one-way messaging************\n";
		std::string fpath = "..\\..\\ClientRepository\\SourceFiles\\Category1\\";
		std::vector<std::string> files = FileSystem::Directory::getFiles(fpath, "*.*");
		std::string fileStr;
		for (std::string file : files)
		{
			fileStr = fpath+file + ","+fileStr;
			//fileStr =   file + "," + fileStr;
		}
		fileStr.pop_back();
		uploadFiles(fileStr, 1);
		::Sleep(1000);
		publishFiles(1);
		::Sleep(2000);
		listFiles(1);
		::Sleep(1000);
		downloadFiles(1, "XmlElement.h.htm");
		::Sleep(1000);
		deleteFiles(1);
	};
	std::string uploadFiles(std::string files, int category);
	std::string deleteFiles(int category);
	std::string publishFiles(int category);
	std::string listFiles(int category);
	std::string downloadFiles(int category, std::string fileName);
private:
	std::vector<std::string> splitVect(const std::string &s, char delim);
	HttpMessage listener();
	void openHtmlPages(std::string fpath);
	std::string serverAddress = "localhost:8081";
	void sendCustomMessage(std::string type,std::string category, std::string toAddress, std::string msgBody);
	std::string getCategory(int category);
	std::vector<std::string> filestoOpen;
	void sendFilesToServer(std::string category, std::string toAddr, std::vector<std::string> files);
	int portNumber;
	std::string repoPath;
	HttpMessage makeMessage(size_t n, const std::string& msgBody, const EndPoint& ep);
	void sendMessage(HttpMessage& msg, Socket& socket);
	bool sendFile(const std::string& fqname, Socket& socket, std::string toAddr, std::string category);
	std::stack<std::string> split(const std::string s, char delim);
	
};

