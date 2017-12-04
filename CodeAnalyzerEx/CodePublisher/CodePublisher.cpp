#include "CodePublisher.h"
using namespace CodeAnalysis;
//constructor which accepts a reference to a DependencyTable object and a map which stores the start line and end line for global functions,methods and classes
CodeAnalysis::CodePublisher::CodePublisher(DependencyTable & objDepTable, std::unordered_map<std::string, std::unordered_map<std::size_t, std::size_t>> buttonMap,std::string publishPath):depTable(objDepTable), buttonTableMap(buttonMap)
{
	htmlFpath = htmlFpath + publishPath+"/";
}

//publishes html files for all the files in the filenames vector
void CodePublisher::publishFiles(std::vector<std::string> fileNames)
{
	std::cout << "*************   HTML files are stored in: "<< FileSystem::Path::getFullFileSpec(htmlFpath) <<"    ************\n";
	std::cout << "*************REQUIREMENT 4 : facility to expand or collapse class bodies, methods, and global functions using JavaScript and CSS properties provided, HTML files can be checked************\n";
	std::cout << "*************REQUIREMENT 5 : CSS style sheet present in : "<< FileSystem::Path::getFullFileSpec("../../PublishedCode/CSS/style.css") <<"  Provided in the html files,HTML files can be checked************\n";
	std::cout << "*************REQUIREMENT 5 : Javascript file present in : " << FileSystem::Path::getFullFileSpec("../../PublishedCode/JS/hideHandler.js") << "  Provided in the html files,HTML files can be checked************\n";
	std::cout << "*************REQUIREMENT 6 : Links to Javascript files and CSS style sheet  Provided in the html files,HTML files can be checked************\n";
	std::cout << "*************REQUIREMENT 7 : HTML5 links to dependent files with a label is embedded in the html files,HTML files can be checked************\n";
	//../../PublishedCode
	addFilesToPublish(fileNames);
	std::string fileSpec = "";
	try
	{
		std::vector<std::string>::iterator fileSpec;
		//iterating over all the files
		std::cout << "*************REQUIREMENT 3 : Publisher program that provides for creation of web pages each of which captures the content of a single C++ source code file is called on line 26 of CodePublisher.cpp************\n";
		std::cout << "\n*************************************Publishing HTML FIles******************************************\n";
		std::cout << "\n*************************************Processing******************************************\n";
		for (fileSpec = fileNames.begin(); fileSpec < fileNames.end(); fileSpec++) {
			//creating html files for the file specified by *fileSpec
			createHTMLFile(*fileSpec);
		}
		// generating the HTML menu page with all the filenames that are published
		generateHtmlMenuPage(FileSystem::Path::getFullFileSpec(htmlFpath + "menu.htm"));
	}
	catch (std::logic_error& ex)
	{
		std::cout << ex.what();
	}
}

