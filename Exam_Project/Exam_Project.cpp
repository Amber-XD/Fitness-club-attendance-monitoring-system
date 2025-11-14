
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
		cout << "Переваги загальної передплати" << endl;
    }

};

class Basic : public Subscription {
public:
    Basic() : Subscription("Basic") {}
    void benefits() const override {
        cout << "Базова передплата: доступ тільки до тренажерного залу" << endl;
	}
};

class Premium : public Subscription {
public:
    Premium() : Subscription("Premium") {}
    void benefits() const override {
        cout << "Преміум передплата: доступ до тренажерного залу, басейну та знижка на тренування з тренером" << endl;
    }
};

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
    shared_ptr<Subscription> subscription;
    tm expirationDate{};
    bool active;
    vector<shared_ptr<Visit>> visits;

    void checkVisitIndex(size_t i) const {
        if (i >= visits.size()) {
            throw out_of_range("Неправильний індекс відвідування");
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

    /*void printInfo() const override {
        cout << "Client: " << name << " | ID: " << id << " | Type: " << subscriptionType << " | Status: " << (active ? "Active" : "Expired") << endl;
    }*/

    bool operator<(const Client& other) const {
        return name < other.name;
    }

    bool operator==(const Client& other) const {
        return id == other.id;
	}

    friend ostream& operator<<(ostream& os, const Client& client) {
        os << "Client: " << client.name << " | ID: " << client.id << " | Type: " << client.subscription->getType() << " | Status: " << (client.active ? "Active" : "Expired");
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

    clients.push_back(make_shared<Client>("Tom", 1, make_shared<Premium>(), exp1));
    clients.push_back(make_shared<Client>("Bob", 2, make_shared<Basic>(), exp2));
    clients.push_back(make_shared<Client>("Thomas", 3, make_shared<Premium>(), exp3));

    clients[0]->addVisit(make_shared<Visit>(clients[0], "2025-12-05", "Gym"));
    clients[0]->addVisit(make_shared<Visit>(clients[0], "2025-12-07", "Yoga"));

    clients[1]->addVisit(make_shared<Visit>(clients[1], "2024-01-10", "Swimming"));

    cout << "Search by ID 1:\n";
    auto c = findClientByID(clients, 1);
    if (c) cout << *c << endl;

    cout << "\nSearch name 'Tom':\n";
    for (auto& x : findClientsByName(clients, "Tom"))
        cout << *x << endl;

    cout << "\nSearch pattern 'om':\n";
    for (auto& x : searchCLients(clients, "om"))
        cout << *x << endl;

    saveClientToFile(clients, "clients.txt");

    vector<shared_ptr<Client>> loadedClients;
    loadClientFromFile(loadedClients, "clients.txt");

    cout << "\nClients loaded from file:\n";
    for (auto& x : loadedClients) {
        cout << *x << endl;
        x->printVisits();
    }

    cout << "\nПеретворення типів ID: " << endl;
	int origID = loadedClients[0]->getID();
	cout << "Оригінальний ID: " << origID << endl;

	double idDouble = static_cast<double>(origID);
	cout << "double ID: " << idDouble << endl;

	long idLong = static_cast<long>(idDouble);
	cout << "long ID: " << idLong << endl;

	int idInt = static_cast<int>(idDouble);
	cout << "Повернутий int ID: " << idInt << endl;

	cout << "\nSorting by Name:\n";
	sortByName(loadedClients);
    for (auto& x : loadedClients) {
        cout << *x << endl;
	}
	cout << "\nSorting by ID:\n";
	sortByID(loadedClients);
    for (auto& x : loadedClients) {
        cout << *x << endl;
	}
	cout << "\nSorting by Expiration date:\n";
	sortByExpirationDate(loadedClients);
    for (auto& x : loadedClients) {
        cout << *x << endl;
	}

}

