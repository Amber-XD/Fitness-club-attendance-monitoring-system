#define NOMINMAX
#include <iostream>
#include <Windows.h>
#include <string>
#include <stdexcept>
#include <ctime>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <future>
#include <limits>


using namespace std;

class Subscription {
protected:
	string type;
public:
	Subscription(const string& t) : type(t) {}

	virtual ~Subscription() = default;

	virtual string getType() const {
		return type;
	}

	virtual void benefits() const {
		cout << "Advantages of a general subscription" << endl;
	}

};

class Basic : public Subscription {
public:
	Basic() : Subscription("Basic") {}
	void benefits() const override {
		cout << "Basic subscription: access to the gym only" << endl;
	}
};

class Premium : public Subscription {
public:
	Premium() : Subscription("Premium") {}
	void benefits() const override {
		cout << "Premium subscription: access to the gym, swimming pool and discount on training with a trainer" << endl;
	}
};

class Client;

struct Visit {
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
	Person(string name = "Unknown", int id = 0) : name(name), id(id) {}
	virtual void printInfo() const {
		cout << "Name: " << name << ", ID: " << id << endl;
	}

	virtual ~Person() = default;
};

class Client : public Person {
private:
	shared_ptr<Subscription> subscription;
	tm expirationDate{};
	bool active;
	vector<shared_ptr<Visit>> visits;

	void checkVisitIndex(size_t i) const {
		if (i >= visits.size()) {
			throw out_of_range("Incorrect visit index");
		}
	}

public:
	Client(string name, int id, shared_ptr<Subscription> subType, tm expDate)
		: Person(name, id), subscription(subType), expirationDate(expDate) {
		active = checkStatus();
	}

	Client(const Client& other)
		: Person(other.name, other.id),
		subscription(other.subscription),
		expirationDate(other.expirationDate),
		active(other.active) {
		visits.reserve(other.visits.size());
		for (auto& v : other.visits) {
			visits.push_back(make_shared<Visit>(*v));
		}
	}

	bool isActive() const { return active; }

	bool checkStatus() const {
		time_t now = time(nullptr);
		tm today{};
		localtime_s(&today, &now);
		return mktime(const_cast<tm*>(&expirationDate)) >= mktime(&today);
	}

	void updateStatus() {
		active = checkStatus();
		if (!active) {
			throw runtime_error("Subscription expired");
		}
	}

	bool operator<(const Client& other) const {
		return name < other.name;
	}

	bool operator==(const Client& other) const {
		return static_cast<int>(id) == static_cast<int>(other.id);
	}

	friend ostream& operator<<(ostream& os, const Client& client) {
		os << "Client: " << client.name << " | ID: " << static_cast<int>(client.id) << " | Type: " << client.subscription->getType() << " | Status: " << (client.active ? "Active" : "Expired");
		return os;
	}

	string getName() const {
		return name;
	}

	int getID() const {
		return id;
	}

	string getSubscriptionType() const {
		return subscription->getType();
	}

	tm getExpirationDate() const {
		return expirationDate;
	}

	void rename(const string& newName) {
		if (newName != this->name) {
			this->name = newName;
		}
	}

	void addVisit(shared_ptr<Visit> v) {
		visits.push_back(v);
	}

	size_t getVisitsCount() const {
		return visits.size();
	}

	shared_ptr<Visit> getVisit(size_t i) const {
		checkVisitIndex(i);
		return visits[i];
	}

	void editVisitDate(size_t i, const string& newDate) {
		checkVisitIndex(i);
		visits[i]->date = newDate;
	}

	void editVisitNotes(size_t i, const string& newNotes) {
		checkVisitIndex(i);
		visits[i]->notes = newNotes;
	}

	void printVisits() const {
		for (const auto& v : visits) {
			if (v) v->print();
		}
	}

	shared_ptr<Client> findClientByID(const vector<shared_ptr<Client>>& clients, int id) {

		for (const auto& c : clients) {
			if (static_cast<int>(c->getID()) == static_cast<int>(id)) {
				return c;
			}
		}

		return nullptr;
	}

	vector<shared_ptr<Client>> findClientsByName(const vector<shared_ptr<Client>>& clients, const string& name) {

		vector<shared_ptr<Client>> result;

		for (auto& c : clients) {
			if (c->getName() == name) {
				result.push_back(c);
			}
		}

		return result;
	}

	string toLower(string s) {
		for (char& ch : s) {
			ch = tolower(ch);
		}
		return s;
	}

