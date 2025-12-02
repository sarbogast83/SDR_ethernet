/*******************************
	fifoUDPdump.c
	S Arbogast
	send udp packets with fifo data
	
	format
	|4 byte unsigned counter|1024 bytes of dummy data|
	
	command line 
	udpsender <IP addr> 
	ex: ./udpsender 192.168.1.24 
	
	Checks fifo for > than 256 samples
	reads 26 samples into data and sends frame
	
	
********************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <stdlib.h> // For system()
#define _BSD_SOURCE
#include <time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <signal.h>

// hardware addres info
#define RADIO_TUNER_FAKE_ADC_PINC_OFFSET 0
#define RADIO_TUNER_TUNER_PINC_OFFSET 1
#define RADIO_TUNER_CONTROL_REG_OFFSET 2
#define RADIO_TUNER_TIMER_REG_OFFSET 3
#define RADIO_PERIPH_ADDRESS 0x43c00000
#define AXI_FIFO_ADDRESS 0x43c10000
#define AXI_FIFO_OCCUPANCY_OFFSET 7
#define AXI_FIFO_READ_OFFSET 8
#define AXI_FIFO_RESET_OFFSE 6

// UDP info
#define DEST_PORT 25344
#define DATA_SIZE  1024
#define MESSAGE_SIZE 1028

volatile bool cancel_requested = false; // global flag to handle closing program

/****************************************
* the below code uses a device called /dev/mem to get a pointer to a physical
* address.  We will use this pointer to read/write the custom peripheral
*
************************************************/
volatile unsigned int * get_a_pointer(unsigned int phys_addr)
{

	int mem_fd = open("/dev/mem", O_RDWR | O_SYNC); 
	void *map_base = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, phys_addr); 
	volatile unsigned int *radio_base = (volatile unsigned int *)map_base; 
	return (radio_base);
}

void radioTuner_tuneRadio(volatile unsigned int *ptrToRadio, float tune_frequency)
{
	float pinc = (-1.0*tune_frequency)*(float)(1<<27)/125.0e6;
	*(ptrToRadio+RADIO_TUNER_TUNER_PINC_OFFSET)=(int)pinc;
}

void radioTuner_setAdcFreq(volatile unsigned int* ptrToRadio, float freq)
{
	float pinc = freq*(float)(1<<27)/125.0e6;
	*(ptrToRadio+RADIO_TUNER_FAKE_ADC_PINC_OFFSET) = (int)pinc;
}


/******************************
*	get data frame form FIFO
*
******************************/
void get_fifo_data(volatile unsigned int *fifo_base, unsigned char *data)
{
	int fifo_ocp = 0;
	int current_sample = 0;
	int sample_size = DATA_SIZE / 4; // 4 bytes per complex sample_size
	int complex_sample;
	
	while(current_sample < sample_size - 1)
	{
		fifo_ocp = *(fifo_base+AXI_FIFO_OCCUPANCY_OFFSET);
	
		if (fifo_ocp > sample_size)
		{
			for(int i=0; i<sample_size; i++){
				complex_sample = *(fifo_base+AXI_FIFO_READ_OFFSET);
				data[i * 4 + 0] = (unsigned char)(complex_sample >> 16);
                data[i * 4 + 1] = (unsigned char)(complex_sample >> 24);
                data[i * 4 + 2] = (unsigned char)(complex_sample >> 0);
                data[i * 4 + 3] = (unsigned char)(complex_sample >> 8);
			}
			current_sample = sample_size;
		}
		
		if (fifo_ocp == 0){
			return;
		}
			
	}
}

// Signal handler function
void signal_handler(int signum) {
    if (signum == SIGINT) {
        printf("\nCtrl+C detected. Setting cancellation flag.\n");
        cancel_requested = true;
    }
}

int main(int argc,char *argv[])
{
	//validate command line args
	if (argc != 2){
		printf("Usage: %s <IP> <frame count>\n",argv[0]);
	return EXIT_FAILURE;
	}
	
	// first, get a pointer to the peripheral base address using /dev/mem and the function mmap
    volatile unsigned int *my_periph = get_a_pointer(RADIO_PERIPH_ADDRESS);	
	volatile unsigned int *my_fifo = get_a_pointer(AXI_FIFO_ADDRESS);
	
	// set radio control bits to 0b10
	//*(my_periph+RADIO_TUNER_CONTROL_REG_OFFSET) = 2;
	
	// set signal handler
	signal(SIGINT, signal_handler);
   
	
	// read in args
	const char * dest_ip = argv[1];
	
	//create socket 
	struct sockaddr_in peer_addr = {.sin_family = AF_INET, .sin_port = htons(DEST_PORT)};
	
	// create addr and validate
	if (inet_pton(AF_INET, dest_ip, &(peer_addr.sin_addr)) <= 0){
		perror("Error in IP address!");
		return EXIT_FAILURE;
	}
	
	// create socket and validate
	int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_socket < 0){
		perror("Error creating socket!");
		return EXIT_FAILURE;
	}
	
	
	unsigned char fifo_data[DATA_SIZE];
	unsigned int frame_count = 0;
	unsigned int frame_error = 0;
	unsigned char message[MESSAGE_SIZE];
	
	//printf("\r\n Running UPD test\r\n");
	while (!cancel_requested)
	{
		// Copy frame count to the start of the message buffer
        memcpy(message , &frame_count, sizeof(unsigned int)); // matlab reads in 
		
        // Copy the dummy data after the frame count
        get_fifo_data(my_fifo, fifo_data);
		memcpy(message + sizeof(uint32_t), fifo_data, DATA_SIZE);
		
		//send and validate
		if(sendto(udp_socket, message, MESSAGE_SIZE, 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr))<0){
			perror("Failed to send frame");
			frame_error++;
		}
		
		frame_count++;
	}
	
    unsigned int frames = frame_count - frame_error;
    //printf("\r\n UDP FIFO test complete\r\n");
	//printf("Successfully sent %d of %d frames.\r\n\r\n",frames,frame_count);
	close(udp_socket);
    return EXIT_SUCCESS;
}


