
#include "Server.h"

using Show = Logging::StaticLogger<1>;
using namespace Utilities;
using Utils = StringHelper;

/////////////////////////////////////////////////////////////////////
// ClientCounter creates a sequential number for each client
//
class ClientCounter
{
public:
	ClientCounter() { ++clientCount; }
	size_t count() { return clientCount; }
private:
	static size_t clientCount;
};

size_t ClientCounter::clientCount = 0;


//----< this defines processing to frame messages >------------------

HttpMessage ClientHandler::readMessage(Socket& socket)
{
	connectionClosed_ = false;
	HttpMessage msg;
	while (true)
	{
		std::string attribString = socket.recvString('\n');
		if (attribString.size() > 1)
		{
			HttpMessage::Attribute attrib = HttpMessage::parseAttribute(attribString);
			msg.addAttribute(attrib);
		}
		else
			break;
	}
	if (msg.attributes().size() == 0)
	{
		connectionClosed_ = true;
		return msg;
	}
	if (msg.attributes()[0].first == "POST")
	{
		std::string filename = msg.findValue("file");
		std::string category = msg.findValue("Category");
		if (filename != "")
		{
			size_t contentSize;
			std::string sizeString = msg.findValue("content-length");
			if (sizeString != "")
				contentSize = Converter<size_t>::toValue(sizeString);
			else
				return msg;

			readFile(category+"\\"+filename, contentSize, socket);
		}

		if (filename != "")
		{
			msg.removeAttribute("content-length");
			std::string bodyString = "<file>" + filename + "</file>";
			std::string sizeString = Converter<size_t>::toString(bodyString.size());
			msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
			msg.addBody(bodyString);
		}
		else
		{
			size_t numBytes = 0;
			size_t pos = msg.findAttribute("content-length");
			if (pos < msg.attributes().size())
			{
				numBytes = Converter<size_t>::toValue(msg.attributes()[pos].second);
				Socket::byte* buffer = new Socket::byte[numBytes + 1];
				socket.recv(numBytes, buffer);
				buffer[numBytes] = '\0';
				std::string msgBody(buffer);
				msg.addBody(msgBody);
				delete[] buffer;
			}
		}
	}
	return msg;
}
//----< read a binary file from socket and save >--------------------
/*
* This function expects the sender to have already send a file message,
* and when this function is running, continuosly send bytes until
* fileSize bytes have been sent.
*/
bool ClientHandler::readFile(const std::string& filename, size_t fileSize, Socket& socket)
{
	std::string fqname = repoPath+"SourceCode\\" + filename ;
	FileSystem::File file(fqname);
	file.open(FileSystem::File::out, FileSystem::File::binary);
	if (!file.isGood())
	{
		Show::write("\n\n  can't open file " + fqname);
		return false;
	}

	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];
	size_t bytesToRead;
	while (true)
	{
		if (fileSize > BlockSize)
			bytesToRead = BlockSize;
		else
			bytesToRead = fileSize;

		socket.recv(bytesToRead, buffer);

		FileSystem::Block blk;
		for (size_t i = 0; i < bytesToRead; ++i)
			blk.push_back(buffer[i]);

		file.putBlock(blk);
		if (fileSize < BlockSize)
			break;
		fileSize -= BlockSize;
	}
	file.close();
	return true;
}
//----< receiver functionality is defined by this function >---------

void ClientHandler::operator()(Socket socket)
{
	while (true)
	{
		HttpMessage msg = readMessage(socket);
		if (connectionClosed_ || msg.bodyString() == "quit")
		{
			Show::write("\n\n  clienthandler thread is terminating");
			break;
		}
		msgQ_.enQ(msg);
	}
}

