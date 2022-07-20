#include <iostream>
#include <stdlib.h>
using namespace std;
typedef unsigned long size_t;

void func(size_t& length)
{
	length = 100;
}

int main()
{
	char ipString[40] = {};
	unsigned char value = 255;
	sprintf(ipString, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x", value,value,value,value,value,
			value,value,value,value,value,
			value,value,value,value,value,value);
	// cout << ipString << endl;


	unsigned char mcc[10] = {};
	unsigned char plmmString = 255;
	sprintf((char*)mcc, "%u%u%u", plmmString, plmmString, plmmString);
	cout << mcc << endl;


	unsigned int  a = 10;
	unsigned int  ptr = a;
	func((size_t&)ptr);
	cout << ptr << endl;
	
	char arr[100] = {0};
	arr[10] = '\0';
	cout << "sizeof(char*) = " << sizeof(arr) << endl;
	cout << "strlen(char*) = " << strlen(arr) << endl;

	cout << "This is test for rebase." << endl;

	cout << "Today is 7/20." << endl;

	cout << "This for last one." << endl;
}
