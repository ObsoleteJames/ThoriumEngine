
#include <iostream>

int __declspec(dllexport) Tes()
{
	return 2;
}

int main()
{
	std::cout << "Hello\n";
	return 0;
}