/*
Invokes Code analysis to perform analysis and stores the dependency table information to perform lazy download

*/
void MsgServer::startCodeAnalysis(std::string category)
{
	std::cout << "*************REQUIREMENT 3 : Invoking Repository Program That Provides Functionality To Publish************\n";
	char * array[7];
	std::string path = repoPath + "SourceCode\\" + category;
	std::string x[] = { "CodeAnalyser.exe",path,"*.h","*.cpp","/m","/f","/r" };
	for (int i = 0; i < 7; i++) {
		const char* xx = x[i].c_str();
		array[i] = _strdup(xx);
	}
	CodeAnalysis::CodeAnalysisExecutive obj;
	obj.ProcessCommandLine(7, array);
	if (category == "Category1")
	{
		lazymap1 = obj.startCodeAnalaysis(category);
	}
	else if (category == "Category2")
	{
		lazymap2 = obj.startCodeAnalaysis(category);
	}
	else if (category == "Category3")
	{
		lazymap3 = obj.startCodeAnalaysis(category);
	}	
}
/*
initializes dependentFiles vector to get the dependent files for a given file, to perform lazy download
*/
void MsgServer::initializeDependentFiles(std::string category,std::string fileName)
{
	dependentFiles.clear();
	if(category=="Category1")
	{
		getDependentFiles(fileName, lazymap1);
	}
	else if (category == "Category2")
	{
		getDependentFiles(fileName, lazymap2);
	}
	else if (category == "Category3")
	{
		getDependentFiles(fileName, lazymap3);
	}
	
}
/*
Build the dependentFiles vector to perform lazy download
*/
void MsgServer::getDependentFiles(std::string file, std::unordered_map<std::string, std::vector<std::string>> map)
{
	if (std::find(dependentFiles.begin(), dependentFiles.end(), file) == dependentFiles.end())
	{
		dependentFiles.push_back(file);
	}
	std::vector<std::string> children = map[file];
	for (std::string child : children)
	{
		if (std::find(dependentFiles.begin(), dependentFiles.end(), child) == dependentFiles.end())
		{
			getDependentFiles(child, map);
		}
	}
}
/*
uploadss .htm files to the client on download request
*/
void MsgServer::uploadFiles(std::string category, std::string toAddr,std::string filename)
{
	std::cout << "*************REQUIREMENT 10: Using Lazy Download Strategy to send only the dependent files************\n";
	initializeDependentFiles(category,filename);
	

	std::string fpath = repoPath + "PublishedFiles\\" + category+"\\";
	std::string toadd = split(toAddr, ':').top();
	SocketSystem ss;
	SocketConnecter si;
	while (!si.connect("localhost", std::stoi(toadd)))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	dependentFiles.push_back("hideHandler.js");
	dependentFiles.push_back("style.css");
	//std::vector<std::string> files = FileSystem::Directory::getFiles(fpath, "*.*");

	for (std::string file : dependentFiles)
	{
		Show::write("\n\n  sending file " + file);
		std::string filPathForDownload = fpath + file;
		sendFile(filPathForDownload, si, toAddr, category);
	}
	HttpMessage terminateMsg;
	terminateMsg = makeMessage(1, "Terminating download", toAddr);
	terminateMsg.addAttribute(HttpMessage::Attribute("Type", "Download"));
	terminateMsg.addAttribute(HttpMessage::Attribute("status", "complete"));
	terminateMsg.addAttribute(HttpMessage::attribute("Category", category));
	sendMessage(terminateMsg, si);
}
/*
send a message to the client once the publishing of files is done
*/
void MsgServer::sendPublishMessage(std::string toAddress,std::string category)
{
	std::string toadd = split(toAddress, ':').top();
	SocketSystem ss;
	SocketConnecter si;
	while (!si.connect("localhost", std::stoi(toadd)))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	HttpMessage msg=makeMessage(1, "Message Published", toAddress);
	msg.addAttribute(HttpMessage::attribute("Category", category));
	msg.addAttribute(HttpMessage::attribute("Type", "Publish"));
	sendMessage(msg, si);

	si.close();
}
/*splits the string on the given delimiter and return the stack
*/
std::stack<std::string> MsgServer::split(const std::string s, char delim)
{
	std::stack<std::string> elems;
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		if (item.length() > 0) {
			elems.push(item);
		}
	}
	return elems;
}
/*
Processes the message received by the server and performs the necessary operataions for the requests
*/
void MsgServer::processMessage(HttpMessage msg)
{
	HttpMessage msgg = msg;
	msg.findValue("toAddr");
	std::string category= msg.findValue("Category");
	std::string type = msg.findValue("Type");
	
	if (type == "Publish")
	{

		startCodeAnalysis(category);
		sendPublishMessage(msg.findValue("fromAddr"), category);
	}
	else if (type ==  "Download")
	{
		uploadFiles(category, msg.findValue("fromAddr"),msg.findValue("FileName"));
	}
	else if (type== "Delete")
	{
		deleteFiles(category, msg.findValue("fromAddr"));
	}
	else if (type == "ListFiles")
	{
		listFiles(category, msg.findValue("fromAddr"));
	}
	else if (type == "Upload")
	{
		sendUploadCompleteMessage(category, msg.findValue("fromAddr"));
		// check the message body and send a message to the client saying upload done
	}
}
/*
Builds a HttpMessage with the given parameters

*/

