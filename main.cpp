#include "pack.h"

void Test1();

int main()
{
	Test1();
}

void Test1()
{
	BinaryStream bs(100);

	int a = 10086, b = 0;
	bs.push(&a, sizeof(a));
	//bs.pop()
}