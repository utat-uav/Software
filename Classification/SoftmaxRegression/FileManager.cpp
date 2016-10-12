#include "stdafx.h"
#include "FileManager.h"


FileManager::FileManager(string folder)
{
	FILE* pipe = NULL;
	string pCmd = "dir /B /S \"" + string(folder) + "\"";
	char buf[256];

	if (NULL == (pipe = _popen(pCmd.c_str(), "rt")))
	{
		cout << "Shit" << endl;
		return;
	}

	while (!feof(pipe))
	{
		if (fgets(buf, 256, pipe) != NULL)
		{
			files.push_back(string(buf));
		}
	}

	_pclose(pipe);

	organizeFolders();
}

vector<string>* FileManager::getFiles()
{
	return &(this->files);
}

void FileManager::organizeFolders()
{
	// Get folders
	for (vector<string>::iterator it = files.begin(); it != files.end(); ++it)
	{
		it->pop_back(); // Removes \n character

		if (it->find('.') == string::npos)
		{
			// Initialize
			organizedFolders[*it] = vector<string>();
		}
		else
		{
			// Check which folder it belongs to if it isn't a folder
			for (unordered_map<string, vector<string>>::iterator folder = organizedFolders.begin(); folder != organizedFolders.end(); ++folder)
			{
				// Search *it for the folder name
				if (it->find(folder->first) != string::npos)
				{
					organizedFolders[folder->first].push_back(*it);
				}
			}
		}
	}
}
