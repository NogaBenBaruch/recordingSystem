#include <unordered_map> 
#include <vector>
#include <mutex>
using namespace std;

struct header_package {
	int id;
	int num_of_data_packages;
	int data[10];
};

struct data_package {
	int id;
	int data[124];
};

char data_buff[1024];
char header_buff[1024];
mutex output_mtx;
mutex map_mtx;
unordered_map<int, pair<header_package*, vector<data_package*>>> mp;
void channel1_listen();
void channel2_listen();
bool received_all_data(int id);
void write_to_disk(int id);
