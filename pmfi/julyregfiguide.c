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
		printf("Select aim:\n0 do_fork\ninput:");
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
			printf("0 ax\ninput:");
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

