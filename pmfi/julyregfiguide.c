#include <stdio.h>
#include <string.h>

#define argnum 5
#define MAX_LINE 256

int main(int argc, char **argv)
{
	int i,aim,input,ret;
	char buff[MAX_LINE];
	//printf("%d\n",argc);
	//for(i=0;i<argc;++i)
	//	printf("%d\t%s\n",i,argv[i]);
	if(argc<argnum)
	{
		printf("Not enough args, turn to guide mode.\n");
		printf("Select aim:\n0 Reg FI\ninput:");
		scanf("%d",&input);
		memset(buff,'\0',sizeof(buff));
		sprintf(buff,"echo %d > /proc/julyregfi/aim",input);
		ret=system(buff);
		if(ret)
		{
			printf("Can't write the arg to proc, make sure the module's .ko was insmod.\n");
			return 0;
		}
		printf("Select fault:\n");
		aim=input;
		if(aim==0)
			printf("1 ax\t2 bx\t4 cx\n8 dx\t16 si\t32 di\n64 bp\t128 ds\t256 es\n512 fs\t1024 gs\t2048 orig_ax\n4096 ip\t8192 cs\t16384 flags\n32768 sp\t65536 ss\ninput:");
		scanf("%d",&input);
		memset(buff,'\0',sizeof(buff));
		sprintf(buff,"echo %d > /proc/julyregfi/fault",input);
		ret=system(buff);
		if(ret)
		{
			printf("Can't write the arg to proc, make sure the module's .ko was insmod.\n");
			return 0;
		}
		printf("Select time:\n(an integer large than 0)\ninput:");
		scanf("%d",&input);
		memset(buff,'\0',sizeof(buff));
		sprintf(buff,"echo %d > /proc/julyregfi/time",input);
		ret=system(buff);
		if(ret)
		{
			printf("Can't write the arg to proc, make sure the module's .ko was insmod.\n");
			return 0;
		}
		printf("Select id:\n(an integer)\ninput:");
		scanf("%d",&input);
		memset(buff,'\0',sizeof(buff));
		sprintf(buff,"echo %d > /proc/julyregfi/id",input);
		ret=system(buff);
		if(ret)
		{
			printf("Can't write the arg to proc, make sure the module's .ko was insmod.\n");
			return 0;
		}
		ret=system("echo 1 > /proc/julyregfi/signal");
		if(ret)
		{
			printf("Can't write the arg to proc, make sure the module's .ko was insmod.\n");
			return 0;
		}
		printf("FI Arg pass successfully.\n");
		return 0;
	}
	memset(buff,'\0',sizeof(buff));
	sprintf(buff,"echo %s > /proc/julyregfi/aim",argv[1]);
	ret=system(buff);
	if(ret)
	{
		printf("Can't write the arg to proc, make sure the module's .ko was insmod.\n");
		return 0;
	}
	memset(buff,'\0',sizeof(buff));
	sprintf(buff,"echo %s > /proc/julyregfi/fault",argv[2]);
	ret=system(buff);
	if(ret)
	{
		printf("Can't write the arg to proc, make sure the module's .ko was insmod.\n");
		return 0;
	}
	memset(buff,'\0',sizeof(buff));
	sprintf(buff,"echo %s > /proc/julyregfi/time",argv[3]);
	ret=system(buff);
	if(ret)
	{
		printf("Can't write the arg to proc, make sure the module's .ko was insmod.\n");
		return 0;
	}
	memset(buff,'\0',sizeof(buff));
	sprintf(buff,"echo %s > /proc/julyregfi/id",argv[4]);
	ret=system(buff);
	if(ret)
	{
		printf("Can't write the arg to proc, make sure the module's .ko was insmod.\n");
		return 0;
	}
	ret=system("echo 1 > /proc/julyregfi/signal");
	if(ret)
	{
		printf("Can't write the arg to proc, make sure the module's .ko was insmod.\n");
		return 0;
	}
	printf("FI Arg pass successfully.\n");
	return 0;
}

