#include<iostream>
#include<string>
#include<vector>
#include<algorithm>

using namespace std;

class Observer {
    public:
    virtual void update(string message) = 0;
    virtual ~Observer() {}
};

class Subject {
    public:
    virtual void addObserver(Observer* obs) = 0;
    virtual void removeObserver(Observer* obs) = 0;
    virtual void notifyObservers() = 0;
    virtual ~Subject() {}
};

class Order : public Subject {
    private: 
    vector<Observer*>observers;
    string status;

    public:
    void addObserver(Observer* obs) override {
        observers.push_back(obs);
    }
    
    void removeObserver(Observer* obs) override {
        observers.erase(
            remove(observers.begin(), observers.end(), obs),
            observers.end()
        );
    }

    void setStatus(string newStatus) {
        status = newStatus;
        notifyObservers();
    }

    string getStatus() {
        return status;
    }

    void notifyObservers() override {
        for (Observer* obs : observers) {
            obs->update(status);
        }
    }
};

class EmailObserver : public Observer {
    public:
    void update(string message) override {
        cout<<"email send : order"<< message << endl;
    }
};

class SMSObserver : public Observer {
    public:
    void update(string message) override {
        cout<<"SMS sent : order"<< message << endl;
    }
};

int main(){
    Order order;

    EmailObserver email;
    SMSObserver sms;

    order.addObserver(&email);
    order.addObserver(&sms);

    order.setStatus("PLACED");
    order.setStatus("SHIPPED");
}