//creates HTML file for the file specified by ipfilePath by creating the HTML tags necessary,adding comments,adding contents of the code 
//within the pre tag
//adding buttons for hiding and unhiding of class,method and global function bodies
void CodePublisher::createHTMLFile(std::string ipfilePath)
{
	std::string line;
	size_t lineCount = 1;
	std::string fileName = split(ipfilePath, '\\').top();
	std::string opfilename = htmlFpath + fileName + ".htm";
	std::ifstream ipfile(ipfilePath);
	std::ofstream opHtmlfile(opfilename);
	if (ipfile.is_open() && opHtmlfile.is_open())
	{
		//map of start line count and end line count for the global functions ,methods anad classes in the file specified by ipfilePath
		std::unordered_map<std::size_t, std::size_t> butMap= buttonTableMap[ipfilePath];
		opHtmlfile << getPrologue(split(FileSystem::Path::getFullFileSpec(opfilename), '\\').top());
		opHtmlfile << generateHTMLOpenningTags(cssFpath, fileName) + generateDependencyTags(ipfilePath) + "<pre>\n";
		while (getline(ipfile, line))
		{
			if (checkifButtonclosed(butMap, lineCount))
			{
				opHtmlfile << "</pre>" + processLine(line) + " " + "\n";
				butMap.erase(lineCount);
			}
			if (butMap.find(lineCount) != butMap.end())
			{
				id++;
				if((butMap[lineCount]!=lineCount+1))
				opHtmlfile << processLine(line) + generateButtonTag(std::to_string(id)) + generatePreTag(std::to_string(id))+ " " + "\n";
				else
				{
					opHtmlfile << processLine(line) + " " + "\n";
					butMap.erase(lineCount);	
				}
			}
			else
			{
				if(!checkifButtonclosed(butMap, lineCount))
				opHtmlfile << processLine(line) + " " + "\n";
			}
			lineCount++;
		}
		opHtmlfile<< "</pre>\n<script src=\"" + jsFpath + "\"></script>\n</body>\n</html>";
		ipfile.close();
		opHtmlfile.close();
		htmlFilesMap[split(FileSystem::Path::getFullFileSpec(opfilename), '\\').top()] = opfilename;
	}
	else
		std::cout << "Unable to open file";
}
//checks if in the lineNumber a class or a global function or a method closes
bool CodeAnalysis::CodePublisher::checkifButtonclosed(std::unordered_map<std::size_t, std::size_t> map, size_t lineNumber)
{
	for (auto item : map)
	{
		if (item.second == lineNumber)
			return true;
	}
	return false;
}

