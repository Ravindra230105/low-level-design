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

class RazorPay : public PaymentStrategy {
    public:
    void pay(int amount) override {
        cout << "paid " << amount << " using Credit Card" << endl;
    }
};

class PhonePe : public PaymentStrategy {
    public:
    void pay(int amount) override {
        cout << "paid " << amount << " using UPI" << endl;
    }
};

class PaymentProcessor {
    private:
    PaymentStrategy* paymentStrategy;

    public:
    PaymentProcessor(PaymentStrategy* strategy) {
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
    RazorPay card;
    PhonePe upi;

    PaymentProcessor paymentProcessor(&card);
    paymentProcessor.checkout(500);

    paymentProcessor.setPaymentStrategy(&upi);
    paymentProcessor.checkout(300);
}