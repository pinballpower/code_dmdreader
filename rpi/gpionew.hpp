#pragma once

#include <string>
#include <map>
#include <gpiod.hpp>

using namespace std;

enum GPIOEdge
{
	no,
	raising,
	falling,
	both
};

class GPIOException : public exception {

public:
	GPIOException(const string msg, int gpiono = 0);
	~GPIOException();

	string msg;
	int gpiono;
};

class GPIO
{
public:
	GPIO();
	~GPIO();

	::gpiod::chip chip;

	bool getValue(int gpiono);
	bool waitForEdge(int gpiono, int timeout=-1);

private:
	GPIO(const GPIO& src); // just make sure, this is uncopyable

	static string asEdgeStr(GPIOEdge e);
};