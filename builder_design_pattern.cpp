#include <iostream>
#include <string>

using namespace std;

class User {
private:
    string name;
    int age;
    string email;
    string phone;

public:
    User(string name, int age, string email, string phone)
        : name(name), age(age), email(email), phone(phone) {}

    void display() const {
        cout << "Name: " << name << endl;
        cout << "Age: " << age << endl;
        cout << "Email: " << email << endl;
        cout << "Phone: " << phone << endl;
    }
};


class UserBuilder {
private:
    string name;
    int age;
    string email = "";   // optional
    string phone = "";   // optional

public:
    UserBuilder(string name, int age)
        : name(name), age(age) {}

    UserBuilder& setEmail(string email) {
        this->email = email;
        return *this;  // enables method chaining
    }

    UserBuilder& setPhone(string phone) {
        this->phone = phone;
        return *this;   
    }

    User build() {
        return User(name, age, email, phone);
    }
};


int main() {

    User user = UserBuilder("Aman", 25)
                    .setEmail("aman@gmail.com")
                    .setPhone("9999999999")
                    .build();

    user.display();

    return 0;
}

