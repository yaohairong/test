#include "io.h"
#include "direct.h"

#include <iostream>
using std::cin;
using std::cout;
using std::endl;
using std::cerr;


#include <string>
using std::string;


int main()
{
	string dir;
	cout << "Input the name of directory: ";
	cin >> dir;

	if (_access(dir.c_str(), 06) == -1)
	{
		cerr << "error: directory does not exist." << endl;
		exit(-1);
	}

	if (dir.at(dir.length() - 1) != '\\')
	{
		dir += '\\';
	}

	if (_chdir(dir.c_str()) != 0)
	{
		cerr << "error: function _chdirPath(dir + fileinfo.name);

		cout << "name: " << filePath << 
	}
}
