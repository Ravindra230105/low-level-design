
#include<iostream>
#include<vector>
#include<string>
#include<algorithm>

using namespace std;

class Payment {
    public:
    virtual void pay(int amount) = 0;
    virtual ~Payment() {}
};

class CreditCardPayment : public Payment {
    public:
    void pay(int amount) override {
        cout << "paid " << amount << " using Credit Card" << endl;
    }
};

class UPIPayment : public Payment {
    public:
    void pay(int amount) override {
        cout << "paid " << amount << " using UPI" << endl;
    }
};

class PaymentFactory {
    public:
    static Payment* createPayment(string type) {
        if(type == "CARD") {
            return new CreditCardPayment();
        }
        else if(type == "UPI") {
            return new UPIPayment();
        }

        return nullptr;
    }
};

class PaymentProcessor {
    public:
    void checkout(Payment* payment, int amount) {
        if(payment == nullptr) {
            cout << "Invalid payment method" << endl;
            return;
        }

        payment->pay(amount);
        delete payment;
    }
};

int main() {
    Payment* card = PaymentFactory::createPayment("CARD");
    PaymentProcessor paymentProcessor;
    paymentProcessor.checkout(card, 500);

    Payment* upi = PaymentFactory::createPayment("UPI");
    paymentProcessor.checkout(upi, 300);

    return 0;
}
