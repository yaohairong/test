
#include "Update.h"

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		printf("usage: %s localpath serverurl\n",strrchr(argv[0], '\\') + 1);
		return -1;
	}
	Update update(argv[1], argv[2]);
	if (update.CheckUpdate())
	{
		if (update.HasNewVersion())
		{
			update.update();
		}
	}
	return 0;
}