//splits the string s on the occurence of delim character and returns a stack with the split strings
std::stack<std::string> CodePublisher::split(const std::string s, char delim) {
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

// writes the content to the file fName
bool CodeAnalysis::CodePublisher::writeTofile(std::string content, std::string fName)
{
	try
	{
		std::ofstream fstream(fName);
		if (fstream.is_open())
		{
			fstream << content;
			fstream.close();
		}
		else
		{
			std::cout << "Unable to open file";
			return false;
		}
	}
	catch (std::exception& ex)
	{
		std::cout << "Exception " << ex.what();
		return false;
	}
	return true;
}
//replaces oocurences of < and > with &lt; and &gt; in the line and returns the new string
std::string CodePublisher::processLine(std::string line)
{
	std::string processedLine;
	for (char& c : line) {
		std::string newstr;
		if (c == '<')
		{
			newstr = "&lt;";

		}
		else if (c == '>')
		{
			newstr = "&gt;";
		}
		else if (c == '{')
		{
			newstr = c;
		}
		else if(c=='}')
		{
			newstr = "}";
		}
		else
		{
			newstr = c;
		}
		processedLine = processedLine + newstr;
	}
	return processedLine;
}

// generates a string of the html tags necessary for adding a button in an html file with the given id. and returns this string
std::string CodePublisher::generateButtonTag(std::string id)
{
	std::string buttonTag = "<button onclick=\"myFunction(this)\" id=\"button" + id + "\">-</button>";
	return buttonTag;
}
//generates a string of the html tags necessary for adding a pre tag in an html file with the given id and returns this string
std::string CodePublisher::generatePreTag(std::string id)
{
	std::string preTag="<pre class=\"showElem\" id=\"button"+id+"_pre\">";
		return preTag;
}
// generates a string of the html tags necessary for the starting of an html file 
//and sets the reference of the css file, and adds the filename in the header
std::string CodePublisher::generateHTMLOpenningTags(std::string cssFname, std::string filename)
{
	std::string openingTags = "<html>\n<head>\n<link rel=\"stylesheet\" href=\""+cssFname+"\">\n</head>\n<body>\n<h3>"+filename+"</h3>\n";
	return openingTags;
}
//generates href tags necessaary for an HTML file, it finds all the files that the given filename depends on from the dependency table
//and adds the refernces to these files in the html page, returns this string representation of href tags
std::string CodeAnalysis::CodePublisher::generateDependencyTags(std::string filename)
{
	std::string depTags = "<hr />\n<div class=\"indent\">\n<h4>Dependencies:</h4>\n";
	Element<std::string> record = depTable.getRecord(filename);
	std::stack<std::string> fn = split(split(filename, '\\').top(),'.');
	std::vector<std::string> children = record.children;
	depTags = depTags + "<ul>\n";
	if (fn.top() == "h")
	{
		fn.pop();
		if(isHtmlFilePublished(fn.top() + ".cpp" + ".htm"))
		depTags = depTags + "<li><a href=\"" + fn.top() + ".cpp" + ".htm\">" + fn.top() + ".cpp" + "</a></li>\n";
	}
	for (string child : children) {
		if (!(filename == child))
		{
			std::stack<std::string> depfileNamesSplit = split(child, '\\');
			std::string depfileName = depfileNamesSplit.top();
			depTags = depTags + "<li><a href=\""+depfileName+".htm\">"+ depfileName +"</a></li>\n";
		}
	}
	depTags = depTags + "</ul>\n";
	depTags = depTags + "</div>\n<hr />\n";
	return depTags;
}

//creates a html menu page with refernces to all the html files which are published
void CodeAnalysis::CodePublisher::generateHtmlMenuPage(std::string opfilename)
{
	std::ofstream opHtmlfile(opfilename);
	std::string fileName = split(opfilename, '\\').top();
	if (opHtmlfile.is_open())
	{
		opHtmlfile << getPrologue(fileName);
		opHtmlfile << generateHTMLOpenningTags(cssFpath, fileName) + "<hr />\n<div class=\"indent\">\n<h4>PublishedFiles:</h4>\n<hr />\n";
		opHtmlfile << "<ul>\n";
		for (auto file : htmlFilesMap)
		{
			opHtmlfile << "<li><a href=\"" + file.first + "\">" + file.first + "</a></li>\n";
		}
		opHtmlfile << "</ul>\n";
		opHtmlfile << "<hr />\n";
		opHtmlfile << "</div>\n<hr/>\n";
		opHtmlfile << "\n</body>\n</html>";
		opHtmlfile.close();
		//openHtmlPages(opfilename);
	}
	else
	{
		std::cout << "Unable to open file";
	}
}

//opens the file specified by fpath in the browser
void CodeAnalysis::CodePublisher::openHtmlPages(std::string fpath)
{
	std::string path = "file:///" + FileSystem::Path::getFullFileSpec(fpath);
	std::wstring spath = std::wstring(path.begin(), path.end());
	LPCWSTR swpath = spath.c_str();
	ShellExecute(NULL, L"open", swpath, NULL, NULL, SW_SHOWNORMAL);
	//system("PAUSE");
}
//generates a Prologue for a given filename
std::string CodeAnalysis::CodePublisher::getPrologue(std::string filename)
{
	time_t timeobj;
	time(&timeobj);

	struct tm  tstruct = {};
	char buf[80];
	localtime_s(&tstruct, &timeobj);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	std::string datestr(buf);
	std::string comments = "<!----------------------------------------------------------------------------\n";
	comments = comments+"   " + filename+"  -Published HTML File\n";
	comments = comments+"   " + "Published on : "+ datestr +"\n";
	comments = comments + "   Author:Karthik Palepally Muniyappa  \n";
	comments = comments + "----------------------------------------------------------------------------->\n";
	return comments;
}
// adds the list of files whose html files have to be generated
void CodeAnalysis::CodePublisher::addFilesToPublish(std::vector<std::string> files)
{
	for (auto file : files)
	{
		FilesToPublish.push_back(split(file, '\\').top()+".htm");
	}
}
//checks if the html file specified by fname should be published
bool CodeAnalysis::CodePublisher::isHtmlFilePublished(std::string fname)
{
	for (auto file : FilesToPublish)
	{
		if (file == fname)
			return true;
	}
	return false;
}

#ifdef DEBUGCODEPUBLISHER
int main(int argc, char* argv[])
{
	std::cout << "here";
	CodePublisher obj;
	std::vector<std::string> fileNames;
	fileNames.push_back("../test3/typetable.h")
	fileNames.push_back("../test3/typetable.cpp")
	obj.publishFiles(std::vector<std::string> fileNames);
	getchar();

}
#endif




