#include <time.h>
#include <stdlib.h>
#include <list>
#include <vector>
#include <iostream>
#include <cstdio>

using namespace std;

struct Step
{
	int x;
	int y;
	int d;
};

const Step step[] = 
{
	{0, 1, 10},
	{1, 1, 14},
	{1, 0, 10},
	{1, -1, 14},
	{0, -1, 10},
	{-1, -1, 14},
	{-1, 0, 10},
	{-1, 1, 14},
};

enum State
{
	S_EMPTY,
	S_BLOCK,
	S_OPEN,
	S_CLOSE,
	S_PATH,
};

struct Cell
{
	State s;
	int x;
	int y;
	int f;
	int g;
	int h;
	Cell* parent;
};
bool CompareCell(const Cell* c1, const Cell* c2)
{
	return c1->f < c2->f;
}

int width = 10;
int height = 10;
int start_x = 0;
int start_y = 0;
int goal_x = 9;
int goal_y = 9;
vector<vector<Cell> > cell_map;
Cell* current_cell;
list<Cell*> open_list;
vector<Cell*> close_list;
void print()
{
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (x == start_x && y == start_y)
			{
				cout<<'s';
			}
			else if (x == goal_x && y == goal_y)
			{
				cout<<'g';
			}
			else
			{
				switch(cell_map[y][x].s)
				{
					case S_BLOCK:
						cout<<"@";
						break;
					case S_PATH:
						cout<<'*';
						break;
					default:
						cout<<".";
						break;
				}
			}
		}
		cout<<endl;
	}
	cout<<endl;
}

void init()
{
	start_x = rand() % width;
	start_y = rand() % height;
	while (true)
	{
		goal_x = rand() % width;
		goal_y = rand() % height;
		if (goal_x != start_x || goal_y != start_y)
		{
			break;
		}
	}
	open_list.clear();
	close_list.clear();
	cell_map.resize(height);

	for (int y = 0; y < height; y++)
	{
		cell_map[y].resize(width);
		for (int x = 0; x < width; x++)
		{
			Cell cell;
			cell.s = S_EMPTY;
			cell.x = x;
			cell.y = y;
			cell.f = 0;
			cell.g = 0;
			cell.h = abs(goal_x - x) + abs(goal_y - y);
			cell.parent = NULL;
			cell_map[y][x] = cell;
		}
	}
	current_cell = &cell_map[start_y][start_x];
	current_cell->s = S_CLOSE;
	srand(time(NULL));
	int n = rand() % (width * height - 2);
	for (int i = 0; i < n; i++)
	{
		int x = rand() % width;
		int y = rand() % height;
		if (x == start_x && y == start_y)
		{
			continue;
		}
		if (x == goal_x && y == goal_y)
		{
			continue;
		}
		cell_map[y][x].s = S_BLOCK;
	}
}

bool go()
{
	while (true)
	{
		if (current_cell->x == goal_x && current_cell->y == goal_y)
		{
			Cell* parent_cell = current_cell->parent;
			while (parent_cell != NULL)
			{
				parent_cell->s = S_PATH;
				parent_cell = parent_cell->parent;
			}
			return true;
		}
		for (int i = 0; i < 8; i++)
		{
			int x = current_cell->x + step[i].x;
			int y = current_cell->y + step[i].y;
			if (x > -1 && x < width && y > -1 && y < height)
			{
				Cell* open_cell = &cell_map[y][x];
				if (open_cell->s == S_OPEN)
				{
					int g = current_cell->g + step[i].d;
					if (g < open_cell->g)
					{
						open_cell->g = g;
						open_cell->f = open_cell->g + open_cell->h;
						open_cell->parent = current_cell;
					}
				}
				else if (open_cell->s == S_EMPTY)
				{

					open_cell->g = current_cell->g + step[i].d;
					open_cell->f = open_cell->g + open_cell->h;
					open_cell->parent = current_cell;
					open_cell->s = S_OPEN;
					open_list.push_back(&cell_map[y][x]);
				}
			}
		}
		current_cell->s = S_CLOSE;
		close_list.push_back(current_cell);
		if (open_list.empty())
		{
			return false;
		}
		open_list.sort(CompareCell);
		current_cell = open_list.front();
		open_list.pop_front();
	}
}

int main()
{
	char buff[4];

	while (true)
	{
		while (true)
		{
			cout<<"input width(2-100, ctrl-c to exit)"<<endl;
			cin.clear();
			cin.getline(buff, 4);
			width = atoi(buff);
			if (width < 2 || width > 100)
			{
				cout<<"invalid width"<<endl;
				continue;
			}
			break;
		}
		while (true)
		{
			cout<<"input height(2-100, ctrl-c to exit)"<<endl;
			cin.clear();
			cin.getline(buff, 4);
			height = atoi(buff);
			if (height < 2 || height > 100)
			{
				cout<<"invalid height"<<endl;
				continue;
			}
			break;
		}
		init();
		print();
		if (go())
		{
			cout<<"find path"<<endl;
			print();
		}
		else
		{
			cout<<"no path"<<endl;
		}
	}
	return 0;
}