	vector<shared_ptr<Client>> searchCLients(const vector<shared_ptr<Client>>& clients, const string& pattern) {

		vector<shared_ptr<Client>> result;

		string p = toLower(pattern);

		for (auto& c : clients) {
			string n = toLower(c->getName());
			if (n.find(p) != string::npos) {
				result.push_back(c);
			}
		}
		return result;

	}

};

void saveClientToFile(const vector<shared_ptr<Client>>& clients, const string& filename) {
	ofstream file(filename);
	if (!file.is_open()) {
		throw runtime_error("Failed to open file for writing");
	}

	for (const auto& c : clients) {
		file << c->getName() << ";" << c->getID() << ";" << c->getSubscriptionType() << ";" << (1900 + c->getExpirationDate().tm_year) << '-' << (1 + c->getExpirationDate().tm_mon) << '-' << c->getExpirationDate().tm_mday << '\n';

		file << c->getVisitsCount() << '\n';
		for (size_t i = 0; i < c->getVisitsCount(); ++i) {
			auto visit = c->getVisit(i);
			file << visit->date << ";" << visit->notes << '\n';
		}
		file << "---\n";
	}
	file.close();
	cout << "Client data is saved to the file: " << filename << endl;

}


void loadClientFromFile(vector<shared_ptr<Client>>& clients, const string& filename) {
	ifstream file(filename);

	if (!file.is_open()) {
		throw runtime_error("Failed to open the file for reading");
	}

	clients.clear();
	string line;

	while (getline(file, line)) {
		if (line.empty() || line == "---") {
			continue;
		}

		stringstream ss(line);
		string name, idStr, subType, dateStr;

		getline(ss, name, ';');
		getline(ss, idStr, ';');
		getline(ss, subType, ';');
		getline(ss, dateStr, ';');

		int id = stoi(idStr);
		tm exp{};

		sscanf_s(dateStr.c_str(), "%d-%d-%d", &exp.tm_year, &exp.tm_mon, &exp.tm_mday);
		exp.tm_year -= 1900;
		exp.tm_mon -= 1;
		shared_ptr<Subscription> sub;
		if (subType == "Basic") sub = make_shared<Basic>();
		else if (subType == "Premium") sub = make_shared<Premium>();
		else sub = make_shared<Subscription>(subType);
		auto c = make_shared<Client>(name, id, sub, exp);

		if (!getline(file, line)) break;
		int visitCount = stoi(line);

		for (int i = 0; i < visitCount; ++i) {
			string visitLine;

			getline(file, visitLine);

			stringstream vs(visitLine);
			string vdate, vnote;

			getline(vs, vdate, ';');
			getline(vs, vnote);

			c->addVisit(make_shared<Visit>(c, vdate, vnote));
		}

		getline(file, line);

		clients.push_back(c);
	}
	file.close();
	cout << "Client data loaded from file: " << filename << endl;
}


shared_ptr<Client> findClientByID(const vector<shared_ptr<Client>>& clients, int id) {

	for (const auto& c : clients) {
		if (c->getID() == id) {
			return c;
		}
	}

	return nullptr;
}

vector<shared_ptr<Client>> findClientsByName(const vector<shared_ptr<Client>>& clients, const string& name) {

	vector<shared_ptr<Client>> result;

	for (auto& c : clients) {
		if (c->getName() == name) {
			result.push_back(c);
		}
	}

	return result;
}

string toLower(string s) {
	for (char& ch : s) {
		ch = tolower(ch);
	}
	return s;
}

vector<shared_ptr<Client>> searchCLients(const vector<shared_ptr<Client>>& clients, const string& pattern) {

	vector<shared_ptr<Client>> result;

	string p = toLower(pattern);

	for (auto& c : clients) {
		string n = toLower(c->getName());
		if (n.find(p) != string::npos) {
			result.push_back(c);
		}
	}
	return result;

}

void sortByName(vector<shared_ptr<Client>>& clients) {
	sort(clients.begin(), clients.end(), [](const shared_ptr<Client>& a, const shared_ptr<Client>& b) {
		return a->getName() < b->getName();
		});
}

void sortByID(vector<shared_ptr<Client>>& clients) {
	sort(clients.begin(), clients.end(), [](const shared_ptr<Client>& a, const shared_ptr<Client>& b) {
		return a->getID() < b->getID();
		});
}

void sortByExpirationDate(vector<shared_ptr<Client>>& clients) {
	sort(clients.begin(), clients.end(), [](const shared_ptr<Client>& a, const shared_ptr<Client>& b) {
		tm ta = a->getExpirationDate();
		tm tb = b->getExpirationDate();
		return mktime(&ta) < mktime(&tb);
		});

}

