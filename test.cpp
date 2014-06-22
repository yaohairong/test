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
	printf("%d %d %p\n", b.a, b.b, b.p);
	return 0;
}
