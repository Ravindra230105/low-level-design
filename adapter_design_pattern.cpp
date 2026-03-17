#include <iostream>
using namespace std;

class PaymentProcessor {
public:
    virtual void pay(double amount) = 0;
    virtual ~PaymentProcessor() = default;
};


class RazorpayAPI {
public:
    void makePayment(double value) {
        cout << "Payment of " << value << " processed via Razorpay\n";
    }
};


class RazorpayAdapter : public PaymentProcessor {
private:
    RazorpayAPI& razorpay;    

public:
    RazorpayAdapter(RazorpayAPI& razorpay)
        : razorpay(razorpay) {}

    void pay(double amount) override {
        razorpay.makePayment(amount);
    }
};


class OrderService {
private:
    PaymentProcessor& processor;

public:
    OrderService(PaymentProcessor& processor)
        : processor(processor) {}

    void checkout(double amount) {
        processor.pay(amount);
    }
};

int main() {

    RazorpayAPI razorpay;                 
    RazorpayAdapter adapter(razorpay);    
    OrderService service(adapter);      

    service.checkout(2000);

    return 0;
}


