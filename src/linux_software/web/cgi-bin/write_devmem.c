#include <stdio.h>
#include <sys/mman.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define _BSD_SOURCE

#define RADIO_TUNER_FAKE_ADC_PINC_OFFSET 0
#define RADIO_TUNER_TUNER_PINC_OFFSET 1
#define RADIO_TUNER_CONTROL_REG_OFFSET 2
#define RADIO_TUNER_TIMER_REG_OFFSET 3
#define RADIO_PERIPH_ADDRESS 0x43c00000

// the below code uses a device called /dev/mem to get a pointer to a physical
// address.  We will use this pointer to read/write the custom peripheral
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

void radioTuner_setControl(volatile unsigned int* ptrToRadio, int control)
{
	*(ptrToRadio+RADIO_TUNER_CONTROL_REG_OFFSET) = control;
}






int main(int argc,char *argv[])
{
	if (argc != 4) {
		
        return EXIT_FAILURE;
    }
	float ADC_FREQ_HZ = atof(argv[1]);
	float TUNE_FREQ_HZ = atof(argv[2]);
	const char *STREAMING = argv[3];

    volatile unsigned int *my_periph = get_a_pointer(RADIO_PERIPH_ADDRESS);	


    radioTuner_tuneRadio(my_periph, TUNE_FREQ_HZ);
	radioTuner_setAdcFreq(my_periph, ADC_FREQ_HZ);
	if (strcmp(STREAMING, "on") == 0) 
		radioTuner_setControl(my_periph, 2);
	else
		radioTuner_setControl(my_periph, 0);

    return 0;
}
