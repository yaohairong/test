#include <mysql/mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

typedef unsigned uint32;

uint32 GetMSTime()
{
	timeval tv;
	timezone tz;
	gettimeofday(&tv, &tz);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

int main()
{
	MYSQL mysql;
	mysql_init(&mysql);
	if (mysql_real_connect(&mysql, "192.168.56.101", "yhr", "yhr", "test", 0, NULL, 0) == NULL)
	{
		printf("failed to connect database: Error %s\n", mysql_error(&mysql));
		return -1;
	}
	char sql[128] = {0};
	snprintf(sql, 128, "SELECT * from t1");
	uint32 t = GetMSTime();
	if (mysql_query(&mysql, sql) != 0)
	{
		printf("failed query: Error %s\n", mysql_error(&mysql));
		return -1;
	}
	MYSQL_RES* result;
	result = mysql_store_result(&mysql);
	MYSQL_FIELD* fields;
	MYSQL_ROW row;
	unsigned long* lengths;
	int num_fields = mysql_num_fields(result);
	fields = mysql_fetch_fields(result);
	for (int i = 0; i < num_fields; i++)
	{
		//printf("%s\t", fields[i].name);
	}
	//printf("\n");
	while ((row = mysql_fetch_row(result)) != NULL)
	{
		lengths = mysql_fetch_lengths(result);
		for (int i = 0; i < num_fields; i++)
		{
			//printf("%s\t", row[i] ? row[i] : "NULl");
		}
		//printf("\n");
	}
	printf("time %d\n", GetMSTime() - t);

	return 0;
}
