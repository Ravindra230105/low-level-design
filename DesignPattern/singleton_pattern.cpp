
#include<iostream>
#include<vector>
#include<string>
#include<algorithm>

using namespace std;

class Singleton {
    private:
    Singleton() {
        cout << "Singleton instance created" << endl;
    }

    public:
    Singleton(const Singleton&) = delete;  // Prevents creating a copy of the Singleton object
    Singleton& operator=(const Singleton&) = delete;  // Prevents assigning one Singleton object to another

    static Singleton* getInstance() {
        static Singleton instance;

        return &instance;
    }

    void showMessage() {
        cout << "Singleton instance is being used" << endl;
    }
};

int main() {
    Singleton* obj1 = Singleton::getInstance();
    obj1->showMessage();

    Singleton* obj2 = Singleton::getInstance();
    obj2->showMessage();

    if(obj1 == obj2) {
        cout << "Both pointers point to the same instance" << endl;
    }

    return 0;
}
