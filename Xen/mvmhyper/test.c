#include <stdio.h>
#define max 1000000
int main()
{
	int a[max];
	int i;
	for(i=0;i<max;++i)
		a[i]=i+1;
	for(i=max-1;i>=0;--i)
		if(a[i]!=i+1)
			printf("%d\n",a[i]);
	printf("Done.\n");
	return 0;
}
