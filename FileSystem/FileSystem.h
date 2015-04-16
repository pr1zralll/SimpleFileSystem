#include <fstream>
using namespace std;
#define u_int64 unsigned long long
const int blocks_in_descriptor = 14;
struct descriptor
{
	int atr = 0;
	int links = 0;
	u_int64 size = 0;
	u_int64 blocks[blocks_in_descriptor];
};
struct fileName
{
	int id = 0;
	char name[64];
};

class myFileSystem
{
	fstream file;
	u_int64 currID = 0;
	u_int64 memfd_size = 0;
	u_int64 size = 0;
	u_int64 max_file_size = 0;
	u_int64 block_size = 1024;
	u_int64 block_count = 0;
	u_int64 mask_size_block = 0;
	u_int64 descriptor_count = 0;
	u_int64 descriptor_size_block = 0;
	u_int64 fileName_blocks_count = 0;
	u_int64 fn_start = 0;
	int *memfd;
public:
	bool mounted = false;
	myFileSystem(char*);
	bool unmount();
	void filestat(int);
	void ls();
	bool create(char*);
	u_int64 open(char*);
	void clear();
	bool close(int);
	bool read(int, int, int);
	bool write(int, int, int, const char*);
	bool link(char*, char*);
	bool unlink(char*);
	bool truncate(char*, u_int64);
	bool mkdir_e(char*,int);
	bool mkdir(char*);
	bool rmdir(char*);
	bool rmdir_e(char*);
	u_int64 cd(char*,int);
	u_int64 cd_e(char*, int);
	void pwd();
	bool symlink(char*,char*);
private:
	fileName* get_fileName_by_ID(int);
	void write_fileName(fileName,int);
	void create_fs();
	void deuse(u_int64 id);
	void test_block(int,int);
	descriptor* get_descriptor_by_ID(int);
	void use_block(u_int64);
	void write_descriptor(u_int64, descriptor);
	void writeInternal(int, int, int,const char*);
	bool used(u_int64);
};
