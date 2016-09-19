#include <lib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdlib.h>


int main (void)
{

// variable 
  	int i = 0;
 	char buffer[1024 * 25600];
 	struct timeval s, e;




// Each test will read in 25 MB of data



// My Character Driver's performance

 	gettimeofday(&s, NULL);

 	size_t header;

 	for(i = 0; i < 10; i++) 
	{
		header = open("/dev/mydriver", O_RDONLY);
		if(header < 0) return EINVAL; // invalid destination 
 		read(header, &buffer, 1024*25600);
		close(header);
 	}

 	gettimeofday(&e, NULL);

 	printf("Average from GTOD MyDriver in msec: ");
 	printf("%ld\n", ((e.tv_sec * 1000000 + e.tv_usec) - (s.tv_sec * 1000000 + s.tv_usec))/10000);

	//close

// Zero Driver Performance

 	gettimeofday(&s, NULL);

 	 

 	for(i = 0; i < 10; i++) 
	{
		header = open("/dev/zero", O_RDONLY);
		if(header < 0) return EINVAL; // invalid destination 
 		read(header, &buffer, 1024 * 25600);
		close(header);
 	}

 	gettimeofday(&e, NULL);


 	printf("Average from GTOD Zero Driver in msec: ");
 	printf("%ld\n", ((e.tv_sec * 1000000 + e.tv_usec) - (s.tv_sec * 1000000 + s.tv_usec))/10000);


	//close

// Null Driver Performance

 	gettimeofday(&s, NULL);

 	 

 	for(i = 0; i < 10; i++) 
	{
		header = open("/dev/null", O_RDONLY);
		if(header < 0) return EINVAL; // invalid destination 
 		read(header, &buffer, 1024 * 25600);
		close(header);
 	}

 	gettimeofday(&e, NULL);


 	printf("Average from GTOD Null Driver in msec: ");
 	printf("%ld\n", ((e.tv_sec * 1000000 + e.tv_usec) - (s.tv_sec * 1000000 + s.tv_usec))/10000);


	//close



// File Block Driver Performance

 
 	gettimeofday(&s, NULL);

	//FILE * pFile;

 	//pFile = fopen("testFile.txt", "rb"); 
       //header = open("testFile.txt", O_RDONLY); 
	//if(header < 0) printf("error: header < 0");

 	for(i = 0; i < 10; i++) 
	{
		header = open("testFile.txt", O_RDONLY);
		if(header < 0) return EINVAL; // invalid destination 
 		read(header, &buffer, 1024*25600);
		//fread(pFile, 1024, 25600);
		close(header);
 	}

 	gettimeofday(&e, NULL);

 	printf("Average from GTOD FB Driver in msec: ");
 	printf("%ld\n", ((e.tv_sec * 1000000 + e.tv_usec) - (s.tv_sec * 1000000 + s.tv_usec))/10000);

	//close



}

