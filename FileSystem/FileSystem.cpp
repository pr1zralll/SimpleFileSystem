#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include "FileSystem.h"
#include <iostream>
#define cout std::cout
using namespace std;

myFileSystem::myFileSystem(char* file_path) 
{
	file.open(file_path, std::fstream::in | std::fstream::out | std::fstream::binary);
	if (!file){ cout << "can't read the file" << endl; return; }
	// get length of file:
	file.seekg(0,ios::end);//The function clears the eofbit flag, if set before the call.
	size = file.tellg();//length
	file.seekg(0, ios::beg);
	//clear();
	
	mounted = true;
	block_count = (int)(size / block_size);
	mask_size_block = block_count / 8 / block_size + (block_count / 8 % block_size? 1 : 0);
	descriptor_count = 128;
	memfd_size = descriptor_count * 3;//*************
	memfd = new int[memfd_size];
	for (size_t i = 0; i < memfd_size; i++)
	{
		memfd[i] = 0;
	}
	max_file_size = block_size*blocks_in_descriptor;
	fileName_blocks_count = sizeof(fileName)*(descriptor_count) / block_size + ((sizeof(fileName)*(descriptor_count)) % block_size ? 1 : 0);
	descriptor_size_block = sizeof(descriptor)*(descriptor_count) / block_size + ((sizeof(descriptor)*(descriptor_count)) % block_size?1:0);
	fn_start = mask_size_block*block_size + descriptor_count*sizeof(descriptor);
	// is file system available?
	if (file.get() == 0)
		create_fs();
	cout << "\tsize: " << size << endl;
}
void myFileSystem::create_fs()
{
	cout << "\tcreating file system..." << endl;
	//use blocks for mask
	for (u_int64 i = 0; i < mask_size_block; i++)
	{
		use_block(i);
	}
	//use blocks for descriptors
	for (u_int64 i = mask_size_block; i < mask_size_block + descriptor_size_block; i++)
	{
		use_block(i);
	}
	//descriptors init
	descriptor ds;
	for (size_t i = 0; i < blocks_in_descriptor; i++)
	{
		ds.blocks[i] = 0;
	}
	for (u_int64 i = 0; i < descriptor_count; i++)
	{
		write_descriptor(i,ds);
	}
	//root
	int c = 0;
	descriptor dr = *get_descriptor_by_ID(0);
	dr.atr = 1;
	dr.links = 1;
	write_descriptor(0, dr);
	//init file names
	fileName fn;
	strcpy(fn.name, "");
	fn.id = 0;
	for (size_t i = 0; i < descriptor_count; i++)
	{
		write_fileName(fn,i);
	}
	strcpy(fn.name, "root");
	write_fileName(fn,0);
}
void myFileSystem::write_descriptor(u_int64 id, descriptor ds)
{
	u_int64 byte = mask_size_block*block_size + id*sizeof(descriptor);
	file.seekp(byte, ios::beg);
	file.write((char*)&ds,sizeof(descriptor));
}
void myFileSystem::use_block(u_int64 i)
{
	u_int64 byte = i / 8;
	u_int64 bit = i % 8;
	file.seekg(byte,ios::beg);
	char c;
	file.get(c);
	c |= 1 << bit;
	file.seekp(byte,ios::beg);
	file.put(c);
}
void myFileSystem::clear()
{
	file.seekp(0, ios::beg);
	file.put(0);
	file.seekg(0, ios::beg);
	float cur_block = 0;
	float all_blocks = block_count;
	int percent = 0;
	for (u_int64 i = 0; i < block_size*block_count; i++)
	{
		file.seekp(i, ios::beg);
		file.put(0);
		file.seekg(0, ios::beg);
		cur_block = (int) (i / block_size);
		if ((int)((cur_block / all_blocks) * 100) > percent)
		{
			system("cls");
			percent = (int)((cur_block / all_blocks) * 100);
			cout << "formated : " << percent + 1 << "%" << endl;
		}
	}
	cout << "formating is completed." << endl;
	file.seekg(0, ios::beg);
}
bool myFileSystem::unmount()
{
	if (mounted){
		mounted = false;
		file.close();
		return true;
	}
	else return false;
}
void myFileSystem::filestat(int id)
{
	if (id >= descriptor_count)
		return;
	descriptor d = *get_descriptor_by_ID(id);
	cout << "\tsize: " << d.size << endl;
	cout << "\tatr: " << d.atr << endl;
	cout << "\tlinks: " << d.links << endl;
	cout << "\tblocks: ";
	for (int i = 0; i < blocks_in_descriptor; i++)
	{
		if (d.blocks[i] != 0)
			cout << d.blocks[i] << " ";
	}
	cout << endl;
}
void myFileSystem::ls() 
{
	fileName f;
	file.seekg(fn_start, ios::beg);
	for (size_t i = 0; i < descriptor_count; i++)
	{
		file.read((char*)&f, sizeof(fileName));
		//out
		if (f.id == 0) continue;
		cout << "\tid: " << f.id;
		cout << "\tname: " << f.name << endl;
	}
}
bool myFileSystem::create(char* name)
{
	//file exist
	fileName fi;
	descriptor c = * get_descriptor_by_ID(currID);
	for (size_t i = 1; i < blocks_in_descriptor; i++)
	{
		if (c.blocks[i] != 0)
		{
			for (size_t j = 1; j < block_count-1; j++)
			{
				fi = *get_fileName_by_ID(j);
				if (fi.id == c.blocks[i])
				{
					if (strcmp(name, fi.name) == 0)
					{
						cout << "file exists" << endl;
						return false;
					}
				}
			}
		}
	}
	//

	int id = -1;
	//find free descriptor
	for (int i = 1; i < descriptor_count; i++)
	{
		if (get_descriptor_by_ID(i)->links==0)
		{
			descriptor d = *get_descriptor_by_ID(i);
			d.links = 1;
			write_descriptor(i,d); 
			id = i;
			break;
		}
	}
	if (id == -1)
	{
		cout << "\t*no free space" << endl;
		return false;
	}
	fileName f;
	//write file name
	for (size_t i = 1; i < descriptor_count; i++)
	{
		f = *get_fileName_by_ID(i);
		if (f.id == 0)
		{
			f.id = id;
			strcpy(f.name, name);
			write_fileName(f,i);
			break;
		}
	}
	//add link current folder
	c = *get_descriptor_by_ID(currID);
		for (size_t i = 1; i < blocks_in_descriptor; i++)
		{
			if (c.blocks[i] == 0)
			{
				c.blocks[i] = id;
				break;
			}
		}
	write_descriptor(currID, c);
	
	return true;
}
u_int64 myFileSystem::open(char* name)
{
	u_int64 byte;
	fileName fn;
	fn.id = 0;
	int mfd = -1;
	//find descriptor
	descriptor c = *get_descriptor_by_ID(currID);
	for (size_t i = 1; i < blocks_in_descriptor; i++)
	{
		fn = *get_fileName_by_ID(c.blocks[i]);
		if (fn.id > 0 && strcmp(fn.name,name) == 0 )
		{
			break;
		}
	}
	if (fn.id == 0)
	{
		cout << "\t*file not found" << endl;
		return-1;
	}
	if (get_descriptor_by_ID(fn.id)->atr == 1) return -1;
	for (size_t i = 0; i < memfd_size; i++)
	{
		if (memfd[i] == 0){
			mfd = i;
			break;
		}
	}
	if (mfd == -1)
	{
		cout << "\t*can't open file. too many files are opened" << endl;
		return -1;
	}
	memfd[mfd] = fn.id;
	cout << "\tfd: " << mfd << endl;;
	return mfd;
}
bool myFileSystem::close(int fd)
{
	if (fd >= memfd_size)
		return false;
	memfd[fd] = 0;
	return true;
}
bool myFileSystem::read(int fd, int  offset, int size)
{
	if (fd > memfd_size)
		return false;
	int id = memfd[fd];
	if (id == 0) {
		cout << "\t*can't read... file not opened" << endl;
		return false;
	}
	char* result = new char[size + offset];
	strcpy(result, "");
	descriptor d = *get_descriptor_by_ID(id);
	u_int64 readed = 0;
	
	for (size_t i = 0; i < blocks_in_descriptor; i++)
	{
		if (d.blocks[i] != 0&&size>0)
		{
			if (offset >= block_size)
			{
				offset -= block_size;
				continue;
			}
			
			u_int64 size_to_read=0;
			if (offset == 0)
			{
				size_to_read = size > block_size ? block_size : size;
				size -= size_to_read;
			}
			else{
				size_to_read = size +offset > block_size ? block_size - offset: size-offset;
				size -= size_to_read;
			}
			u_int64 b = d.blocks[i] * block_size + offset;
			file.seekg(b, ios::beg);
			file.read(result+readed, size_to_read);
			readed += size_to_read;
		}
		else{
			if (size == 0)
				break;
		}
	}
	result[readed] = '\0';
	cout << "\tdata: " << result << endl;
	return true;
}
void myFileSystem::writeInternal(int id,int offset,int size,const char* data)
{
	if (size + offset > max_file_size)
	{
		cout << "can't write. file is too big" << endl;
		return;
	}
	u_int64 writed = 0;
	u_int64 col_blocks_offset = offset / block_size;
	test_block(id, col_blocks_offset);//if exist ok else create block
	u_int64 byte = get_descriptor_by_ID(id)->blocks[col_blocks_offset] * block_size + offset%block_size;
	u_int64 a_size = block_size - offset%block_size;
	u_int64 size_to_write = (size <= a_size?size:a_size);
	file.seekp(byte, ios::beg);
	file.write(data, size_to_write);
	writed +=size_to_write;
	while (writed < size)
	{
		col_blocks_offset++;
		test_block(id, col_blocks_offset);//if exist ok else create block
		byte = get_descriptor_by_ID(id)->blocks[col_blocks_offset] * block_size;
		size_to_write = ((size - writed) < block_size ? (size - writed):block_size);
		file.seekp(byte, ios::beg);
		file.write(data+writed, size_to_write);
		writed += size_to_write;
	}
	descriptor d = *get_descriptor_by_ID(id);
	d.size = offset + size;
	write_descriptor(id, d);
}
void myFileSystem::test_block(int id,int index)
{
	if (get_descriptor_by_ID(id)->blocks[index] == 0)
	{
		//add
		for (u_int64 i = 0; i < block_size; i++)
		{	
			if (!used(i))
			{
				use_block(i);
				descriptor d = *get_descriptor_by_ID(id);
				d.blocks[index] = i;
				write_descriptor(id,d);
				break;
			}
		}
	}
}
bool myFileSystem::used(u_int64 i)
{
	u_int64 byte = i / 8;
	u_int64 bit = i % 8;
	file.seekg(byte, ios::beg);
	char c;
	file.get(c);
	if (c&(1 << bit))
		return true;
	else
		return false;
}
descriptor* myFileSystem::get_descriptor_by_ID(int id)
{
	descriptor d;
	u_int64 byte = mask_size_block*block_size + id*sizeof(descriptor);
	file.seekg(byte, ios::beg);
	file.read((char *)&d,sizeof(descriptor));
	return &d;
}
bool myFileSystem::write(int fd, int  offset, int size, const char* data)
{
	int id = memfd[fd];
	if (id == 0) {
		cout << "\t*can't write... file not opened" << endl;
		return false;
	}
	writeInternal(id,offset,size,data);
	return true;
}
bool myFileSystem::link(char* name1, char* name2)
{
	fileName fn;
	int id = -1;
	//find descriptor
	for (size_t i = 1; i < descriptor_count; i++)
	{
		fn = *get_fileName_by_ID(i);
		if (strcmp(name1, fn.name) == 0)
		{
			id = fn.id;
			descriptor d = *get_descriptor_by_ID(id);
			d.links++;
			write_descriptor(id,d);
			break;
		}
	}
	if (id == -1)
	{
		cout << "file not found" << endl;
		return false;
	}
	//create link
	for (size_t i = 1; i < descriptor_count; i++)
	{
		fn = *get_fileName_by_ID(i);
		if (fn.id == 0)
		{
			strcpy(fn.name, name2);
			fn.id = id;
			write_fileName(fn,i);
			descriptor e = *get_descriptor_by_ID(currID);
			for (size_t i = 1; i < blocks_in_descriptor; i++)
			{
				if (e.blocks[i] == 0)
				{
					e.blocks[i] = id;
				}
			}
			write_descriptor(currID,e);
			return true;
		}
	}
	return false;
}
bool myFileSystem::unlink(char* name)
{
	fileName fn;
			descriptor e = *get_descriptor_by_ID(currID);
			for (size_t i = 1; i < blocks_in_descriptor; i++)
			{
				fn = *get_fileName_by_ID(e.blocks[i]);
				if (fn.id>0 && (strcmp(name, fn.name) == 0))
				{
					e.blocks[i] = 0;
					descriptor d = *get_descriptor_by_ID(fn.id);
					d.links--;
					if (d.links <= 0)
					{
						d.links = 0;
						d.atr = 0;
						//delete descriptor
						for (size_t i = 0; i < blocks_in_descriptor; i++)
						{
							if (d.blocks[i] != 0)
							{
								deuse(d.blocks[i]);
								d.blocks[i] = 0;
							}
						}
						d.size = 0;
					}
					write_descriptor(fn.id, d);
					fn.id = 0;
					strcpy(fn.name, "");
					write_fileName(fn, e.blocks[i]);
					write_descriptor(currID, e);
					return true;
				}
			}
		
	
	return false;
}
void myFileSystem::deuse(u_int64 i)
{
	u_int64 byte = i / 8;
	u_int64 bit = i % 8;
	file.seekg(byte, ios::beg);
	char c;
	file.get(c);
	c &= ~(1 << bit);
	file.seekp(byte, ios::beg);
	file.put(c);
}
bool myFileSystem::truncate(char* name, u_int64 size)
{
	//test max size
	if (size > blocks_in_descriptor*block_size)
	{
		cout << "\t*size more than file_max_size" << endl;
		return false;
	}
	//get file id
	int fd = open(name);
	if (fd == -1) return false;
	int id = memfd[fd];
	if (id == 0) return false;
	descriptor d = *get_descriptor_by_ID(id);
	if (d.atr == 1) return false;
	//if size > ? fill '0'
	if (size > d.size)
	{
		char* arr = new char[size - d.size];
		for (size_t i = 0; i < size - d.size; i++)
		{
			arr[i] = '0';
		}
		arr[size - d.size] = '\0';
		write(fd,d.size,size-d.size,arr);
	}
	else
	{
		u_int64 blocks_save = size / block_size + (size % block_size ? 1 : 0);
		d.size = size;
		//delete blocks that is after end block
		for (size_t i = blocks_save; i < blocks_in_descriptor; i++)
		{
			if (d.blocks[i] != 0)
			{
				deuse(d.blocks[i]);
				d.blocks[i] = 0;
			}
		}
		write_descriptor(id, d);
	}
	close(fd);
	return true;
}
bool myFileSystem::mkdir_e(char* name,int IDfrom)
{
	//folder exist
	descriptor de = *get_descriptor_by_ID(IDfrom);
	for (size_t i = 1; i < blocks_in_descriptor; i++)
	{
		if (de.blocks[i] != 0)
		{
			fileName fm = *get_fileName_by_ID(de.blocks[i]);
			if (strcmp(name, fm.name) == 0)
			{
				cout << "folder exists" << endl;
				return false;
			}
		}
	}
	//

	int id = -1;
	//find free descriptor
	for (int i = 1; i < descriptor_count; i++)
	{
		if (get_descriptor_by_ID(i)->links == 0)
		{
			descriptor d = *get_descriptor_by_ID(i);
			d.links = 1;
			d.atr = 1;
			write_descriptor(i, d);
			id = i;
			break;
		}
	}
	if (id == -1)
	{
		cout << "\t*no free space" << endl;
		return false;
	}
	fileName f;
	//write file name
	for (size_t i = 1; i < descriptor_count; i++)
	{
		f = *get_fileName_by_ID(i);
		if (f.id == 0)
		{
			f.id = id;
			strcpy(f.name, name);
			write_fileName(f,i);
			break;
		}
	}
	//add links
	descriptor d = *get_descriptor_by_ID(id);
	d.blocks[0] = IDfrom;
	write_descriptor(id, d);

		d = *get_descriptor_by_ID(IDfrom);
		for (size_t i = 1; i < blocks_in_descriptor; i++)
		{
			if (d.blocks[i] == 0)
			{
				d.blocks[i] = id;
				break;
			}
		}
		write_descriptor(IDfrom,d);
	
	return true;
}
bool myFileSystem::mkdir(char* name)
{
	int slesh_count = 0;
	for (size_t i = 0; i < strlen(name); i++)
	{
		if (name[i] == '\\'&& i != strlen(name) - 1 && i != 0)
		{
			slesh_count++;
		}
	}
	string* str = new string[slesh_count + 1];
	int index = 0;
	for (size_t i = 0; i < strlen(name); i++)
	{
		if (name[i] == '\\')
		{
			if (i != strlen(name) - 1 && i != 0) index++;
			continue;
		}
		if (name[i] != '\n')str[index] += name[i];
	}
	int temp = currID;
	for (size_t i = 0; i < slesh_count + 1; i++)
	{
		if (cd((char*)str[i].c_str(), currID) == -1)
		{
			mkdir_e((char *)str[i].c_str(), currID);
			if (slesh_count != 0) i--;
		}
	}
	currID = temp;
	return true;
}
bool myFileSystem::rmdir_e(char* name)
{
	fileName fn;
	int id = -1;
	descriptor c = *get_descriptor_by_ID(currID);
	int k;
	for (k = 0; k < blocks_in_descriptor; k++)
	{
		fn = *get_fileName_by_ID(c.blocks[k]);
		if (strcmp(fn.name, name) == 0)
		{
			descriptor tempD = *get_descriptor_by_ID(fn.id);
			if (tempD.atr == 1)
			{
				id = fn.id;
				break;
			}
		}
	}
	if (id == -1) return false;
	descriptor d = *get_descriptor_by_ID(id);
	for (size_t i = 1; i < blocks_in_descriptor; i++)
	{
		if (d.blocks[i] != 0)
		{
			cout << "folder is not empty" << endl;
			return false;
		}
	}
	unlink(name);
	if (id != -1)
	{
		strcpy(fn.name, "");
		fn.id = 0;
		write_fileName(fn, c.blocks[k]);
	}
	int IDfrom = d.blocks[0];//get id from
	d = *get_descriptor_by_ID(IDfrom);//open it
	//remove link
	for (int i = 0; i < blocks_in_descriptor; i++)
	{
		if (d.blocks[i] == id)
		{
			d.blocks[i] = 0;
			deuse(d.blocks[i]);
			break;
		}
	}
	write_descriptor(IDfrom, d);

}
bool myFileSystem::rmdir(char* name)
{
	int slesh_count = 0;
	for (size_t i = 0; i < strlen(name); i++)
	{
		if (name[i] == '\\'&& i != strlen(name) - 1 && i != 0)
		{
			slesh_count++;
		}
	}
	string* str = new string[slesh_count + 1];
	int index = 0;
	for (size_t i = 0; i < strlen(name); i++)
	{
		if (name[i] == '\\')
		{
			if (i != strlen(name) - 1 && i != 0) index++;
			continue;
		}
		if (name[i] != '\n')str[index] += name[i];
	}
	int tempID = currID;
	cd(name, currID);
	if (tempID == currID) return false;
	cd("..",currID);
	rmdir_e((char *)str[slesh_count].c_str());
	currID = tempID;
	
	return true;
}
void myFileSystem::pwd()
{
	fileName f;
	descriptor d;
	d = *get_descriptor_by_ID(currID);
	f = *get_fileName_by_ID(currID);
	cout << "\t. " << currID << " " << f.name << endl;
	f = *get_fileName_by_ID(d.blocks[0]);
	cout << "\t.. " << d.blocks[0] << " " << f.name << endl;

	for (size_t i = 1; i < blocks_in_descriptor; i++)
	{
		f = *get_fileName_by_ID(d.blocks[i]);
		if (f.id == 0) continue;
		if( get_descriptor_by_ID(f.id)->atr == 0)
		{
			cout << "\tfile: "<<f.id<<" "<< f.name << endl;
		}
		else
		{
			cout << "\tfolder: " << f.id << " " << f.name << endl;
		}
	}
}
fileName* myFileSystem::get_fileName_by_ID(int id)
{
	fileName f;
	u_int64 byte = fn_start + id * sizeof(fileName);
	file.seekg(byte, ios::beg);
	file.read((char*)&f, sizeof(fileName));
	return &f;
}
void myFileSystem::write_fileName(fileName f,int id)
{
	u_int64 byte = fn_start + id * sizeof(fileName);
	file.seekp(byte, ios::beg);
	file.write((char*)&f, sizeof(fileName));
}
u_int64 myFileSystem::cd_e(char* name, int curr)
{
	if (strcmp(name, "root") == 0)
	{
			currID = 0;
			return 0;
	}
	else
	{
		fileName fn;
		descriptor c = *get_descriptor_by_ID(currID);
		for (size_t i = 1; i < fileName_blocks_count; i++)
		{
			fn = *get_fileName_by_ID(c.blocks[i]);
			if (fn.id > 0 && strcmp(fn.name, name) == 0)
			{
				if (get_descriptor_by_ID(fn.id)->atr == 1)
				{
					currID = fn.id;
					return fn.id;
				}
				else
				{
					cout << "not a folder" << endl;
				}
			}
		}
	}
	currID = -1;
	return -1;
}
u_int64 myFileSystem::cd(char* name, int curr)
{
	int tempID = currID;
#pragma region 1
	if (strcmp(name, ".") == 0)
	{
		return curr;
	}
	if (strcmp(name, "..") == 0)
	{
		if (curr != 0)
		{
			currID = get_descriptor_by_ID(curr)->blocks[0];
			return get_descriptor_by_ID(curr)->blocks[0];
		}
		else{
			currID = 0;
			return 0;
		}
	}
	int slesh_count = 0;
	for (size_t i = 0; i < strlen(name); i++)
	{
		if (name[i] == '\\'&& i != strlen(name) - 1 && i != 0)
		{
			slesh_count++;
		}
	}
	string* str = new string[slesh_count + 1];
	int index = 0;
	for (size_t i = 0; i < strlen(name); i++)
	{
		if (name[i] == '\\')
		{
			if (i != strlen(name) - 1 && i != 0) index++;
			continue;
		}
		if (name[i] != '\n')str[index] += name[i];
	}
	//for (size_t i = 0; i < slesh_count+1; i++)
	//{
	//	cout << str[i].c_str()<< endl;
	//}
#pragma endregion

		for (int i = 0; i < slesh_count + 1; i++)
		{
			cd_e((char*)str[i].c_str(), currID);
		}
	
		if (currID == -1)
		{
			currID = tempID;
			return -1;
		}
	//parse end
	return currID;
}
bool myFileSystem::symlink(char* name1,char* name2)
{
	int temp = currID;
	int id = cd(name1,currID);
	if (id == -1)
	{
		cout << "not found" << endl;
		return false;
	}
	descriptor d = *get_descriptor_by_ID(id);
	d.links++;
	write_descriptor(id,d);
	currID = temp;
	fileName f1;
	for (size_t i = 1; i < descriptor_count; i++)
	{
		f1 = *get_fileName_by_ID(i);
		if (f1.id == 0)
		{
			f1.id = id;
			strcpy(f1.name, name2);
			write_fileName(f1,i);
			descriptor c = *get_descriptor_by_ID(currID);
			for (size_t j = 1; j < blocks_in_descriptor; j++)
			{
				if (c.blocks[j] == 0)
				{
					c.blocks[j] = i;
					break;
				}
			}
			write_descriptor(currID,c);
			return true;
		}
	}
	cout << "can't symlink" << endl;
	return false;
}