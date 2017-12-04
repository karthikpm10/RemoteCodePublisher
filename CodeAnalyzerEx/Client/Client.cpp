
#include "Client.h"
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
/*
reads the messages from the socket and builds an HttpMessage,
if the mesage is for a file upload then reads the files and stores it in the path
*/
HttpMessage ClientHandler::readMessage(Socket& socket)
{	connectionClosed_ = false;
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

			readFile(category + "\\" + filename, contentSize, socket);
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
	std::string fqname = repoPath + "PublishedFiles\\" + filename;
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
checks for the delimiter in the given string aand splits them and returns a vector
*/

std::vector<std::string> MsgClient::splitVect(const std::string & s, char delim)
{
	std::stringstream ss;
	ss.str(s);
	std::string item;
	std::vector<std::string> elems;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}
/*
listens for messagaes on a given port
*/
HttpMessage MsgClient::listener()
{
	Async::BlockingQueue<HttpMessage> msgQ;
	HttpMessage msg;
	try
	{
		std::string type;


		SocketSystem ss;
		SocketListener sl(portNumber, Socket::IP6);
		ClientHandler cp(msgQ, repoPath);
		sl.start(cp);
		while (true)
		{
			msg = msgQ.deQ();
			type = msg.findValue("Type");
			if ((type == "Publish") || (type == "Download") || (type == "Delete") || (type == "ListFiles") || (type == "Upload"))
			{
				break;
			}
		}
		sl.close();
	}
	catch (std::exception& exc)
	{
		Show::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}

	return msg;
}

/*
Opens the file in the browser
*/
void MsgClient::openHtmlPages(std::string fpath)
{
	
	fpath =repoPath + "PublishedFiles\\" + fpath;
	
		std::string path = "file:///" + FileSystem::Path::getFullFileSpec(fpath);
		std::wstring spath = std::wstring(path.begin(), path.end());
		LPCWSTR swpath = spath.c_str();
		ShellExecute(NULL, L"open", swpath, NULL, NULL, SW_SHOWNORMAL);
		//system("PAUSE");
	
}
/*
build a HttpMessage and send the message to the given address
*/
void MsgClient::sendCustomMessage(std::string type, std::string category, std::string toAddress, std::string msgBody)
{
	std::string toadd = split(toAddress, ':').top();
	SocketSystem ss;
	SocketConnecter si;
	while (!si.connect("localhost", std::stoi(toadd)))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	HttpMessage msg = makeMessage(1, msgBody, toAddress);
	msg.addAttribute(HttpMessage::attribute("Category", category));
	msg.addAttribute(HttpMessage::attribute("Type", type));
	sendMessage(msg, si);

	si.close();
}
/*
maps integer number to the category
*/
std::string MsgClient::getCategory(int category)
{
	switch (category) {
	case 1:
		return "Category1";
		break;
	case 2:
		return "Category2";
		break;
	case 3:
		return "Category3";
		break;
	default:
		return "none";
		break;
	}
}
/*
send the list of files to the server

*/

void MsgClient::sendFilesToServer(std::string category, std::string toAddr,std::vector<std::string> files)
{
	//std::string fpath = repoPath + "SourceFiles\\" + category + "\\";
	std::string toadd = split(toAddr, ':').top();
	SocketSystem ss;
	SocketConnecter si;
	while (!si.connect("localhost", std::stoi(toadd)))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	

	//std::vector<std::string> files = FileSystem::Directory::getFiles(fpath, "*.*");
	for (size_t i = 0; i < files.size(); ++i)
	{
		Show::write("\n\n  sending file " + files[i]);
	//	std::string filPathForDownload = fpath + files[i];
		sendFile(files[i], si, toAddr, category);
	}

	HttpMessage terminateMsg;
	terminateMsg = makeMessage(1, "Terminating Upload", toAddr);
	terminateMsg.addAttribute(HttpMessage::Attribute("Type", "Upload"));
	terminateMsg.addAttribute(HttpMessage::Attribute("status", "complete"));
	terminateMsg.addAttribute(HttpMessage::attribute("Category", category));
	sendMessage(terminateMsg, si);
	si.close();
}
/*
Builds a HttpMessage with the given parameters

*/
HttpMessage MsgClient::makeMessage(size_t n, const std::string& body, const EndPoint& ep)
{
	HttpMessage msg;
	HttpMessage::Attribute attrib;
	EndPoint myEndPoint = "localhost:" + std::to_string(portNumber);  
																	  
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

void MsgClient::sendMessage(HttpMessage& msg, Socket& socket)
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
bool MsgClient::sendFile(const std::string& filename, Socket& socket, std::string toAddress, std::string category)
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
	msg.addAttribute(HttpMessage::Attribute("file", split(fname, '\\').top()));
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
/*splits the string on the given delimiter and return the stack
*/
std::stack<std::string> MsgClient::split(const std::string s, char delim)
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
Starts listening for messages on a given port
*/
void MsgClient::startListener(int port)
{
	::SetConsoleTitle(L"HttpMessage Client - Runs Forever");

	Show::attach(&std::cout);
	Show::start();
	Show::title("\n  HttpMessage Server started");

	Async::BlockingQueue<HttpMessage> msgQ;

	try
	{
		SocketSystem ss;
		SocketListener sl(port, Socket::IP6);
		ClientHandler cp(msgQ, repoPath);
		sl.start(cp);
		/*
		* Since this is a server the loop below never terminates.
		* We could easily change that by sending a distinguished
		* message for shutdown.
		*/
		while (true)
		{
			HttpMessage msg = msgQ.deQ();
		}
	}
	catch (std::exception& exc)
	{
		Show::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}
/*
Connects to the server and uploads the files to it, and listen for a message from the server
*/
std::string MsgClient::uploadFiles(std::string files, int Category)
{
	std::cout << "*************REQUIREMENT 5 : Invoking Client Program That Provides Functionality To Upload Files************\n";
	//filestoOpen = files;
	std::cout << "\n******************************** Sending Upload Request from Client *********************************\n";
	std::vector <std::string> filesVector = splitVect(files, ',');
	std::string category = getCategory(Category);
	sendFilesToServer(category, serverAddress, filesVector);
	HttpMessage message = listener();
	std::cout << "\n**********************Received Message From Server*********************\n" << message.bodyString()<<"\n*********************\n";
	return message.bodyString();
}
/*
Connects to the server and Send a delete request to it, and listen for a message from the server
*/
std::string MsgClient::deleteFiles(int Category)
{
	std::cout << "\n******************************** Sending Delete Request from Client *********************************\n";
	SocketSystem ss;
	SocketConnecter si;
	std::string toadd = split(serverAddress, ':').top();
	std::string category = getCategory(Category);
	while (!si.connect("localhost", std::stoi(toadd)))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	HttpMessage msg = makeMessage(1, "Delete Files", serverAddress);
	msg.addAttribute(HttpMessage::Attribute("Type", "Delete"));
	msg.addAttribute(HttpMessage::attribute("Category", category));
	sendMessage(msg, si);
	si.close();
	HttpMessage message = listener();
	std::cout << "\n**********************Received Message From Server*********************\n" << message.bodyString() << "\n*********************\n";
	return message.bodyString();
}
/*
Connects to the server and Send a publish request to it, and listen for a message from the server
*/
std::string MsgClient::publishFiles(int Category)
{
	std::cout << "\n******************************** Sending Publish Request from Client *********************************\n";
	SocketSystem ss;
	SocketConnecter si;
	std::string toadd = split(serverAddress, ':').top();
	std::string category = getCategory(Category);
	while (!si.connect("localhost", std::stoi(toadd)))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	HttpMessage msg = makeMessage(1, "Publish Files", serverAddress);
	msg.addAttribute(HttpMessage::Attribute("Type", "Publish"));
	msg.addAttribute(HttpMessage::attribute("Category", category));
	sendMessage(msg, si);
	si.close();
	HttpMessage message = listener();
	std::cout << "\n**********************Received Message From Server*********************\n" << message.bodyString() << "\n*********************\n";
	return message.bodyString();
}
/*
Connects to the server and Send a List Files request to it, and listen for a message from the server
*/
std::string MsgClient::listFiles(int Category)
{
	std::cout << "*************REQUIREMENT 5 : Invoking Client Program That Provides Functionality To View Repository Contents************\n";
	std::cout << "\n******************************** Sending List Files Request from Client *********************************\n";
	SocketSystem ss;
	SocketConnecter si;
	std::string toadd = split(serverAddress, ':').top();
	std::string category = getCategory(Category);
	while (!si.connect("localhost", std::stoi(toadd)))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	HttpMessage msg = makeMessage(1, "List Files", serverAddress);
	msg.addAttribute(HttpMessage::Attribute("Type", "ListFiles"));
	msg.addAttribute(HttpMessage::attribute("Category", category));
	sendMessage(msg, si);
	si.close();
	HttpMessage message = listener();
	std::cout << "\n**********************Received Message From Server*********************\n" << message.findValue("FilesList") << "\n*********************\n";
	//std::cout << "\n\n" << message.findValue("FilesList");
	return "success,"+message.findValue("FilesList");
}
/*
Connects to the server and Send a Download request to it, and listen for a message from the server
*/
std::string MsgClient::downloadFiles(int Category, std::string fileName)
{
	std::cout << "\n******************************** Sending Download Files Request from Client *********************************\n";
	SocketSystem ss;
	SocketConnecter si;
	std::string toadd = split(serverAddress, ':').top();
	std::string category = getCategory(Category);
	while (!si.connect("localhost", std::stoi(toadd)))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	HttpMessage msg = makeMessage(1, "Download files", serverAddress);
	msg.addAttribute(HttpMessage::Attribute("Type", "Download"));
	msg.addAttribute(HttpMessage::attribute("Category", category));
	msg.addAttribute(HttpMessage::attribute("FileName", fileName));

	sendMessage(msg, si);
	si.close();
	HttpMessage message = listener();
	std::cout << "\n**********************Received Message From Server*********************\n" << message.bodyString() << "\n*********************\n";
	openHtmlPages(category + "\\" + fileName);
	return message.bodyString();
}

#ifdef DEBUG_MI
int main()
{
	::SetConsoleTitle(L"Clients Running on Threads");

	Show::title("Demonstrating two HttpMessage Clients each running on a child thread");
	std::vector<std::string> vect;

	MsgClient c1(8080,"..\\..\\ClientRepository\\");
	c1.uploadFiles("",1);
	::Sleep(100);
	c1.publishFiles(1);
	::Sleep(100);
	c1.listFiles(1);
	::Sleep(100);
	c1.downloadFiles(1,"XmlElement.h.htm");
	
}
#endif // DEBUG