#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define C_NUM 1024
int get_para(int, char**);
void print_usage();
int my_atoi(char *str);
int get_digit(int num);
int get_handle();
int get_memory();
void run();
int get_part_memory(int i);

unsigned long  block_size = -1;
char block_type = 'B';
int block_num = -1;
int **handle = NULL;
int main(int argc, char* argv[])
{
	int i = 0;
	if(get_para(argc, argv) == -1)
	{
		print_usage();
		return 0;
	}
	printf("block size = %ld\nblock num = %d\n", block_size, block_num);	
	if(get_handle() == -1)
	{
		printf("内存申请失败,请尝试缩小块的数量\n");
		return -1;
	}
	if(get_memory() == -1)
	{
		return -1;
	}
	run();
	for(i = 0; i < block_num; i++)
	{
		free(handle[i]);
	}
	free(handle);
	return 0;
}
void run()
{
	int i = 0;
	int tag = 0;
	while(1)
	{
		printf("Num %d\n", tag);
		for(i = 0; i < block_num; i++)
		{
			if(tag % 2 == 0)
			{
				if(memset(handle[i], 0xFFFF, block_size) == NULL)
				{
					printf("\t%d failed!\n", i);
				}
				else printf("\t%d success!\n", i);
			}
			else
			{
				if(memset(handle[i], 0x0, block_size) == NULL)
				{
					printf("\t%d failed!\n", i);
				}
				else printf("\t%d success!\n", i);
			}
		}
		tag ++;
		sleep(1);
	}
}
int get_memory()
{
	int i = 0; 
	for(i = 0; i < block_num; i++)
	{
		handle[i] = (int *)calloc(sizeof(char), block_size);
		if(handle[i] == NULL)
		{
			return get_part_memory(i);
		}
		else
		{
			memset(handle[i], 0xFFFF, block_size);
			printf("We already get %d %c memory\n", i + 1, block_type);
		}
	}
	return 0;
}
int get_part_memory(int i)
{
	printf("We have no enough memory!\n");
	if(i == 0) return -1;
	printf("Use %d%c Memory that we alread get?(y/n)", i, block_type);
	char temp = getchar();
	if(temp == 'y' || temp == 'Y')
	{
		block_num = i;
		return 0;
	}
	else
		return -1;
}
int get_handle()
{
	int i = 0;
	handle = (int **)calloc(sizeof(int *), block_num);
	if(handle == NULL) return -1;
	for(i = 0; i < block_num; i ++)
	{
		handle[i] = NULL;
	}
	return 0;
}
int get_para(int argc, char* argv[])
{
	//int temp;
	char ch;
	if(argc <= 1 || argc > 3)
	{
		return -1;
	}
	block_size = 1;
	if(argc == 3)
	{
		if(argv[1][0] == '-')
		{
			ch = argv[1][1];
			if(ch == 'B' || ch == 'b')
			{
				block_size = 1;
				block_type = 'B';
			}
			else if(ch == 'k' || ch == 'K')
			{
				block_size = 1;
				block_size *= C_NUM;
				block_type = 'K';
			}
			else if(ch == 'm' || ch == 'M')
			{
				block_size = 1;
				block_size *= C_NUM;
				block_size *= C_NUM;
				block_type = 'M';
			}
			else if(ch == 'g' || ch == 'G')
			{
				block_size = 1;
				block_size *= C_NUM;
				block_size *= C_NUM;
				block_size *= C_NUM;
				block_type = 'G';
			}
			else
			{
				print_usage();
				return -1;
			}
		}
		else
		{
			return -1;
		}
		block_num = my_atoi(argv[2]);
	}
	else if(argc == 2)
	{
		block_num = my_atoi(argv[1]);
	}
	if(block_size == -1 || block_num == -1)
	{
		return -1;
	}
	return 0;
}
void print_usage()
{
	printf("\nusemem USEAGE:\n");
	printf("\tusemem 10 : use 10 Byte Memory, the default block size is Bype\n\n");
	printf("\tusemem -B 10 : use 10 Byte Memory\n\n");
	printf("\tusemem -K 10 : use 10 K Byte Memory\n\n");
	printf("\tusemem -M 10 : use 10 M Byte Memory\n\n");
	printf("\tusemem -G 10 : use 10 G Byte Memory\n\n");
	return;
}
int my_atoi(char *str)
{
	int temp = atoi(str);
	if(strlen(str) != get_digit(temp))
	{
		printf("num is not valid\n");
		return -1;
	}	
	return temp;
}
int get_digit(int num)
{
	int size = 0;
	double temp = (double)num;
	if(num < 0)
	{
		return size;
	}
	else if(num == 0)
	{

	}
	while(temp >= 1)
	{
		temp /= 10;
		size++;
	}
	return size;
}
