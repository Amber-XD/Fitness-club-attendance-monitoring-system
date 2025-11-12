
#include <iostream>
#include <string>

using namespace std;


struct Visit{
	shared_ptr<Client> client;
	string date;
	string notes;
	Visit(shared_ptr<Client> c, string& d, string& n) : client(c), date(d), notes(n) {}
};

class Client {
	

};

int main()
{
	cout << "Hello Viewer!" << endl;

}
