#pragma once
#include "stdafx.h"

class FileManager
{
private:
	vector<string> files;
	void organizeFolders();

public:
	FileManager(string folder);

	vector<string>* getFiles();
	unordered_map<string, vector<string>> organizedFolders;
};

