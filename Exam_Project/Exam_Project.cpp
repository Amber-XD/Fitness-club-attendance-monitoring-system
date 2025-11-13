
#include <iostream>
#include <Windows.h>
#include <string>
#include <stdexcept>
#include <ctime>
#include <vector>
#include <memory>
#include <algorithm>

using namespace std;

class Client;

struct Visit{
	shared_ptr<Client> client;
	string date;
	string notes;
	Visit() : client(nullptr), date(""), notes("") {}
	Visit(shared_ptr<Client> c, const string& d, const string& n) : client(c), date(d), notes(n) {}

	void print() const;

};

class Person {
protected:
	string name;
	int id;

public:
	Person(string name = "IDK", int id = 0) : name(name), id(id) {}
	virtual void printInfo() const {
		cout << "Name: " << name << ", ID: " << id << endl;
	}

	virtual ~Person() = default;
};

class Client : public Person {
private:
	string subscriptionType;
	tm expirationDate{};
	bool active;
	vector<Visit> visits;

	void checkVisitIndex(size_t i) const {
		if (i >= visits.size()) {
			throw out_of_range("Неправильний індекс відвідування");
		}
	}

public:
	Client(string name, int id, string subType, tm expDate) : Person(name, id), subscriptionType(subType), expirationDate(expDate) {
		active = checkStatus();
	}

	Client(const Client& other) : Person(other.name, other.id), subscriptionType(other.subscriptionType), expirationDate(other.expirationDate), active(other.active), visits(other.visits) {}

	bool isActive() const {
		return active;
	}
	bool checkStatus() const {
		time_t now = time(nullptr);
		tm today{};
		localtime_s(&today, &now);
		if (mktime(const_cast<tm*>(&expirationDate)) < mktime(&today)) {
			return false;
		}
		return true;
	}

	void updateStatus() {
		active = checkStatus();
		if (!active) {
			throw runtime_error("Абонемент прострочено!");
		}
	}

	void printInfo() const override {
		cout << "Client: " << name
			<< " | ID: " << id
			<< " | Type: " << subscriptionType
			<< " | Status: " << (active ? "Active" : "Expired") << endl;
	}

	bool operator<(const Client& other) const {
		return name < other.name;
	}

	string getName() const {
		return name;
	}
	int getID() const {
		return id;
	}
	string getSubscriptionType() const {
		return subscriptionType;
	}

	void rename(const string& newName) {
		if (newName != this->name) {
			this->name = newName;
		}
	}

	void addVisit(const Visit& v) {
		visits.push_back(v);
	}

	size_t getVisitsCount() const {
		return visits.size();
	}
	
	Visit& getVisit(size_t i) {
		checkVisitIndex(i);
		return visits[i];
	}

	const Visit& getVisit(size_t i) const {
		checkVisitIndex(i);
		return visits[i];
	}

	void editVisitDate(size_t i, const string& newDate) {
		checkVisitIndex(i);
		visits[i].date = newDate;
	}
	void editVisitNotes(size_t i, const string& newNotes) {
		checkVisitIndex(i);
		visits[i].notes = newNotes;
	}

	void printVisits() const {
		for (const auto& v : visits)
		{
			v.print();
		}
	}
};

void Visit::print() const {
	if (client) {
		cout << "Відвідування клієнта: " << client->getName() << " | Дата: " << date << " | Нотатки: " << notes << endl;
	}
	else {
		cout << "Відвідування (клієнт не вказаний) | Дата: " << date << " | Нотатки: " << notes << endl;
	}
}	

int main()
{
	//cout << "Hello Viewer!" << endl;
	SetConsoleOutputCP(1251);

	vector<shared_ptr<Client>> clients;

	tm exp = {};
	exp.tm_year = 2025 - 1900;
	exp.tm_mon = 11 - 1;
	exp.tm_mday = 12;

	auto client1 = make_shared<Client>("Anna", 1, "Premium", exp);
	clients.push_back(client1);

	auto client2 = make_shared<Client>("Ivan", 2, "Basic", exp);
	clients.push_back(client2);

	client1->addVisit(Visit(client1, "2025-11-01", "Йога-зал"));
	client1->addVisit(Visit(client1, "2025-11-05", "Фітнес"));

	cout << "До редагування:" << endl;
	client1->printVisits();

	client1->editVisitDate(0, "2025-11-02");
	client1->editVisitNotes(1, "Фітнес - зміна групи");

	cout << "\nПісля редагування:" << endl;
	client1->printVisits();


}