void Visit::print() const {
	if (client) {
		cout << "Visiting a client: " << client->getName() << " | Date: " << date << " | Notes: " << notes << endl;
	}
	else {
		cout << "Visiting (client not specified) | Date: " << date << " | Notes: " << notes << endl;
	}
}

mutex logMutex;
mutex clientMutex;
condition_variable logCV;
queue<string> logQueue;
bool doneLogging = false;
bool doneUpdating = false;

void logThreadFunc() {
	ofstream logFile("log.txt", ios::app);
	if (!logFile.is_open()) {
		cerr << "Failed to open log file" << endl;
		return;
	}

	unique_lock<mutex> lock(logMutex);
	while (!doneLogging || !logQueue.empty()) {
		logCV.wait(lock, [] {
			return !logQueue.empty() || doneLogging;
			});

		while (!logQueue.empty()) {
			string message = logQueue.front();
			logQueue.pop();
			lock.unlock();
			logFile << message << endl;
			lock.lock();
		}
	}

	logFile.close();

}

void addLog(const string& message) {
	{
		lock_guard<mutex> lock(logMutex);
		logQueue.push(message);
	}
	logCV.notify_one();
}

void updateClientStatusThread(vector<shared_ptr<Client>>& clients, int intervalSec) {
	while (!doneUpdating) {
		this_thread::sleep_for(chrono::seconds(intervalSec));

		lock_guard<mutex> lock(clientMutex);
		for (auto& c : clients) {
			try {
				c->updateStatus();
				addLog("Client " + c->getName() + " is active");
			}
			catch (const exception& e) {
				addLog("Client " + c->getName() + " status check" + string(e.what()));
			}
		}
	}
}

bool isValidEnglishInput(const string& input) {
	for (char c : input) {
		if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == ' ' || c == '_')) {
			return false;
		}
	}
	return true;
}

void showMenu() {
	cout << "\nMenu:\n";
	cout << "1. Add Client\n";
	cout << "2. View Clients\n";
	cout << "3. Save clients to file\n";
	cout << "4. Load clients from file\n";
	cout << "5. Add Visit\n";
	cout << "6. Search Client by ID\n";
	cout << "7. Search Clients by Name\n";
	cout << "8. Sort Clients\n";
	cout << "9. Exit\n";
	cout << "Enter your choice: ";
}

void showSortMenu() {
	cout << "\nSort by:\n";
	cout << "1. Name\n";
	cout << "2. ID\n";
	cout << "3. Expiration Date\n";
	cout << "Enter your choice: ";
}

