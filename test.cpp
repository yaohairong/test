#include <stdio.h>
#include <string.h>

static int uint_2_str(unsigned int x, char *str)
{
	int i = 0;
	unsigned int xx = x, t;

	while (xx) {
		str[i++] = (xx % 10) + '0';
		xx = xx / 10;
	}
	str[i] = '\0';

	xx = i;
	for (i = 0; i < xx/2; i++) {
		t = str[i];
		str[i] = str[xx - i -1];
		str[xx - i -1] = t;
	}

	return xx;
}
int main(int argc, char *argv[])
{
	int a[][4] =
	{
		{1, 0, 1, 0},
		{0, 1, 0, 1},
		{1, 1, 1, 1},
		{0, 0, 0, 1},
		{1, 0, 0, 0},
		{0, 0, 1, 0},
		{1, 1, 1, 0},
	};
	int n = sizeof(a) / sizeof(a[0]);
	for (int i = 0; i < n; i++)
	{
		int m = 0;
		for (int j = 0; j < 4; j++)
		{
			printf("%d", a[i][j]);
			if (a[i][j])
			{
				m |= 1 << j;
			}
		}
		printf("\n%x\n", m);
	}
}
