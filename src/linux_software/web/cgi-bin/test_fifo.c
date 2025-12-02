#include <stdio.h>
#include <sys/mman.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <stdlib.h> // For system()
#define _BSD_SOURCE
#include <time.h>

#define RADIO_TUNER_FAKE_ADC_PINC_OFFSET 0
#define RADIO_TUNER_TUNER_PINC_OFFSET 1
#define RADIO_TUNER_CONTROL_REG_OFFSET 2
#define RADIO_TUNER_TIMER_REG_OFFSET 3
#define RADIO_PERIPH_ADDRESS 0x43c00000
#define AXI_FIFO_ADDRESS 0x43c10000
#define AXI_FIFO_OCCUPANCY_OFFSET 7
#define AXI_FIFO_READ_OFFSET 8
#define AXI_FIFO_RESET_OFFSE 6

// the below code uses a device called /dev/mem to get a pointer to a physical
// address.  We will use this pointer to read/write the custom peripheral
volatile unsigned int * get_a_pointer(unsigned int phys_addr)
{

	int mem_fd = open("/dev/mem", O_RDWR | O_SYNC); 
	void *map_base = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, phys_addr); 
	volatile unsigned int *radio_base = (volatile unsigned int *)map_base; 
	return (radio_base);
}

 /************************************************************
	Time process to read 480,000 samples from FIFO
	Does NOT save samples 
	Time should be roughly 10 sec
 *************************************************************/
void timed_fifo_test(volatile unsigned int *fifo_base)
{
	struct timespec start, stop;
    double start_time, current_time, elapsed_time;
	int total_samples = 480000;
	int current_sample = 0;
	int fifo_ocp;
	
	printf("Content-Type:text/html\n\n"); 
    printf("<html>\n");
    printf("<head><title>C Program Output</title></head>\n");
    printf("<body>\n");
	printf("<h1>Milestone 2: Radio + Custom Peripheral</h1>\n");
	clock_gettime(CLOCK_MONOTONIC, &start);
	start_time = start.tv_sec+(double)start.tv_nsec/1e9;
	
	while(current_sample < total_samples)
	{
		fifo_ocp = *(fifo_base+AXI_FIFO_OCCUPANCY_OFFSET);
		for(int i=0; i<fifo_ocp; i++)
			*(fifo_base+AXI_FIFO_READ_OFFSET);
		
		current_sample = current_sample + fifo_ocp;	
	}
	clock_gettime(CLOCK_MONOTONIC, &stop);
	current_time = stop.tv_sec+(double)stop.tv_nsec/1e9;
	elapsed_time = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec)/1e9;
	printf("<h2>Test Complete: Read 480000 samples\n<h2>");
	printf("<p> Start time [sec]: %f <p>\n", start_time);
	printf("<p>Stop time [sec]: %f\n<p>", current_time);
	printf("<h2>Total Time [sec]: %f\n<h2>",elapsed_time);
	printf("</body>\n");
    printf("</html>\n");
}

int main()
{

// first, get a pointer to the peripheral base address using /dev/mem and the function mmap
    volatile unsigned int *my_periph = get_a_pointer(RADIO_PERIPH_ADDRESS);	
	volatile unsigned int *my_fifo = get_a_pointer(AXI_FIFO_ADDRESS);
	
    //printf("\r\n\r\n\r\nLab 11 Steven Arbogast - FIFO TEST\n\r");
    *(my_periph+RADIO_TUNER_CONTROL_REG_OFFSET) = 2; // make sure radio isn't in reset and fifo is running
 
    //printf("Running FIFO test\r\n");
	timed_fifo_test(my_fifo);
    //printf("FIFO test complete\r\n");
    return 0;
}