int main()
{
	//cout << "Hello Viewer!" << endl;
	SetConsoleOutputCP(1251);
	try {
		vector<shared_ptr<Client>> clients;

		thread logThread(logThreadFunc);
		thread updateThread(updateClientStatusThread, ref(clients), 5);

		bool running = true;

		while (running)
		{
			try {


				showMenu();
				int choise;
				cin >> choise;

				if (!cin) {
					cin.clear();
					cin.ignore(numeric_limits<streamsize>::max(), '\n');
					throw runtime_error("Invalid input. Enter a number");
				}

				switch (choise) {
				case 1: {
					try {
						string name, type;
						int id;
						string dateStr;

						cout << "Enter client name: ";
						cin >> name;
						if (!isValidEnglishInput(name)) {
							throw runtime_error("Invalid input: Only English letters, digits, dash, underscore, and space allowed");
						}

						cout << "Enter client ID: ";
						cin >> id;

						if (findClientByID(clients, id)) {
							throw runtime_error("This ID is aready used");
						}

						cout << "Enter subscription type: ";
						cin >> type;

						for (auto& ch : type) {
							ch = tolower(ch);
						}

						if (type != "basic" && type != "premium") {
							throw runtime_error("Subscription must be Basic or Premium");
						}

						cout << "Enter expiration date (YYYY-MM-DD): ";
						cin >> dateStr;

						int year, month, day;
						if (sscanf_s(dateStr.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
							throw runtime_error("Invalid date format. Use YYYY-MM-DD");
						}

						if (year < 1900 || year > 2100) {
							throw runtime_error("Invalid year");
						}

						if (month < 1 || month > 12) {
							throw runtime_error("Invalid month");
						}

						int daysInMonth[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
						if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
							daysInMonth[1] = 29;
						}

						if (day < 1 || day > daysInMonth[month - 1]) {
							throw runtime_error("Invalid day for this month");
						}

						tm exp{};
						exp.tm_year = year - 1900;
						exp.tm_mon = month - 1;
						exp.tm_mday = day;

						shared_ptr<Subscription> sub;

						auto c = make_shared<Client>(name, id, sub, exp);

						clients.push_back(c);
						addLog("Added client: " + name + " with ID: " + to_string(id));

						cout << "Client added successfully" << endl;
					}
					catch (const exception& e) {
						cout << "Error: " << e.what() << endl;
					}
					break;
				}
				case 2: {
					if (clients.empty()) {
						cout << "No clients available" << endl;
						break;
					}

					cout << "\nClients List:" << endl;
					for (auto& c : clients) {
						cout << *c << endl;
						c->printVisits();
						cout << endl;
					}
					break;
				}
				case 3: {
					try {
						saveClientToFile(clients, "clients.txt");
						addLog("Clients saved to file");
					}
					catch (const exception& e) {
						cout << "Save error: " << e.what() << endl;
					}
					break;
				}
				case 4: {
					try {
						loadClientFromFile(clients, "clients.txt");
						addLog("Clients loaded from file");
					}
					catch (const exception& e) {
						cout << "Load error: " << e.what() << endl;
					}

					break;
				}
				case 5: {
					if (clients.empty()) {
						cout << "No clients available\n";
						break;
					}
					int id;
					cout << "Enter client ID for visit: ";
					cin >> id;
					auto client = findClientByID(clients, id);
					if (!client) {
						cout << "Client not found\n";
						break;
					}

					string vdate, vnote;
					cout << "Enter visit date (YYYY-MM-DD): ";
					cin >> vdate;
					for (char c : vdate) {
						if (!isdigit(c) && c != '-') {
							throw runtime_error("Invalid date format: only digits and '-' allowed");
						}
					}
					cin.ignore();
					cout << "Enter visit notes: ";
					cin.ignore(numeric_limits<streamsize>::max(), '\n');
					getline(cin, vnote);
					if (!isValidEnglishInput(vnote)) {
						throw runtime_error("Invalid input: Only English letters, digits, dash, underscore, and space allowed");
					}

					int year, month, day;
					if (sscanf_s(vdate.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
						cout << "Invalid date format. Visit not added\n";
						break;
					}
					tm visitTm{};
					visitTm.tm_year = year - 1900;
					visitTm.tm_mon = month - 1;
					visitTm.tm_mday = day;

					tm expDate = client->getExpirationDate();
					if (mktime(&visitTm) > mktime(&expDate)) {
						cout << "Warning: Visit is after subscription expired\n";
						addLog("Client " + client->getName() + " visited after subscription expired on " + vdate);
					}

					client->addVisit(make_shared<Visit>(client, vdate, vnote));
					addLog("Added visit for client ID " + to_string(id));
					cout << "Visit added successfully\n";
					break;
				}

				case 6: {
					int id;
					cout << "Enter client ID to search: ";
					cin >> id;
					auto client = findClientByID(clients, id);
					if (client) {
						cout << *client << endl;
						client->printVisits();
					}
					else {
						cout << "Client not found\n";
					}
					break;
				}

				case 7: {
					string name;
					cout << "Enter client name to search: ";
					cin >> name;
					auto results = searchCLients(clients, name);
					if (!results.empty()) {
						for (auto& c : results) {
							cout << *c << endl;
							c->printVisits();
						}
					}
					else {
						cout << "No clients found\n";
					}
					break;
				}

				case 8: {
					showSortMenu();
					int sortChoice;
					cin >> sortChoice;
					switch (sortChoice) {
						case 1: {
							sortByName(clients);
							break;
						}
						case 2: {
							sortByID(clients);
							break;
						}
						case 3: {
							sortByExpirationDate(clients);
							break;
						}
						default: {
							cout << "Invalid choice\n"; 
							continue;
						}
					}
					cout << "Clients sorted successfully\n";
					break;
				}
				case 9: {
					running = false;
					break;
				}
				default:
					cout << "Invalid choice. Please try again" << endl;
				}
			}
			catch (const exception& e) {
				cout << "Error: " << e.what() << endl;
			}
		}

		{
			lock_guard<mutex> lock(logMutex);
			doneLogging = true;
		}

		logCV.notify_one();
		logThread.join();
		doneUpdating = true;
		updateThread.join();
	}
	catch (const exception& e) {
		cerr << "Fatal error: " << e.what() << endl;
	}
	catch (...) {
		cerr << "Unknown fatal error occurred" << endl;
	}

}

