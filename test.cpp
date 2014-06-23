#include <stdio.h>
#include <string.h>

class B
{
	public:
		int a;
		int b;
		int* p;
		int c;
};

int main(int argc, char *argv[])
{
	B b;
	for (int i = 40; i < 48; i++)
	{
		for (int j = 30; j < 38; j++)
		{
			printf("\x1b[%d;%dmhello world\nx1b[0m", i, j);
		}
	}

}
