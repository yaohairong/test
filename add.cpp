nclude <iostream>
#include <vector>

using namespace std;

int cnt = 0;
void Add(vector<int>& list, int s, int n)
{
	for (int i = s; i <= n / 2; i++)
	{
		for (auto v : list)
		{
			cout<<v;
		}
		cout<<i<<n - i<<endl;
		cnt++;
		list.push_back(i);
		Add(list, i, n - i);
		list.pop_back();
	}
}

int main()
{
	vector<int> list;
	Add(list, 1,  30);
	cout<<"cnt"<<cnt<<endl;
	return 0;
}
