#include<iostream>
#include<vector>
#include<string>
#include<algorithm>

using namespace std;

class PaymentStrategy {
    public:
    virtual void pay(int amount) = 0;
    virtual ~PaymentStrategy() {}
};

class CardPayment : public PaymentStrategy {
    public:
    void pay(int amount) override {
        cout << "paid " << amount << " using Credit Card" << endl;
    }
};

class UpiPayment : public PaymentStrategy {
    public:
    void pay(int amount) override {
        cout << "paid " << amount << " using UPI" << endl;
    }
};

class Order {
    private:
    PaymentStrategy* paymentStrategy;

    public:
    Order(PaymentStrategy* strategy) {
        paymentStrategy = strategy;
    }

    void setPaymentStrategy(PaymentStrategy* strategy) {
        paymentStrategy = strategy;
    }

    void checkout(int amount) {
        paymentStrategy->pay(amount);
    }
};

int main() {
    CardPayment card;
    UpiPayment upi;

    Order order(&card);
    order.checkout(500);

    order.setPaymentStrategy(&upi);
    order.checkout(300);
}