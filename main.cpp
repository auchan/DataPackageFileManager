#include "package.h"

void Test1();
void Test2();
void Test3();
void Test4();

int main()
{
	//Test1();
	Test2();
	//Test3();
	Test4();

	Test3();

}

void Test1()
{
	BinaryPack bp(100);

	std::string str = "Hello, World!";
	bp << str;
	bp << (uint8_t)2;
	bp << (uint8_t)3;
	bp << (uint8_t)3;

	BinaryPack bp2(bp.GetData(), bp.GetDataSize());
	std::string str2;
	uint8_t a, b, c;
	bp2 >> str2;
	bp2 >> a >> b >> c >> c;

	return;
}

void Test2()
{
	Package pak;

	pak.open("Test2.pak", OpenMode::CREATE);
	pak.addFile("t1", "t1.txt");
	pak.addFile("t2", "t2.txt");
	pak.save();

	return;
}

void Test3()
{
	Package pak;

	pak.open("Test2.pak", OpenMode::READ_ONLY);

	size_t size, zipSize;
	pak.getFileData("t3", NULL, &size, &zipSize);

	uint8_t *data = new uint8_t[zipSize];
	pak.getFileData("t3", data, &size, &zipSize);

	fwrite(data, zipSize, 1, stdout);
	//pak.reorganize();
	return;
}

void Test4()
{
	Package pak;

	pak.open("Test2.pak", OpenMode::READ_WRITE);
	pak.addFile("t1", "Are you ok? na nb no", 20, 20);
	pak.addFile("t3", "t11.txt");
	pak.addFile("t4", "t12.txt");
	pak.addFile("t5", "t12.txt");
	//pak.addFile("t2", "t2.txt");
	pak.deleteFile("t1");
	pak.deleteFile("t2");
	pak.deleteFile("t4");
	pak.deleteFile("t5");

	pak.save();
}