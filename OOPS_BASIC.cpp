#include<iostream>
#include<string>

using namespace std;

class BankAccount {
  private:
    string bankAccountHolder;
    double balance;

    void withdraw(double amount) {
        if(amount<=balance) {
            balance -= amount;
            cout << "Withdrew " << amount << " successfully."<<endl;
        } else {
            cout << "insufficient balance" << endl;
        }
    }
    
  public:
    string accountNumber;

    BankAccount(double initialBalance) {
        balance = (initialBalance <= 0) ? 0 : initialBalance;
    }

    void depositeAmount(double amount) {
        balance += amount;
        cout << "Deposited " << amount << " successfully" << endl;
    }

    void display() {
        cout << "Balance" << balance << endl;
    }
};

int main() {
    BankAccount acc(500);

    // acc.withdraw(100);    // error

    acc.depositeAmount(200);

    acc.display();
}