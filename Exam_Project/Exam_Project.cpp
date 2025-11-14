
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
    vector<shared_ptr<Visit>> visits;

    void checkVisitIndex(size_t i) const {
        if (i >= visits.size()) {
            throw out_of_range("Неправильний індекс відвідування");
        }
    }

public:
    Client(string name, int id, string subType, tm expDate)
        : Person(name, id), subscriptionType(subType), expirationDate(expDate) {
        active = checkStatus();
    }

    Client(const Client& other)
        : Person(other.name, other.id),
        subscriptionType(other.subscriptionType),
        expirationDate(other.expirationDate),
        active(other.active),
        visits(other.visits) {
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
            throw runtime_error("Абонемент прострочено!");
        }
    }

    void printInfo() const override {
        cout << "Client: " << name << " | ID: " << id << " | Type: " << subscriptionType << " | Status: " << (active ? "Active" : "Expired") << endl;
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
        for (char& ch : s){
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
        throw runtime_error("Не вдалося відкрити файл для запису");
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
    cout << "Дані клієнтів збережено у файл: " << filename << endl;

}

    
void loadClientFromFile (vector<shared_ptr<Client>>& clients, const string& filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        throw runtime_error("Не вдалося відкрити файл для читання");
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

        auto c = make_shared<Client>(name, id, subType, exp);

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
    cout << "Дані клієнтів завантажено з файлу: " << filename << endl;
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

    tm exp1{}, exp2{}, exp3{};
    sscanf_s("2025-12-01", "%d-%d-%d", &exp1.tm_year, &exp1.tm_mon, &exp1.tm_mday);
    exp1.tm_year -= 1900; exp1.tm_mon -= 1;

    sscanf_s("2024-01-01", "%d-%d-%d", &exp2.tm_year, &exp2.tm_mon, &exp2.tm_mday);
    exp2.tm_year -= 1900; exp2.tm_mon -= 1;

    sscanf_s("2025-10-10", "%d-%d-%d", &exp3.tm_year, &exp3.tm_mon, &exp3.tm_mday);
    exp3.tm_year -= 1900; exp3.tm_mon -= 1;

    clients.push_back(make_shared<Client>("Tom", 1, "Premium", exp1));
    clients.push_back(make_shared<Client>("Bob", 2, "Basic", exp2));
    clients.push_back(make_shared<Client>("Thomas", 3, "Premium", exp3));

    clients[0]->addVisit(make_shared<Visit>(clients[0], "2025-12-05", "Gym"));
    clients[0]->addVisit(make_shared<Visit>(clients[0], "2025-12-07", "Yoga"));

    clients[1]->addVisit(make_shared<Visit>(clients[1], "2024-01-10", "Swimming"));

    cout << "Search by ID 1:\n";
    auto c = findClientByID(clients, 1);
    if (c) cout << c->getName() << endl;

    cout << "\nSearch name 'Tom':\n";
    for (auto& x : findClientsByName(clients, "Tom"))
        cout << x->getName() << endl;

    cout << "\nSearch pattern 'om':\n";
    for (auto& x : searchCLients(clients, "om"))
        cout << x->getName() << endl;

    saveClientToFile(clients, "clients.txt");

    vector<shared_ptr<Client>> loadedClients;
    loadClientFromFile(loadedClients, "clients.txt");

    cout << "\nClients loaded from file:\n";
    for (auto& x : loadedClients) {
        x->printInfo();
        x->printVisits();
    }
}

