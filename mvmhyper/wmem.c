#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>//mmap head file
#define SIZE 0x1000
int main (void)
{
   int i;
   int fd;
   unsigned long *start;
   char *buf = "butterfly!";

   //open /dev/mem with read and write mode
   fd = open ("/dev/mem", O_RDWR);
   if (fd < 0)
   {
	   printf("cannot open /dev/mem.");
	   return -1;
   }

   //map physical memory 0-10 bytes 
   start = (unsigned long *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0xe00001);
   if(start < 0)
   {
	  printf("mmap failed.");
	  return -1;
   }
   //Read old value
   //start = (unsigned long *)((void*)start + 0x13d0);
   for (i = 0; i < 0x1000; i++)
   {
	   printf("0x%p  |  0x%lx\n", start + i, *((int *)start + i));
	   *((int *)start +i) = 0x6785876;
	   usleep(100);
   }
   //write memory
   //memcpy(start, buf, 10);
   //Read new value
   munmap(start, 10); //destroy map memory
   close(fd);  //close file
   return 0;
}
