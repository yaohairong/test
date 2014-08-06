#include <stdio.h>
#include <string.h>
#include <random>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Test
{
	int a = 0;
	char msg[2][128];
};

static int uint_2_str(unsigned int x, char *str)
{
	unsigned int i = 0;
	unsigned int xx = x, t;

	while (xx) {
		str[i++] = (xx % 10) + '0';
		xx = xx / 10;
	}
	str[i] = '\0';

	xx = i;
	for (i = 0; i < xx; i++) {
		t = str[i];
		str[i] = str[xx - i -1];
		str[xx - i -1] = t;
	}

	return 0;
}
struct B;
struct A
{
	shared_ptr<B> m_b;
	~A()
	{
		cout<<"~A"<<endl;
	}
};
struct B
{
	weak_ptr<A> m_a;
	~B()
	{
		cout<<"~B"<<endl;
	}
};
int main()
{
	Test t;
	vector<int> vec;
}
