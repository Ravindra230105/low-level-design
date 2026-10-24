#include<iostream>
#include<string>

using namespace std;

class Coffee {
public:
    virtual double getCost() = 0;
    virtual string getDescription() = 0;
    virtual ~Coffee() {}   
};

class SimpleCoffee : public Coffee {
public:
    double getCost() override {
        return 5.0;
    }

    string getDescription() override {
        return "Simple Coffee";
    }
};

class CoffeeDecorator : public Coffee {
protected:
    Coffee* coffee;

public:
    CoffeeDecorator(Coffee* coffee) {
        this->coffee = coffee;
    }

    virtual ~CoffeeDecorator() {
        delete coffee;  
    }
};

class MilkDecorator : public CoffeeDecorator {
public:
    MilkDecorator(Coffee* coffee)
        : CoffeeDecorator(coffee) {}

    double getCost() override {
        return coffee->getCost() + 2.0;
    }

    string getDescription() override {
        return coffee->getDescription() + ", Milk";
    }
};

class SugarDecorator : public CoffeeDecorator {
public:
    SugarDecorator(Coffee* coffee)
        : CoffeeDecorator(coffee) {}

    double getCost() override {
        return coffee->getCost() + 1.0;
    }

    string getDescription() override {
        return coffee->getDescription() + ", Sugar";
    }
};

int main() {

    Coffee* coffee = new SimpleCoffee();

    coffee = new MilkDecorator(coffee);
    coffee = new SugarDecorator(coffee);

    cout << coffee->getDescription() << endl;
    cout << "Total Cost: " << coffee->getCost() << endl;

    delete coffee;   // only delete outermost decorator

    return 0;
}
