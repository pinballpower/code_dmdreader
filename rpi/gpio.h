#pragma "once"

#include <string>
#include <map>

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

	static void setup_gpio(int gpiono, bool output, GPIOEdge e);
	bool get_value(int gpiono);
	bool wait_for_edge(int gpiono, int timeout=-1);

private:
	GPIO(const GPIO& src); // just make sure, this is uncopyable

	static string edge_str(GPIOEdge e);
	int get_value_fd(int gpiono);

	map<int, int> value_fd;
};