#include"Typtable.h"
using namespace CodeAnalysis;
//constructor
TypeTable::TypeTable() :
	ASTref_(Repository::getInstance()->AST()) {
	
}

//checks the type of the node
bool TypeTable::checkType(ASTNode* pNode)
{
	static std::string toDisplay[] = {
		"class", "struct", "enum"
	};
	for (std::string type : toDisplay)
	{
		if (pNode->type_ == type)
			return true;
	}
	return false;
}

// stores the global function into a map
void TypeTable::storeGlobalFunction(ASTNode* pNode)
{
	std::vector<std::string> filenames;
	if (globalFuncMap.find(pNode->name_) == globalFuncMap.end())
	{
		filenames.push_back(pNode->path_);
	}
	else
	{
		filenames = globalFuncMap[pNode->name_];
		filenames.push_back(pNode->path_);
	}
	globalFuncMap[pNode->name_] = filenames;
}
//performs dfs and stores type info
void TypeTable::typeAnalysis(ASTNode* pNode)
{
	insertIntoButtonMap(pNode);
	static std::string path = "";
	if (pNode->path_ != path)
	{
		//std::cout << "\n    -- " << pNode->path_ << "\\" << pNode->package_;
		path = pNode->path_;
	}
	if (pNode->type_ == "namespace")
	{
		nameSpaceStack.push(pNode->name_);
	}

	if (checkType(pNode))
	{
		TypeInfo mapelem;
		mapelem.setType(pNode->type_);
		mapelem.setFile(pNode->path_);
		mapelem.setNamespace(nameSpaceStack.top());
		std::vector<TypeInfo> vectorMap;
		if (typeTableMap.find(pNode->name_) == typeTableMap.end())
		{
			vectorMap.push_back(mapelem);
		}
		else
		{
			vectorMap = typeTableMap[pNode->name_];
			vectorMap.push_back(mapelem);
		}
		typeTableMap[pNode->name_] = vectorMap;
	}
	for (auto pChild : pNode->children_)
	{
		typeAnalysis(pChild);
	}
	if (pNode->type_ == "namespace") {
		nameSpaceStack.pop();
	}
}
//initates type analysis
void TypeTable::doTypeAnal()
{
	ASTNode* pRoot = ASTref_.root();
	typeAnalysis(pRoot);
	extractGlobalFunctions(Repository::getInstance()->getGlobalScope());
}

void TypeTable::extractGlobalFunctions(ASTNode * globalNode) {
	for (auto node : globalNode->children_) {
		if (node->type_ == "function") {
			if ((node->name_ != "main") && (node->name_ != "void"))
				storeGlobalFunction(node);
			//std::cout<< "found global function: " << node->name_<<"\n";
			// do stuff to the found global function
		}
	}
}
// displays the type table
void TypeTable::showTypeTable()
{
	std::cout << "\n\n\n************************************TypeTable*****************************************\n";
	std::cout << std::setw(20) << "TypeName" << std::setw(15) << "Type" << std::setw(25) << "Namespace" << "Filename";
	std::cout << "\n--------------------------------------------------------------------------------------------------------------\n";
	for (auto item = typeTableMap.begin(); item != typeTableMap.end(); ++item)
	{
		std::string typeName = item->first;
		std::vector<TypeInfo> filesVector = item->second;


		for (size_t it = 0; it < filesVector.size(); it++) {
			TypeInfo map = filesVector[it];
			std::cout << std::setw(20) << typeName << std::setw(15) << map.getType() << std::setw(25) << map.getNamespace() << map.getFile();
			std::cout << "\n";
		}
	}
	std::cout << "\n\n";
	std::cout << "\n\n\n************************************GlobalFunctionTable*****************************************\n";
	std::cout << std::setw(20) << "TypeName" << std::setw(17) << "Type" << std::setw(25) << "Namespace" << "Filename";
	std::cout << "\n--------------------------------------------------------------------------------------------------------------\n";
	for (auto globalFunc : globalFuncMap)
	{
		std::cout << std::setw(20) << globalFunc.first << std::setw(17) << "GlobalFunction" << std::setw(25) << "GlobalNamespace";
		for (std::string fname : globalFunc.second)
		{
			std::cout << fname << "  ";
		}
		std::cout << "\n";
	}
	std::cout << "\n\n";
}

//returns the filenames where the token is defined
std::vector<std::string> TypeTable::getTypeDefinationFnames(std::string typeName)
{
	if (typeTableMap.find(typeName) != typeTableMap.end())
	{
		std::vector<std::string> files;
		std::vector<TypeInfo> filesVector = typeTableMap[typeName];
		//std::vector<stringMap>::iterator it;

		for (size_t it = 0; it < filesVector.size(); it++) {
			TypeInfo map = filesVector[it];
			files.push_back(map.getFile());
		}
		return files;
	}
	else if (globalFuncMap.find(typeName) != globalFuncMap.end())
	{
		return globalFuncMap[typeName];
	}
	return std::vector<std::string>();
}
//returns a map which stores the start line and end line for classes,global functions and methods for all the files
std::unordered_map<std::string, std::unordered_map<std::size_t, std::size_t>> CodeAnalysis::TypeTable::getButtonMap()
{
	return buttonTableMap;
}
//checks if a node is a global function
void TypeTable::chekIfGlobalFun(ASTNode * pNode)
{
	//if (((checkType(pNode)) && (pNode->children_.size() > 0)))
	if ((!doNotInclFun) && checkType(pNode))
	{
		doNotInclFun = true;
		parentSetter = pNode->name_;
	}
}
//checks if a node is a global function with children
void TypeTable::chekIfGlobalFunWithChild(ASTNode * pNode)
{
	if ((!doNotInclFun) && ((pNode->type_ == "function") && ((pNode->children_.size() > 0))))
	{
		doNotInclFun = true;
		globFunWchild = true;
		parentSetter = pNode->name_;
	}
}
//populats a map with  the start line and end line for classes,global functions and methods for  the file which pNode represents
void CodeAnalysis::TypeTable::insertIntoButtonMap(ASTNode * pNode)
{
	if (pNode->startLineCount_ != pNode->endLineCount_)
	{
		static std::string toDisplay[] = {
			"class", "function"
		};
		for (std::string type : toDisplay)
		{
			if (pNode->type_ == type)
			{
				std::unordered_map<std::size_t, std::size_t> map;
				if (buttonTableMap.find(pNode->path_) != buttonTableMap.end())
				{
					map = buttonTableMap[pNode->path_];
					map[pNode->startLineCount_] = pNode->endLineCount_;
				}
				else
				{
					map[pNode->startLineCount_] = pNode->endLineCount_;
				}
				buttonTableMap[pNode->path_] = map;
			}
		}
	}
}
#ifdef DEBUGTypTable
int main()
{
	TypeTable obj;
	obj.doTypeAnal();
	obj.showTypeTable();
	obj.getTypeDefinationFnames("NosqlDB");
	obj.getButtonMap();
	return 0;
}
#endif
