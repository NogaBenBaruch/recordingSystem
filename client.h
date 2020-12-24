struct header_package {
	int id; 
	int num_of_data_packages; 
	int data[10]; 
};

struct data_package {
	int id;
	int data[124];
};

const int NUM_OF_HEADERS = 10; 
int ids_arr[NUM_OF_HEADERS];
int num_of_dp_arr[NUM_OF_HEADERS];
void channel1_send();
void channel2_send();
