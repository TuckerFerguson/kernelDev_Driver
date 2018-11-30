#ifndef __BOOGA_H
#define __BOOGA_H
/*
* Majority of this code was derived from the examples located in 
* ~/CS453-resources/examples/device-management/linux-device-drivers
*
*/

#ifndef BOOGA_MINORS
#define BOOGA_MINORS 4    
#endif


#ifndef BOOGA_MAJOR
#define BOOGA_MAJOR 0    
#endif

#define TYPE(dev)   (MINOR(dev) >> 4)  
#define NUM(dev)    (MINOR(dev) & 0xf) 
#define SUDDEN_DEATH 3


struct booga_stats {
	long int num_read;
	long int num_write; 
	long int booga0; 
	long int booga1; 
	long int booga2; 
	long int booga3; 
	long int string0; 
	long int string1; 
	long int string2; 
	long int string3;
    struct semaphore sem;
};
typedef struct booga_stats booga_stats;

//extern booga_vals booga_Device_vals;

#endif /* __BOOGA_H */