HttpMessage MsgServer::makeMessage(size_t n, const std::string& body, const EndPoint& ep)
{
	HttpMessage msg;
	HttpMessage::Attribute attrib;
	EndPoint myEndPoint = "localhost:"+ std::to_string(portNumber); 
									
	switch (n)
	{
	case 1:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("POST", "Message"));
		msg.addAttribute(HttpMessage::Attribute("mode", "oneway"));
		msg.addAttribute(HttpMessage::parseAttribute("toAddr:" + ep));
		msg.addAttribute(HttpMessage::parseAttribute("fromAddr:" + myEndPoint));

		msg.addBody(body);
		if (body.size() > 0)
		{
			attrib = HttpMessage::attribute("content-length", Converter<size_t>::toString(body.size()));
			msg.addAttribute(attrib);
		}
		break;
	default:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("Error", "unknown message type"));
	}
	return msg;
}
//----< send message using socket >----------------------------------

void MsgServer::sendMessage(HttpMessage& msg, Socket& socket)
{
	std::string msgString = msg.toString();
	socket.send(msgString.size(), (Socket::byte*)msgString.c_str());
}
//----< send file using socket >-------------------------------------
/*
* - Sends a message to tell receiver a file is coming.
* - Then sends a stream of bytes until the entire file
*   has been sent.
* - Sends in binary mode which works for either text or binary.
*/
bool MsgServer::sendFile(const std::string& filename, Socket& socket,std::string toAddress,std::string category)
{
	std::cout << "*************REQUIREMENT 8:  support sending and receiving streams of bytes6. Streams will be established with an initial exchange of messages************\n";
	// assumes that socket is connected
	std::string fname = filename;
	FileSystem::FileInfo fi(filename);
	size_t fileSize = fi.size();
	std::string sizeString = Converter<size_t>::toString(fileSize);
	FileSystem::File file(filename);
	file.open(FileSystem::File::in, FileSystem::File::binary);
	if (!file.isGood())
		return false;

	HttpMessage msg = makeMessage(1, "", toAddress);
	msg.addAttribute(HttpMessage::Attribute("file", split(fname,'\\').top()));
	msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
	msg.addAttribute(HttpMessage::attribute("Category", category));
	sendMessage(msg, socket);
	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];
	while (true)
	{
		FileSystem::Block blk = file.getBlock(BlockSize);
		if (blk.size() == 0)
			break;
		for (size_t i = 0; i < blk.size(); ++i)
			buffer[i] = blk[i];
		socket.send(blk.size(), buffer);
		if (!file.isGood())
			break;
	}
	file.close();
	return true;
}
/*
List files for the given cataegory in the server and sends a message to the client
*/
void MsgServer::listFiles(std::string category, std::string toAddr)
{
	try
	{
		std::string fpath = repoPath + "PublishedFiles\\" + category + "\\";
		std::string toadd = split(toAddr, ':').top();
		SocketSystem ss;
		SocketConnecter si;
		while (!si.connect("localhost", std::stoi(toadd)))
		{
			Show::write("\n client waiting to connect");
			::Sleep(100);
		}
		HttpMessage msg;
		std::vector<std::string> files = FileSystem::Directory::getFiles(fpath, "*.*");

		std::string msgBody = "List of files in the Repository";
		msg = makeMessage(1, msgBody, toAddr);
		std::string filesList = buildFilesList(files);
		msg.addAttribute(HttpMessage::attribute("FilesList", filesList));
		msg.addAttribute(HttpMessage::attribute("Type", "ListFiles"));
		sendMessage(msg, si);
		si.close();
	}
	catch (std::exception& exc)
	{
		Show::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}

/*
Builds a  filelist for the given cataegory in the server and sends a message to the client
*/
std::string MsgServer::buildFilesList(std::vector<std::string> fileList) {

	std::string fileNames;
	for (std::string file : fileList)
	{
		fileNames = fileNames + file+",";
		std::cout << "\n" << fileNames << "\n";
	}
	if (fileNames.length() > 0)
	{
		fileNames.pop_back();
	}
	else
	{
		fileNames = "No Files";
	}
	return fileNames;
}
/*
Deletes files for the given cataegory in the server and sends a message to the client
*/
void MsgServer::deleteFiles(std::string category, std::string toAddr)
{
	std::string fHtmlpath = repoPath + "PublishedFiles\\" + category + "\\";
	std::string fSourceCodepath = repoPath + "SourceCode\\" + category + "\\";

	std::vector<std::string> Htmlfiles = FileSystem::Directory::getFiles(fHtmlpath, "*.*");
	for (std::string file : Htmlfiles)
	{
		if ((file != "style.css") && (file !="hideHandler.js"))
		{
			std::remove((fHtmlpath + file).c_str());
		}
	}

	std::vector<std::string> sourcefiles = FileSystem::Directory::getFiles(fSourceCodepath, "*.*");
	for (std::string file : sourcefiles)
	{
			std::remove((fSourceCodepath + file).c_str());
	}
	std::string toadd = split(toAddr, ':').top();
	SocketSystem ss;
	SocketConnecter si;
	while (!si.connect("localhost", std::stoi(toadd)))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	HttpMessage msg;
	std::string msgBody = "List of files in the Repository deleted";
	msg = makeMessage(1, msgBody, toAddr);
	msg.addAttribute(HttpMessage::attribute("Category", category));
	msg.addAttribute(HttpMessage::attribute("Type", "Delete"));
	sendMessage(msg, si);
}
/*
sends a UPLOAD COMPLETE message to the client
*/
void MsgServer::sendUploadCompleteMessage(std::string category, std::string toAddr)
{
	std::string toadd = split(toAddr, ':').top();
	SocketSystem ss;
	SocketConnecter si;
	while (!si.connect("localhost", std::stoi(toadd)))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	HttpMessage msg;
	msg = makeMessage(1, "Upload Complete", toAddr);
	msg.addAttribute(HttpMessage::attribute("Type", "Upload"));
	sendMessage(msg, si);
	si.close();
}
/*
Starts listening for messages on a given port
*/
void MsgServer::startListener()
{
	::SetConsoleTitle(L"HttpMessage Server - Runs Forever");

	Show::attach(&std::cout);
	Show::start();
	Show::title("\n  HttpMessage Server started");

	Async::BlockingQueue<HttpMessage> msgQ;

	try
	{
		SocketSystem ss;
		SocketListener sl(portNumber, Socket::IP6);
		ClientHandler cp(msgQ,repoPath);
		sl.start(cp);
		/*
		* Since this is a server the loop below never terminates.
		* We could easily change that by sending a distinguished
		* message for shutdown.
		*/
		while (true)
		{
			HttpMessage msg = msgQ.deQ();
			processMessage(msg);
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


int main()
{

	::SetConsoleTitle(L"Server Running on Threads");

	Show::title("Demonstrating two HttpMessage Clients each running on a child thread");
	CodeAnalysis::CodeAnalysisExecutive ab;
	//ab.demo();

	MsgServer c1(8081, "..\\..\\Repository\\");

	std::thread tserver(
		[&]() { c1.startListener(); } // Start listener on the client c1
	);

	tserver.join();
	::Sleep(10000);

	//c1.execute(100, 1);

	getchar();
	//Repository\Category1
	/*char *hb = "";
	char *fpath = "..\\..\\Repository\\";
	char *options1 = "*.h";
	char *options2 = "*.cpp";
	char *options3 = "*.cs";
	char *options4 = "/m";
	char *options5 = "/f";
	char *options6 = "/r";
	CodeAnalysis::CodeAnalysisExecutive obj;
	char *b[] = { hb,fpath,options1,options2,options3,options4,options5,options6 };
	obj.ProcessCommandLine(8, b,"Category1");
	obj.startCodeAnalaysis(8, b, "Category1");*/
	//obj.demo();


	

	/*std::thread t1(
	[&]() { c1.execute(100, 20); } // 20 messages 100 millisec apart
	);*/

	/*MsgServer c2;
	std::thread t2(
	[&]() { c2.execute(120, 20); } // 20 messages 120 millisec apart
	);
	t1.join();
	t2.join();*/
}