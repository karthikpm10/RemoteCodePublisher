#pragma once
///////////////////////////////////////////////////////////////////////////////////////
// DependencyAnalysis.h - Pubishes HTML files for the code                             //
//                                                                                    //
// Karthik Palepally Muniyappa,CSE687 - Object Oriented Design, Spring 2017          // 
//                                                                                  //
////////////////////////////////////////////////////////////////////////////////////
/*
Module Operations:
==================
This module defines an CodePublisher Class
CodePublisher class defines functions to:
publish the html files for the input files specified and generate a HTML Menu page with all the published files

Public Interface:
=================
CodePublisher(DependencyTable &objDepTable, std::unordered_map<std::string, std::unordered_map<std::size_t, std::size_t>> buttonMap);
void publishFiles(std::vector<std::string> fileNames);
* Required Files:
* ---------------
*   - DependencyAnalysis.h
*   
* Build Process:
* --------------
*   devenv CodePublisher.sln /debug rebuild
*
* Maintenance History:
* --------------------
ver 0.1 : 4th April 2017
*
/*
* -
*/
#include <iostream>
#include<fstream>
#include<string>
#include<vector>
#include<stack>
#include<algorithm>
#include <sstream>
#include<Windows.h>
#include "../DependencyAnalyzer/DependencyAnalysis.h"

namespace CodeAnalysis
{
	class CodePublisher
	{
	public:
		CodePublisher(DependencyTable &objDepTable, std::unordered_map<std::string, std::unordered_map<std::size_t, std::size_t>> buttonMap,std::string publishPath);
		void publishFiles(std::vector<std::string> fileNames);
		


	private:
		std::unordered_map<std::string, std::string> htmlFilesMap;
		std::unordered_map<std::string, std::unordered_map<std::size_t, std::size_t>> buttonTableMap;
		DependencyTable &depTable;
		//"../../Repository/PublishedFiles/"
		std::string htmlFpath = "../../Repository/PublishedFiles/";
		std::string cssFpath = "style.css";
		std::string jsFpath = "hideHandler.js";
		std::vector<std::string> FilesToPublish;
		int id = 1;
		void createHTMLFile(std::string ipfilePath);
		bool checkifButtonclosed(std::unordered_map<std::size_t, std::size_t> map, size_t lineNumber);
		std::stack<std::string> split(const std::string s, char delim);
		bool writeTofile(std::string content, std::string fName);
		std::string processLine(std::string line);
		std::string generateButtonTag(std::string id);
		std::string generatePreTag(std::string id);
		std::string generateHTMLOpenningTags(std::string cssFname, std::string filename);
		std::string generateDependencyTags(std::string filename);
		void generateHtmlMenuPage(std::string opfilename);
		void openHtmlPages(std::string fpath);
		std::string getPrologue(std::string filename);
		void addFilesToPublish(std::vector<std::string> files);
		bool isHtmlFilePublished(std::string fname);
	

	};
}
