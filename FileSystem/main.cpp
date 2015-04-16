#include <iostream>
#include <string>
#include "FileSystem.h"
using namespace std;

char* filepath = "D:\\myfs.img";
myFileSystem* myFS=NULL;
int currenID = 0;
void parse(char* input)
{
		if (strcmp(input, "mount") == 0)
		{
			if (myFS==NULL)
			{
				myFS = new myFileSystem(filepath);
				if(myFS->mounted) 
					cout << "\tmounted" << endl;
				else
					cout << "\tnot mounted" << endl;
			}
		}
		else if (strcmp(input, "pwd") == 0)
		{
			if (myFS!=NULL)
			{
				myFS->pwd();
			}
		}
		else if (strcmp(input, "cd") == 0)
		{
			if (myFS != NULL)
			{
				string str;
				cin >> str;
				int temp = currenID;
				currenID = myFS->cd((char *)str.c_str(),currenID);
				cout << "cd: " << currenID << endl;
				if (currenID == -1) currenID = temp;
				
			}
		}
		else if (strcmp(input, "mkdir") == 0)
		{
			if (myFS != NULL)
			{
				char name[64];
				cin >> name;
				myFS->mkdir(name);
			}
		}
		else if (strcmp(input, "symlink") == 0)
		{
			if (myFS != NULL)
			{
				char name1[64];
				cin >> name1;
				char name2[64];
				cin >> name2;
				myFS->symlink(name1,name2);
			}
		}
		else if (strcmp(input, "rmdir") == 0)
		{
			if (myFS != NULL)
			{
				char name[64];
				cin >> name;
				myFS->rmdir(name);
			}
		}
		else if (strcmp(input, "unmount") == 0)
		{
			if (myFS->unmount())
			{
				myFS = NULL;
				cout << "\tunmounted" << endl;
			}
		}
		else if (strcmp(input, "ls") == 0)
		{
			if(myFS!=NULL) myFS->ls();
			else cout << "file system not mounted" << endl;
		}
		else if (strcmp(input, "create") == 0)
		{
			char name[64] = "";
			cin >> name;
			if (myFS != NULL)
			{
				if (myFS->create(name))
					cout << "\tcreated " << name << endl;
				else
					cout << "\tnot created " << name << endl;
			}
			else cout << "file system not mounted" << endl;
			
		}
		else if (strcmp(input, "open") == 0)
		{
			char name[128] = "";
			cin >> name;
			if (myFS != NULL) myFS->open(name);
			else cout << "file system not mounted" << endl; 
		}
		else if (strcmp(input, "close") == 0)
		{
			int i = -1;
			cin >> i;
			if (cin.fail()){
				cin.clear();
				cout << "\t** not a number" << endl;
			}
			else
			{
				if (myFS != NULL) 
					if (myFS->close(i))
						cout << "\tclosed " << i << endl;
				else cout << "file system not mounted" << endl;
				
			}

		}
		else if (strcmp(input, "filestat") == 0)
		{
			int i = -1;
			cin >> i;
			if (cin.fail()){
				cin.clear();
				cout << "\t** not a number" << endl;
			}
			else
			{
				if (myFS != NULL) myFS->filestat(i);
				else cout << "file system not mounted" << endl;
			}
			
		}
		else if (strcmp(input, "read") == 0)
		{
			int i = -1;
			int ofset = 0;
			int size = 0;
			cin >> i >> ofset >> size;
			if (cin.fail()){
				cin.clear();
				cout << "\t** not a number" << endl;
			}
			else
			{
				if (myFS != NULL)
				{
					myFS->read(i, ofset, size);
				}
				else cout << "file system not mounted" << endl;
				
			}
		}
		else if (strcmp(input, "write") == 0)
		{
			int i = -1;
			int ofset = 0;
			cin >> i >> ofset;
			string str;
			std::getline(std::cin, str);
			if (cin.fail()){
				cin.clear();
				cout << "\t** not a number" << endl;
			}
			else
			{
				if (myFS != NULL)
				{
					myFS->write(i, ofset, str.size(), str.c_str());
					
				}
				else cout << "file system not mounted" << endl;
			}
		}
		else if (strcmp(input, "format") == 0)
		{
			if (myFS != NULL)
			{
				myFS->clear();
				myFS->unmount();
				myFS = new myFileSystem(filepath);
			}
			else cout << "file system not mounted" << endl;
		}

		else if (strcmp(input, "link") == 0)
		{
			char name1[128] = "";
			char name2[128] = "";
			cin >> name1 >> name2;
			if (myFS != NULL)
			{
				if (myFS->link(name1, name2))
					cout << "\tlinked " << name1 << " to " << name2 << endl;
			}
			else cout << "file system not mounted" << endl;
		}
		else if (strcmp(input, "unlink") == 0)
		{
			char name[128] = "";
			cin >> name;
			if (myFS != NULL)
			{
				if (myFS->unlink(name))
					cout << "\tunlinked " << name <<" "<< endl;
			}
			else cout << "file system not mounted" << endl;
			
		}
		else if (strcmp(input, "truncate") == 0)
		{
			char name[128] = "";
			int size = 0;
			cin >> name >> size;
			if (cin.fail()){
				cin.clear();
				cout << "\t** not a number" << endl;
			}
			else
			if (myFS != NULL)
			{
				myFS->truncate(name, size);
			}
			else cout << "file system not mounted" << endl;
			
		}
		else if (strcmp(input, "clear") == 0)
		{
			system("cls");
		}
		else if (strcmp(input, "help") == 0)
		{
			cout << "\tclear \t- clear the screen" << endl;
			cout << "\tformat \t- formatting file system" << endl;
			cout << "\tls \t- file list" << endl;
			cout << "\tmount \t- mount file system" << endl; 
			cout << "\tunmount \t- unmount file system" << endl;
			cout << "\tfilestat \'id\' \t- descriptor info" << endl;
			cout << "\tcreate \'name\' \t- create file" << endl;
			cout << "\topen \'name\' \t- open file" << endl;
			cout << "\tclose \'fd\' \t- close file" << endl;
			cout << "\ttrancate \'name\' \'size\' \t- change size" << endl;
			cout << "\tread \'fd\' \'offset\' \'size\' \t- read data" << endl;
			cout << "\twrite \'fd\' \'offset\' \'data\' \t- write data" << endl;
			cout << "\tlink \'name1\' \'name2\' \t- create link" << endl;
			cout << "\tunlink \'name\' \t- remove link" << endl;\
		}
		else
		{
			cout << "\t* incorect command" << endl;
		}
}
void main()
{
	char input[128]="mount";
	while (strcmp(input,"exit") != 0)
	{
		cout << ">> ";
		cin >> input;
		parse(input);
	}
	delete(myFS);
}