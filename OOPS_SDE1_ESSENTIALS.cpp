// ===================================================================================
//        OOPS ESSENTIALS FOR SDE-1 INTERVIEWS (0-1 YEAR)  --  C++17, single file
// ===================================================================================
//  Compile : g++ -std=c++17 OOPS_SDE1_ESSENTIALS.cpp -o oops && ./oops
//
//  Only what actually gets asked at SDE-1. Nothing exotic.
//
//  01  Class, Object, Access specifiers, this
//  02  Constructors, Destructor, Copy ctor, Shallow vs Deep, Virtual destructor
//  03  static members + const member functions
//  04  ENCAPSULATION
//  05  ABSTRACTION (abstract class vs interface)
//  06  INHERITANCE (5 types, access table, ctor/dtor order, diamond)
//  07  POLYMORPHISM (overloading vs overriding, virtual, vtable, slicing)
//  08  Operator overloading (the 3 you need)
//  09  friend
//  10  Composition vs Inheritance + relationships
//  11  Smart pointers (unique/shared/weak)
//  12  SOLID
//  13  6 must-know design patterns
//  14  Rapid-fire Q&A  <-- read this the night before
// ===================================================================================

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <cstring>

using namespace std;

static void banner(const string& title) {
    cout << "\n=========== " << title << " ===========\n";
}

// ===================================================================================
// 01 : CLASS, OBJECT, ACCESS SPECIFIERS, this
// ===================================================================================
//  CLASS  = blueprint (data + behaviour). OBJECT = instance of it, takes memory.
//
//  private   -> only this class (and its friends)     [default for class]
//  protected -> this class + derived classes
//  public    -> everyone                              [default for struct]
//
//  struct vs class in C++ : ONLY the default access differs. Nothing else.
//  `this` = pointer to the current object. Use it to disambiguate and to `return *this`
//           for method chaining.
// -----------------------------------------------------------------------------------
namespace basics {

class Student {
private:
    string name;
    int    marks;

protected:
    string section;

public:
    int rollNumber;

    Student(string name, int marks, int rollNumber)
        : name(name), marks(marks), rollNumber(rollNumber) {}

    Student& setName(const string& name) {
        this->name = name;                                  // this-> disambiguates
        return *this;                                       // enables chaining
    }

    Student& setMarks(int marks) {
        this->marks = marks;
        return *this;
    }

    void print() const { cout << "  " << rollNumber << " " << name << " " << marks << "\n"; }
};

void demo() {
    banner("01 class / object / this");

    Student student("Ravi", 80, 1);
    student.setName("Ravi Sapkal").setMarks(92).print();    // chaining
    // student.marks = 100;   // ERROR: private
    student.rollNumber = 7;                                 // OK: public
    student.print();
}

} // namespace basics

// ===================================================================================
// 02 : CONSTRUCTORS, DESTRUCTOR, COPY CTOR, SHALLOW vs DEEP, VIRTUAL DESTRUCTOR
// ===================================================================================
//  CONSTRUCTOR : same name as class, no return type, runs automatically.
//     default          Foo()
//     parameterized    Foo(int)
//     copy             Foo(const Foo&)     <-- takes a REFERENCE; by value would recurse forever
//     Can be overloaded. CANNOT be virtual. Can be private (Singleton).
//
//  MEMBER INITIALIZER LIST  Foo(int x) : member(x) {}
//     REQUIRED for: const members, reference members, and base classes / members that
//     have no default constructor. Also faster than assigning inside the body.
//
//  DESTRUCTOR ~Foo() : no args, cannot be overloaded, one per class.
//
//  SHALLOW vs DEEP COPY
//     The compiler-generated copy constructor copies members bit-by-bit. For a raw
//     pointer that copies the POINTER, so two objects share one buffer and BOTH delete
//     it -> double free / crash. If your class owns a raw resource you must write:
//     destructor + copy constructor + copy assignment  (RULE OF THREE).
//     Easiest fix in real code: use std::string / vector / smart pointers and write NONE
//     of them (RULE OF ZERO).
//
//  VIRTUAL DESTRUCTOR - very common interview question
//     If you `delete basePtr` and the object is really a Derived, the derived destructor
//     runs ONLY if the base destructor is virtual. Otherwise -> leak / undefined behaviour.
//     RULE: any class meant to be a base class gets `virtual ~Base() = default;`
// -----------------------------------------------------------------------------------
namespace ctors {

class Buffer {                                              // owns a raw resource
private:
    char*  data;
    size_t size;

public:
    explicit Buffer(const char* text)
        : size(strlen(text)), data(new char[strlen(text) + 1]) {
        strcpy(data, text);
    }

    Buffer(const Buffer& other)                             // DEEP copy
        : size(other.size), data(new char[other.size + 1]) {
        strcpy(data, other.data);
        cout << "  deep copy made (separate memory)\n";
    }

    Buffer& operator=(const Buffer& other) {                // copy assignment
        if (this == &other) return *this;                    // self-assignment guard
        char* fresh = new char[other.size + 1];             // allocate BEFORE deleting
        strcpy(fresh, other.data);
        delete[] data;
        data = fresh;
        size = other.size;
        return *this;
    }

    ~Buffer() { delete[] data; }

    void set(size_t index, char value) { data[index] = value; }
    const char* text() const { return data; }
};

class Base {
public:
    Base()          { cout << "  Base ctor\n"; }
    virtual ~Base() { cout << "  Base dtor\n"; }             // VIRTUAL -> derived dtor runs
};

class Derived : public Base {
private:
    int* payload;

public:
    Derived()  : payload(new int[100]) { cout << "  Derived ctor\n"; }
    ~Derived() override { delete[] payload; cout << "  Derived dtor (memory freed)\n"; }
};

void demo() {
    banner("02 constructors / deep copy / virtual destructor");

    Buffer original("hello");
    Buffer copy = original;                                 // copy ctor
    copy.set(0, 'J');
    cout << "  original=" << original.text() << "  copy=" << copy.text()
         << "   <-- unaffected because the copy is DEEP\n";

    cout << "-- delete through a base pointer --\n";
    Base* pointer = new Derived();
    delete pointer;                                         // both dtors run (virtual)
    cout << "  without `virtual`, only Base dtor would run -> leak\n";
}

} // namespace ctors

// ===================================================================================
// 03 : static MEMBERS + const MEMBER FUNCTIONS
// ===================================================================================
//  static data member    : ONE copy shared by all objects. Must be defined outside
//                          the class (or be `static inline` / constexpr).
//  static member function: has NO `this`, can only touch static members,
//                          called as ClassName::function(). Used for counters and
//                          for Singleton::getInstance().
//  const member function : promises not to modify the object -> it is the only kind you
//                          can call on a const object. Mark every read-only getter const.
// -----------------------------------------------------------------------------------
namespace statics {

class User {
private:
    static int userCount;                                   // shared
    string name;

public:
    explicit User(string name) : name(name) { ++userCount; }
    ~User() { --userCount; }

    const string& getName() const { return name; }          // const getter
    static int getUserCount() { return userCount; }          // static: no `this`
};

int User::userCount = 0;                                     // definition, exactly once

void demo() {
    banner("03 static / const");

    User first("A"), second("B");
    cout << "  count=" << User::getUserCount() << "\n";
    {
        User third("C");
        cout << "  inside scope count=" << User::getUserCount() << "\n";
    }
    cout << "  after scope count=" << User::getUserCount() << "\n";

    const User readOnly("D");
    cout << "  const object can call a const method: " << readOnly.getName() << "\n";
}

} // namespace statics

// ===================================================================================
// 04 : ENCAPSULATION  (pillar 1)
// ===================================================================================
//  Bundle data + the methods that work on it, and hide the data behind methods so the
//  object can never reach an invalid state. private fields + public behaviour + validation.
//
//  Encapsulation is NOT "a getter and setter for every field" - that's a public field
//  with extra steps. Expose BEHAVIOUR, hide DATA:
//      BAD  : account.setBalance(account.getBalance() - 500);
//      GOOD : account.withdraw(500);
// -----------------------------------------------------------------------------------
namespace encapsulation {

class BankAccount {
private:
    string accountNumber;
    double balance;

public:
    BankAccount(string accountNumber, double opening)
        : accountNumber(accountNumber), balance(0) {
        deposit(opening);                                   // reuse the validated path
    }

    void deposit(double amount) {
        if (amount <= 0) throw invalid_argument("amount must be positive");
        balance += amount;
    }

    void withdraw(double amount) {
        if (amount <= 0)      throw invalid_argument("amount must be positive");
        if (amount > balance) throw runtime_error("insufficient balance");
        balance -= amount;
    }

    void transferTo(BankAccount& target, double amount) {
        withdraw(amount);
        target.deposit(amount);
    }

    double getBalance() const { return balance; }
};

void demo() {
    banner("04 ENCAPSULATION");

    BankAccount source("AC1", 1000), target("AC2", 100);
    source.transferTo(target, 400);
    cout << "  source=" << source.getBalance() << " target=" << target.getBalance() << "\n";

    try {
        source.withdraw(999999);
    } catch (const exception& e) {
        cout << "  invariant protected: " << e.what() << "\n";
    }
}

} // namespace encapsulation

// ===================================================================================
// 05 : ABSTRACTION  (pillar 2)
// ===================================================================================
//  Show only WHAT an object does, hide HOW. Done with abstract classes / interfaces.
//
//  PURE VIRTUAL FUNCTION : `virtual double area() const = 0;`
//  ABSTRACT CLASS  : has at least one pure virtual. CANNOT be instantiated. MAY have
//                    data members, a constructor, and normal concrete methods.
//  INTERFACE       : an abstract class with ONLY pure virtual functions, no data, and a
//                    virtual destructor. C++ has no `interface` keyword - this is the idiom.
//
//  Abstraction vs Encapsulation (asked constantly):
//     Abstraction   = design level, hides COMPLEXITY, exposes intent  (abstract class)
//     Encapsulation = code level,   hides DATA,       restricts access (private + methods)
// -----------------------------------------------------------------------------------
namespace abstraction {

class PaymentMethod {                                       // INTERFACE
public:
    virtual ~PaymentMethod() = default;                     // never forget this
    virtual bool   pay(double amount) = 0;
    virtual string name() const = 0;
};

class UpiPayment : public PaymentMethod {
public:
    bool pay(double amount) override { cout << "  paid " << amount << " via UPI\n"; return true; }
    string name() const override { return "UPI"; }
};

class CardPayment : public PaymentMethod {
public:
    bool pay(double amount) override { cout << "  paid " << amount << " via Card\n"; return true; }
    string name() const override { return "Card"; }
};

class Shape {                                               // ABSTRACT CLASS (has state too)
protected:
    string label;

public:
    explicit Shape(string label) : label(label) {}
    virtual ~Shape() = default;

    virtual double area() const = 0;                        // pure virtual

    void describe() const {                                 // concrete, shared by all
        cout << "  " << label << " area=" << area() << "\n";
    }
};

class Circle : public Shape {
private:
    double radius;

public:
    explicit Circle(double radius) : Shape("Circle"), radius(radius) {}
    double area() const override { return 3.14159 * radius * radius; }
};

class Rectangle : public Shape {
private:
    double width, height;

public:
    Rectangle(double width, double height)
        : Shape("Rectangle"), width(width), height(height) {}
    double area() const override { return width * height; }
};

void checkout(PaymentMethod& method, double amount) {        // depends on the ABSTRACTION
    cout << "  using " << method.name() << "\n";
    method.pay(amount);
}

void demo() {
    banner("05 ABSTRACTION");

    UpiPayment upi;
    CardPayment card;
    checkout(upi, 499);
    checkout(card, 1299);

    vector<unique_ptr<Shape>> shapes;
    shapes.push_back(make_unique<Circle>(2));
    shapes.push_back(make_unique<Rectangle>(3, 4));
    for (const auto& shape : shapes) shape->describe();

    // Shape s("x");   // ERROR: abstract class cannot be instantiated
}

} // namespace abstraction

// ===================================================================================
// 06 : INHERITANCE  (pillar 3)
// ===================================================================================
//  Derived class reuses / extends a base class. Relationship = "IS-A".
//
//  5 TYPES
//    1. Single        A <- B
//    2. Multilevel    A <- B <- C
//    3. Hierarchical  A <- B and A <- C
//    4. Multiple      A and B <- C
//    5. Hybrid        mix of the above -> can produce the DIAMOND problem
//
//  ACCESS TABLE (what the member becomes inside the derived class)
//  +-------------+----------------+-------------------+-----------------+
//  | base member | public inherit | protected inherit | private inherit |
//  +-------------+----------------+-------------------+-----------------+
//  | public      | public         | protected         | private         |
//  | protected   | protected      | protected         | private         |
//  | private     | not accessible | not accessible    | not accessible  |
//  +-------------+----------------+-------------------+-----------------+
//
//  ORDER : construction = Base -> members -> Derived body.  Destruction = exact reverse.
//
//  NOT INHERITED : constructors, destructor, copy-assignment operator, friends.
//  Private members ARE part of the object but are not accessible in the derived class.
//
//  DIAMOND PROBLEM
//    Person <- Student, Person <- Teacher, (Student, Teacher) <- TeachingAssistant
//    Without `virtual`, TA has TWO Person sub-objects -> `ta.name` is ambiguous and the
//    Person constructor runs twice.
//    FIX: `class Student : virtual public Person` -> exactly ONE shared Person, and the
//    most-derived class initialises it.
// -----------------------------------------------------------------------------------
namespace inheritance {

// ---- single + multilevel ----
class Animal {
protected:
    string name;

public:
    explicit Animal(string name) : name(name) { cout << "  Animal ctor\n"; }
    virtual ~Animal() { cout << "  Animal dtor\n"; }

    virtual void speak() const { cout << "  " << name << " makes a sound\n"; }
};

class Dog : public Animal {
public:
    explicit Dog(string name) : Animal(name) { cout << "  Dog ctor\n"; }
    ~Dog() override { cout << "  Dog dtor\n"; }

    void speak() const override { cout << "  " << name << " barks\n"; }
    void fetch() const { cout << "  " << name << " fetches\n"; }
};

class Puppy : public Dog {                                  // multilevel
public:
    explicit Puppy(string name) : Dog(name) { cout << "  Puppy ctor\n"; }
    ~Puppy() override { cout << "  Puppy dtor\n"; }

    void speak() const override { cout << "  " << name << " yips\n"; }
};

// ---- multiple inheritance + ambiguity ----
class Swimmer { public: void move() const { cout << "  swimming\n"; } };
class Flyer   { public: void move() const { cout << "  flying\n"; } };

class Duck : public Swimmer, public Flyer {
public:
    void move() const {                                     // must qualify - both have move()
        Swimmer::move();
        Flyer::move();
    }
};

// ---- diamond, fixed with virtual inheritance ----
class Person {
public:
    string name;

    explicit Person(string name = "anon") : name(name) {
        cout << "  Person ctor runs ONCE (virtual base)\n";
    }
    virtual ~Person() = default;
};

class Student : virtual public Person {
public:
    explicit Student(string name) : Person(name) {}
    void study() const { cout << "  " << name << " studies\n"; }
};

class Teacher : virtual public Person {
public:
    explicit Teacher(string name) : Person(name) {}
    void teach() const { cout << "  " << name << " teaches\n"; }
};

class TeachingAssistant : public Student, public Teacher {
public:
    explicit TeachingAssistant(string name)
        : Person(name), Student(name), Teacher(name) {}      // most-derived inits virtual base
};

void demo() {
    banner("06 INHERITANCE");

    cout << "-- multilevel: ctors top-down, dtors bottom-up --\n";
    {
        Puppy puppy("Bruno");
        puppy.speak();
        puppy.fetch();                                      // inherited from Dog
    }

    cout << "-- multiple inheritance ambiguity --\n";
    Duck().move();

    cout << "-- diamond solved --\n";
    TeachingAssistant ta("Meera");
    ta.study();
    ta.teach();
    cout << "  ta.name = " << ta.name << " (unambiguous)\n";
}

} // namespace inheritance

// ===================================================================================
// 07 : POLYMORPHISM  (pillar 4)
// ===================================================================================
//  A. COMPILE-TIME (static binding)  : function overloading, operator overloading, templates
//  B. RUN-TIME     (dynamic binding) : virtual functions called through a base pointer/ref
//
//  OVERLOADING vs OVERRIDING  <-- the single most asked OOP question
//  +----------------+--------------------------------+------------------------------+
//  |                | OVERLOADING                    | OVERRIDING                   |
//  +----------------+--------------------------------+------------------------------+
//  | signature      | must DIFFER                    | must be the SAME (incl const)|
//  | where          | same class                     | base + derived               |
//  | needs virtual  | no                             | yes                          |
//  | resolved       | compile time                   | run time (vtable)            |
//  | return type    | cannot overload on it alone    | same, or covariant pointer   |
//  +----------------+--------------------------------+------------------------------+
//
//  vtable / vptr : a class with any virtual function gets ONE vtable per class (array of
//  function pointers); each OBJECT gets a hidden vptr to it, which is why sizeof grows by
//  one pointer. `p->speak()` looks up the function through the vptr at run time.
//
//  MUST-KNOW RULES
//   1. Base class deleted through a base pointer -> destructor MUST be virtual.
//   2. OBJECT SLICING : copying a Derived into a Base BY VALUE loses the derived part and
//      polymorphism. Always use pointers or references.
//   3. Constructors are never virtual (no vptr exists yet). Use a virtual clone() instead.
//   4. `override` makes the compiler verify you really overrode something - always use it.
//   5. `final` stops further overriding.
//   6. A virtual call from inside a constructor dispatches to the BASE version.
//   7. Non-virtual function with the same name = HIDING, not overriding.
//   8. static / friend / non-member functions cannot be virtual.
//   9. dynamic_cast does a safe downcast (returns nullptr on failure) and needs a
//      polymorphic type.
// -----------------------------------------------------------------------------------
namespace polymorphism {

// ---- compile-time: overloading ----
class Calculator {
public:
    int    add(int a, int b)                     { return a + b; }
    double add(double a, double b)               { return a + b; }
    int    add(int a, int b, int c)              { return a + b + c; }
    string add(const string& a, const string& b) { return a + b; }
};

// ---- run-time: virtual dispatch ----
class Employee {
protected:
    string name;
    double baseSalary;

public:
    Employee(string name, double baseSalary) : name(name), baseSalary(baseSalary) {}
    virtual ~Employee() = default;                          // rule 1

    virtual double salary() const { return baseSalary; }
    virtual string role()   const { return "Employee"; }

    void payslip() const { cout << "  " << name << " (" << role() << ") = " << salary() << "\n"; }
};

class Developer : public Employee {
private:
    int overtimeHours;

public:
    Developer(string name, double baseSalary, int overtimeHours)
        : Employee(name, baseSalary), overtimeHours(overtimeHours) {}

    double salary() const override { return baseSalary + overtimeHours * 500; }
    string role()   const override { return "Developer"; }
};

class Manager : public Employee {
private:
    double bonus;

public:
    Manager(string name, double baseSalary, double bonus)
        : Employee(name, baseSalary), bonus(bonus) {}

    double salary() const override { return baseSalary + bonus; }
    string role()   const final { return "Manager"; }        // rule 5
};

// ---- object slicing ----
class BaseValue {
public:
    virtual ~BaseValue() = default;
    virtual string who() const { return "Base"; }
};

class DerivedValue : public BaseValue {
public:
    string who() const override { return "Derived"; }
};

void demo() {
    banner("07 POLYMORPHISM");

    cout << "-- overloading (compile time) --\n";
    Calculator calculator;
    cout << "  " << calculator.add(2, 3) << " " << calculator.add(2.5, 3.5)
         << " " << calculator.add(1, 2, 3) << " " << calculator.add("a", "b") << "\n";

    cout << "-- overriding (run time, one loop handles every type) --\n";
    vector<unique_ptr<Employee>> team;
    team.push_back(make_unique<Employee>("Asha", 40000));
    team.push_back(make_unique<Developer>("Ravi", 60000, 10));
    team.push_back(make_unique<Manager>("Neha", 90000, 25000));
    for (const auto& member : team) member->payslip();

    cout << "-- object slicing --\n";
    DerivedValue derived;
    BaseValue    sliced    = derived;                       // BY VALUE -> sliced
    BaseValue&   reference = derived;                       // BY REF   -> polymorphic
    cout << "  by value = " << sliced.who() << "  <-- lost\n";
    cout << "  by ref   = " << reference.who() << "\n";

    cout << "-- dynamic_cast --\n";
    Employee* pointer = team[1].get();
    if (Developer* developer = dynamic_cast<Developer*>(pointer))
        cout << "  it really is a Developer, salary=" << developer->salary() << "\n";
    if (!dynamic_cast<Manager*>(pointer))
        cout << "  and it is not a Manager (dynamic_cast returned nullptr)\n";

    cout << "  sizeof(BaseValue)=" << sizeof(BaseValue) << " (includes the hidden vptr)\n";
}

} // namespace polymorphism

// ===================================================================================
// 08 : OPERATOR OVERLOADING  (only what gets asked)
// ===================================================================================
//  Give your type natural syntax. The three you will actually be asked to write:
//     operator+   -> prefer a NON-MEMBER friend so conversions work on both sides
//     operator==  -> non-member friend, symmetric
//     operator<<  -> MUST be non-member (left operand is the stream), usually a friend
//  Also common: operator= (assignment), operator[] (const + non-const), operator()
//  (functor / comparator), operator++ (pre returns Type&, post takes a dummy int).
//
//  CANNOT be overloaded :  ::   .   .*   ?:   sizeof   typeid
// -----------------------------------------------------------------------------------
namespace operators {

class Complex {
private:
    double real, imaginary;

public:
    Complex(double real = 0, double imaginary = 0) : real(real), imaginary(imaginary) {}

    Complex& operator+=(const Complex& other) {
        real += other.real;
        imaginary += other.imaginary;
        return *this;
    }

    Complex& operator++()    { ++real; return *this; }                          // pre
    Complex  operator++(int) { Complex before = *this; ++real; return before; } // post

    friend Complex operator+(Complex left, const Complex& right) { return left += right; }

    friend bool operator==(const Complex& a, const Complex& b) {
        return a.real == b.real && a.imaginary == b.imaginary;
    }
    friend bool operator!=(const Complex& a, const Complex& b) { return !(a == b); }

    friend ostream& operator<<(ostream& out, const Complex& value) {
        return out << value.real << (value.imaginary < 0 ? " - " : " + ")
                   << abs(value.imaginary) << "i";
    }
};

void demo() {
    banner("08 OPERATOR OVERLOADING");

    Complex first(3, 4), second(1, -2);
    cout << "  first=" << first << "  second=" << second << "\n";
    cout << "  sum=" << (first + second) << "\n";
    cout << "  equal? " << (first == second ? "yes" : "no") << "\n";
    cout << "  post++ returns " << first++ << " then first=" << first << "\n";
}

} // namespace operators

// ===================================================================================
// 09 : friend
// ===================================================================================
//  `friend` lets a specific outside function or class touch private/protected members.
//   - Friendship is GRANTED by the class, not taken.
//   - NOT symmetric, NOT transitive, NOT inherited.
//   - A friend function is not a member: no `this`, not called with the dot operator.
//   - Legitimate uses: operator<< / operator>>, symmetric binary operators, a tightly
//     coupled helper class. It is the tightest coupling in C++ - use it sparingly.
// -----------------------------------------------------------------------------------
namespace friendship {

class Box {
private:
    int length, width;

public:
    Box(int length, int width) : length(length), width(width) {}

    friend int area(const Box& box);                        // friend FUNCTION
    friend class BoxInspector;                              // friend CLASS
};

int area(const Box& box) { return box.length * box.width; }  // reads private members

class BoxInspector {
public:
    void inspect(const Box& box) const {
        cout << "  inspector sees " << box.length << "x" << box.width << "\n";
    }
};

void demo() {
    banner("09 friend");

    Box box(4, 5);
    cout << "  area via friend function = " << area(box) << "\n";
    BoxInspector().inspect(box);
}

} // namespace friendship

// ===================================================================================
// 10 : COMPOSITION vs INHERITANCE + THE RELATIONSHIP VOCABULARY
// ===================================================================================
//  INHERITANCE  "IS-A"        Dog is an Animal.
//  COMPOSITION  "PART-OF"     strong ownership, SAME lifetime. Car owns its Engine; destroy
//                             the Car and the Engine dies. -> member by value / unique_ptr.
//  AGGREGATION  "HAS-A"       weak ownership, INDEPENDENT lifetime. Team has Players, but
//                             the Players outlive the Team. -> shared_ptr / pointer / ref.
//  ASSOCIATION  "USES/KNOWS"  mutual reference, no ownership. Doctor <-> Patient.
//  DEPENDENCY   "DEPENDS-ON"  appears only as a parameter / local / return type.
//
//  UML strength : dependency < association < aggregation < composition < inheritance
//
//  *** FAVOUR COMPOSITION OVER INHERITANCE ***  (say this in every LLD round)
//   - inheritance is fixed at compile time; composition can be swapped at run time
//   - inheritance exposes the base's protected internals (fragile base class problem)
//   - inheritance explodes combinatorially (MilkCoffee, CaramelCoffee, MilkCaramelCoffee)
//   - composition is far easier to unit-test (inject a fake)
//   Use inheritance ONLY for a genuine IS-A that satisfies the Liskov Substitution Principle.
// -----------------------------------------------------------------------------------
namespace relationships {

class Engine {
public:
    explicit Engine(int horsepower) : horsepower(horsepower) { cout << "    Engine built\n"; }
    ~Engine() { cout << "    Engine destroyed\n"; }

    void start() const { cout << "    engine running " << horsepower << "hp\n"; }

private:
    int horsepower;
};

class Car {                                                 // COMPOSITION
private:
    string registration;
    Engine engine;                                          // dies with the Car

public:
    Car(string registration, int horsepower) : registration(registration), engine(horsepower) {}

    void drive() const {
        cout << "  " << registration << " driving\n";
        engine.start();
    }
};

class Player {
private:
    string name;

public:
    explicit Player(string name) : name(name) {}
    const string& getName() const { return name; }
};

class Team {                                                // AGGREGATION
private:
    string name;
    vector<shared_ptr<Player>> players;                     // players outlive the team

public:
    explicit Team(string name) : name(name) {}

    void sign(const shared_ptr<Player>& player) { players.push_back(player); }

    void roster() const {
        cout << "  " << name << ":";
        for (const auto& player : players) cout << " " << player->getName();
        cout << "\n";
    }
};

void demo() {
    banner("10 COMPOSITION vs AGGREGATION");

    cout << "-- composition: engine dies with the car --\n";
    {
        Car car("MH12-AB-1234", 150);
        car.drive();
    }

    cout << "-- aggregation: players survive the team --\n";
    auto virat = make_shared<Player>("Virat");
    {
        Team team("Mumbai");
        team.sign(virat);
        team.sign(make_shared<Player>("Rohit"));
        team.roster();
    }
    cout << "  team gone, but player still alive: " << virat->getName() << "\n";
}

} // namespace relationships

// ===================================================================================
// 11 : SMART POINTERS + RAII
// ===================================================================================
//  RAII : tie a resource's lifetime to an object's lifetime - acquire in the constructor,
//  release in the destructor. The destructor runs on EVERY exit path, including an
//  exception, so nothing leaks and you never write manual cleanup.
//
//  unique_ptr<T>  sole ownership, move-only (cannot be copied), zero overhead. THE DEFAULT.
//                 make_unique<T>(args...)
//  shared_ptr<T>  shared ownership through an atomic reference count. Heavier.
//                 make_shared<T>(args...)  <- one allocation instead of two
//  weak_ptr<T>    non-owning observer of a shared_ptr. Call .lock() to use it.
//                 Its job is to BREAK REFERENCE CYCLES (parent<->child), which otherwise
//                 leak because neither refcount ever reaches zero.
//
//  Rules: never call delete yourself; never build two smart pointers from one raw pointer;
//  pass unique_ptr by value to transfer ownership, otherwise pass T& or const T&.
// -----------------------------------------------------------------------------------
namespace smartpointers {

class FileHandle {                                          // RAII
private:
    string name;

public:
    explicit FileHandle(string name) : name(name) { cout << "  OPEN  " << name << "\n"; }
    ~FileHandle() { cout << "  CLOSE " << name << "\n"; }

    void write(const string& line) const { cout << "  WRITE " << name << " <- " << line << "\n"; }
};

class Child;

class Parent {
public:
    vector<shared_ptr<Child>> children;
    ~Parent() { cout << "  ~Parent (no leak)\n"; }
};

class Child {
public:
    weak_ptr<Parent> parent;                                // weak breaks the cycle
    ~Child() { cout << "  ~Child (no leak)\n"; }
};

void demo() {
    banner("11 SMART POINTERS / RAII");

    cout << "-- RAII survives an exception --\n";
    {
        FileHandle file("audit.log");
        try {
            file.write("risky");
            throw runtime_error("boom");
        } catch (const exception& e) {
            cout << "  caught " << e.what() << " - file still closes below\n";
        }
    }

    cout << "-- unique_ptr is move-only --\n";
    unique_ptr<FileHandle> owner = make_unique<FileHandle>("a.log");
    unique_ptr<FileHandle> newOwner = std::move(owner);
    cout << "  old owner is " << (owner ? "set" : "null after move") << "\n";

    cout << "-- shared_ptr refcount --\n";
    shared_ptr<int> first = make_shared<int>(42);
    cout << "  count=" << first.use_count();
    {
        shared_ptr<int> second = first;
        cout << " -> inside scope " << first.use_count();
    }
    cout << " -> after scope " << first.use_count() << "\n";

    cout << "-- weak_ptr breaks the parent/child cycle --\n";
    {
        auto parent = make_shared<Parent>();
        auto child  = make_shared<Child>();
        parent->children.push_back(child);
        child->parent = parent;                             // weak, so both get destroyed
    }
}

} // namespace smartpointers

// ===================================================================================
// 12 : SOLID PRINCIPLES
// ===================================================================================
//  S  Single Responsibility  one class, one reason to change. If the description needs
//                            "and", split it.
//  O  Open / Closed          open to extension, closed to modification. Adding a variant
//                            should mean adding a class, not editing an if/switch chain.
//  L  Liskov Substitution    a subclass must work anywhere the base is expected, without
//                            the caller knowing. Violations: throwing "not supported",
//                            tightening a precondition, breaking a base invariant
//                            (Square deriving from a mutable Rectangle is the classic).
//  I  Interface Segregation  many small role interfaces beat one fat one; no class should
//                            be forced to stub out methods it cannot support.
//  D  Dependency Inversion   high-level code depends on an ABSTRACTION, not a concrete
//                            class, and the implementation is INJECTED from outside.
//                            DIP = principle, Dependency Injection = the technique.
// -----------------------------------------------------------------------------------
namespace solid {

// ---- S : split the responsibilities ----
class Invoice {                                             // business rules only
private:
    double amount, taxRate;

public:
    Invoice(double amount, double taxRate) : amount(amount), taxRate(taxRate) {}
    double total() const { return amount * (1 + taxRate); }
};

class InvoiceRepository {                                   // persistence only
public:
    void save(const Invoice& invoice) { cout << "  saved total=" << invoice.total() << "\n"; }
};

class InvoicePrinter {                                      // presentation only
public:
    void print(const Invoice& invoice) { cout << "  printed total=" << invoice.total() << "\n"; }
};

// ---- O : add a class, edit nothing ----
class DiscountPolicy {
public:
    virtual ~DiscountPolicy() = default;
    virtual double apply(double price) const = 0;
    virtual string name() const = 0;
};

class NoDiscount : public DiscountPolicy {
public:
    double apply(double price) const override { return price; }
    string name() const override { return "None"; }
};

class FestivalDiscount : public DiscountPolicy {
public:
    double apply(double price) const override { return price * 0.8; }
    string name() const override { return "Festival20"; }
};

class LoyaltyDiscount : public DiscountPolicy {             // NEW: nothing above changed
public:
    double apply(double price) const override { return price - 100; }
    string name() const override { return "Loyalty100"; }
};

// ---- L : violation and the fix ----
namespace lspViolation {
class Rectangle {
protected:
    int width = 0, height = 0;

public:
    virtual ~Rectangle() = default;
    virtual void setWidth(int width)   { this->width = width; }
    virtual void setHeight(int height) { this->height = height; }
    int area() const { return width * height; }
};

class Square : public Rectangle {
public:
    void setWidth(int side)  override { width = side; height = side; }  // surprises the caller
    void setHeight(int side) override { width = side; height = side; }
};

void clientCode(Rectangle& rectangle) {
    rectangle.setWidth(5);
    rectangle.setHeight(4);
    cout << "  expected 20, got " << rectangle.area()
         << (rectangle.area() == 20 ? "  (ok)" : "  <-- LSP VIOLATED") << "\n";
}
}

// ---- I : small interfaces ----
class Printer {
public:
    virtual ~Printer() = default;
    virtual void printDocument(const string& document) = 0;
};

class Scanner {
public:
    virtual ~Scanner() = default;
    virtual void scanDocument(const string& document) = 0;
};

class SimplePrinter : public Printer {                      // implements only what it can do
public:
    void printDocument(const string& document) override { cout << "  printing " << document << "\n"; }
};

class OfficeMachine : public Printer, public Scanner {
public:
    void printDocument(const string& document) override { cout << "  office print " << document << "\n"; }
    void scanDocument(const string& document)  override { cout << "  office scan "  << document << "\n"; }
};

// ---- D : inject the abstraction ----
class UserRepository {
public:
    virtual ~UserRepository() = default;
    virtual void save(const string& user) = 0;
};

class MySqlUserRepository : public UserRepository {
public:
    void save(const string& user) override { cout << "  mysql save " << user << "\n"; }
};

class FakeUserRepository : public UserRepository {           // trivial test double
public:
    void save(const string& user) override { cout << "  fake save " << user << " (for tests)\n"; }
};

class SignupService {
private:
    UserRepository& repository;                             // depends on the abstraction

public:
    explicit SignupService(UserRepository& repository) : repository(repository) {}
    void signup(const string& user) { repository.save(user); }
};

void demo() {
    banner("12 SOLID");

    cout << "-- S --\n";
    Invoice invoice(1000, 0.18);
    InvoiceRepository().save(invoice);
    InvoicePrinter().print(invoice);

    cout << "-- O --\n";
    vector<unique_ptr<DiscountPolicy>> policies;
    policies.push_back(make_unique<NoDiscount>());
    policies.push_back(make_unique<FestivalDiscount>());
    policies.push_back(make_unique<LoyaltyDiscount>());
    for (const auto& policy : policies)
        cout << "  " << policy->name() << " on 1000 -> " << policy->apply(1000) << "\n";

    cout << "-- L --\n";
    lspViolation::Rectangle rectangle;
    lspViolation::Square    square;
    lspViolation::clientCode(rectangle);
    lspViolation::clientCode(square);
    cout << "  fix: make Rectangle and Square siblings under an immutable Shape\n";

    cout << "-- I --\n";
    SimplePrinter().printDocument("resume.pdf");
    OfficeMachine().scanDocument("contract.pdf");

    cout << "-- D --\n";
    MySqlUserRepository production;
    FakeUserRepository  fake;
    SignupService productionService(production);
    SignupService testService(fake);
    productionService.signup("ravindra");
    testService.signup("test-user");
}

} // namespace solid

// ===================================================================================
// 13 : THE 6 DESIGN PATTERNS AN SDE-1 IS ACTUALLY ASKED
// ===================================================================================
//  CREATIONAL : Singleton, Factory, Builder
//  STRUCTURAL : Decorator, Adapter
//  BEHAVIOURAL: Strategy, Observer
//  (Learn these 7 well. Composite / Command / State / Facade are nice-to-have next.)
// -----------------------------------------------------------------------------------
namespace patterns {

// ---------------- 1. SINGLETON ----------------
//  One instance + a global access point. Use for config / logger / connection pool.
//  Implementation: private constructor, deleted copy, static accessor.
//  The function-local `static` below is the "Meyers singleton" - lazy AND thread-safe
//  since C++11, because static local initialisation is guaranteed to happen exactly once.
//  Downside (say this): global mutable state, hidden dependencies, hard to unit-test.
class Logger {
private:
    Logger() = default;

public:
    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;

    static Logger& getInstance() {
        static Logger only;
        return only;
    }

    void log(const string& message) { cout << "  [log] " << message << "\n"; }
};

// ---------------- 2. FACTORY ----------------
//  Move the `new` decision into one place so the client never names a concrete class.
//  Client depends only on the Shape interface -> adding a Triangle touches one function.
class Shape {
public:
    virtual ~Shape() = default;
    virtual void draw() const = 0;
};

class Circle   : public Shape { public: void draw() const override { cout << "  drawing Circle\n"; } };
class Square   : public Shape { public: void draw() const override { cout << "  drawing Square\n"; } };
class Triangle : public Shape { public: void draw() const override { cout << "  drawing Triangle\n"; } };

class ShapeFactory {
public:
    static unique_ptr<Shape> create(const string& type) {
        if (type == "circle")   return make_unique<Circle>();
        if (type == "square")   return make_unique<Square>();
        if (type == "triangle") return make_unique<Triangle>();
        throw invalid_argument("unknown shape: " + type);
    }
};

// ---------------- 3. BUILDER ----------------
//  Fixes the "constructor with 8 optional parameters" problem, gives readable call sites,
//  and validates the whole object once in build(). Each setter returns *this (fluent).
struct PizzaParts {
    string size = "medium";
    bool   cheese = false;
    bool   olives = false;
    int    toppings = 0;
};

class Pizza {
private:
    PizzaParts parts;

    explicit Pizza(PizzaParts parts) : parts(parts) {}

public:
    class Builder {
    private:
        PizzaParts parts;

    public:
        Builder& size(const string& size) { parts.size = size; return *this; }
        Builder& addCheese()  { parts.cheese = true; ++parts.toppings; return *this; }
        Builder& addOlives()  { parts.olives = true; ++parts.toppings; return *this; }

        Pizza build() const {
            if (parts.size != "small" && parts.size != "medium" && parts.size != "large")
                throw invalid_argument("bad size");
            return Pizza(parts);
        }
    };

    void describe() const {
        cout << "  " << parts.size << " pizza, cheese=" << parts.cheese
             << " olives=" << parts.olives << " toppings=" << parts.toppings << "\n";
    }
};

// ---------------- 4. STRATEGY ----------------
//  Pull an algorithm out behind an interface so it can be swapped at run time.
//  This is the standard answer to "how do you avoid a big if/else chain?" (Open/Closed).
class PaymentStrategy {
public:
    virtual ~PaymentStrategy() = default;
    virtual void pay(double amount) const = 0;
    virtual string name() const = 0;
};

class UpiStrategy : public PaymentStrategy {
public:
    void pay(double amount) const override { cout << "  " << amount << " via UPI\n"; }
    string name() const override { return "UPI"; }
};

class CardStrategy : public PaymentStrategy {
public:
    void pay(double amount) const override { cout << "  " << amount << " via Card\n"; }
    string name() const override { return "Card"; }
};

class Checkout {                                            // the CONTEXT
private:
    unique_ptr<PaymentStrategy> strategy;

public:
    explicit Checkout(unique_ptr<PaymentStrategy> strategy) : strategy(std::move(strategy)) {}

    void setStrategy(unique_ptr<PaymentStrategy> replacement) { strategy = std::move(replacement); }

    void pay(double amount) const {
        cout << "  paying with " << strategy->name() << "\n";
        strategy->pay(amount);
    }
};

// ---------------- 5. OBSERVER ----------------
//  Publish / subscribe: one subject notifies many observers, and neither knows the other's
//  concrete type. Used for notifications, event buses, UI updates, cache invalidation.
class Observer {
public:
    virtual ~Observer() = default;
    virtual void notify(const string& event) = 0;
};

class EmailObserver : public Observer {
public:
    void notify(const string& event) override { cout << "    email: " << event << "\n"; }
};

class SmsObserver : public Observer {
public:
    void notify(const string& event) override { cout << "    sms: " << event << "\n"; }
};

class OrderService {                                        // the SUBJECT
private:
    vector<shared_ptr<Observer>> observers;

public:
    void subscribe(const shared_ptr<Observer>& observer) { observers.push_back(observer); }

    void unsubscribe(const shared_ptr<Observer>& target) {
        observers.erase(remove(observers.begin(), observers.end(), target), observers.end());
    }

    void placeOrder(const string& orderId) {
        cout << "  order " << orderId << " placed -> notifying " << observers.size() << "\n";
        for (const auto& observer : observers) observer->notify("order " + orderId);
    }
};

// ---------------- 6. DECORATOR ----------------
//  Add behaviour to ONE object at run time by wrapping it, instead of creating a subclass
//  for every combination. Wrapper implements the same interface and holds the wrappee.
class Coffee {
public:
    virtual ~Coffee() = default;
    virtual double cost() const = 0;
    virtual string description() const = 0;
};

class Espresso : public Coffee {
public:
    double cost() const override { return 80; }
    string description() const override { return "Espresso"; }
};

class CoffeeDecorator : public Coffee {
protected:
    unique_ptr<Coffee> inner;

public:
    explicit CoffeeDecorator(unique_ptr<Coffee> inner) : inner(std::move(inner)) {}
};

class WithMilk : public CoffeeDecorator {
public:
    using CoffeeDecorator::CoffeeDecorator;
    double cost() const override { return inner->cost() + 20; }
    string description() const override { return inner->description() + " + milk"; }
};

class WithCaramel : public CoffeeDecorator {
public:
    using CoffeeDecorator::CoffeeDecorator;
    double cost() const override { return inner->cost() + 35; }
    string description() const override { return inner->description() + " + caramel"; }
};

// ---------------- 7. ADAPTER ----------------
//  Make an incompatible existing class fit the interface your code expects.
//  DECORATOR keeps the interface and adds behaviour. ADAPTER changes the interface.
class JsonSender {
public:
    virtual ~JsonSender() = default;
    virtual void send(const string& json) = 0;
};

class LegacyXmlSender {                                     // the class we cannot change
public:
    void sendXml(const string& xml) const { cout << "  legacy sent " << xml << "\n"; }
};

class XmlSenderAdapter : public JsonSender {
private:
    LegacyXmlSender& legacy;

public:
    explicit XmlSenderAdapter(LegacyXmlSender& legacy) : legacy(legacy) {}

    void send(const string& json) override { legacy.sendXml("<data>" + json + "</data>"); }
};

void demo() {
    banner("13 DESIGN PATTERNS");

    cout << "-- Singleton --\n";
    Logger::getInstance().log("app started");
    cout << "  same instance? "
         << (&Logger::getInstance() == &Logger::getInstance() ? "yes" : "no") << "\n";

    cout << "-- Factory --\n";
    for (const string& type : {"circle", "square", "triangle"})
        ShapeFactory::create(type)->draw();
    try {
        ShapeFactory::create("hexagon");
    } catch (const exception& e) {
        cout << "  " << e.what() << "\n";
    }

    cout << "-- Builder --\n";
    Pizza pizza = Pizza::Builder().size("large").addCheese().addOlives().build();
    pizza.describe();

    cout << "-- Strategy (swapped at run time) --\n";
    Checkout checkout(make_unique<UpiStrategy>());
    checkout.pay(499);
    checkout.setStrategy(make_unique<CardStrategy>());
    checkout.pay(1299);

    cout << "-- Observer --\n";
    OrderService orderService;
    auto email = make_shared<EmailObserver>();
    auto sms   = make_shared<SmsObserver>();
    orderService.subscribe(email);
    orderService.subscribe(sms);
    orderService.placeOrder("ORD-1");
    orderService.unsubscribe(sms);
    orderService.placeOrder("ORD-2");

    cout << "-- Decorator (stacked) --\n";
    unique_ptr<Coffee> order = make_unique<Espresso>();
    order = make_unique<WithMilk>(std::move(order));
    order = make_unique<WithCaramel>(std::move(order));
    cout << "  " << order->description() << " = " << order->cost() << "\n";

    cout << "-- Adapter --\n";
    LegacyXmlSender legacy;
    XmlSenderAdapter adapter(legacy);
    adapter.send("{\"event\":\"login\"}");
}

} // namespace patterns

// ===================================================================================
// 14 : RAPID-FIRE Q&A  --  READ THIS THE NIGHT BEFORE
// ===================================================================================
//  Q  4 pillars of OOP?
//  A  Encapsulation, Abstraction, Inheritance, Polymorphism.
//
//  Q  Abstraction vs Encapsulation?
//  A  Abstraction hides COMPLEXITY and shows only what an object does (abstract class /
//     interface, design level). Encapsulation hides DATA and controls access
//     (private members + public methods, implementation level).
//
//  Q  Overloading vs Overriding?
//  A  Overloading: same name, DIFFERENT parameters, same class, resolved at COMPILE time.
//     Overriding : same name, SAME signature, base virtual + derived, resolved at RUN time.
//     You cannot overload on return type alone.
//
//  Q  What is a virtual function?
//  A  A member function that can be redefined in a derived class and is dispatched at
//     run time through the vtable, so a base pointer calls the derived version.
//
//  Q  Why do we need a virtual destructor?
//  A  So that `delete basePtr` on a Derived object also runs the derived destructor.
//     Without it only the base destructor runs -> resource leak / undefined behaviour.
//
//  Q  Can a constructor be virtual?
//  A  No. The vptr is set up by the constructor itself, so no dynamic type exists yet.
//     For "virtual construction" use a virtual clone() or a factory.
//
//  Q  Can a class have more than one destructor?
//  A  No. One destructor, no parameters, cannot be overloaded.
//
//  Q  Abstract class vs Interface in C++?
//  A  Abstract class: at least one pure virtual, MAY also have data and concrete methods.
//     Interface: an abstract class with ONLY pure virtuals, no data, virtual destructor.
//     C++ has no `interface` keyword.
//
//  Q  Can an abstract class have a constructor?
//  A  Yes - it runs when a derived object is created, to initialise the base's data.
//
//  Q  What is the diamond problem and how do you fix it?
//  A  Two inheritance paths to the same base duplicate that base sub-object and make its
//     members ambiguous. Fix: `virtual` inheritance in the intermediate classes, so there
//     is a single shared sub-object initialised by the most-derived class.
//
//  Q  What is object slicing?
//  A  Assigning a Derived to a Base BY VALUE keeps only the base part, so polymorphism is
//     lost. Use pointers or references instead.
//
//  Q  What is a vtable?
//  A  One table of virtual function pointers per class; each object stores a hidden vptr
//     to its class's vtable. Cost = one extra pointer per object + one indirection per call.
//
//  Q  Shallow copy vs deep copy?
//  A  Shallow copies the pointer (two objects share one buffer, both delete it -> crash).
//     Deep allocates new memory and copies the contents. The compiler-generated copy
//     constructor is shallow for raw pointers - hence the Rule of Three.
//
//  Q  Rule of Three / Five / Zero?
//  A  Three: if you write a destructor, copy ctor, or copy assignment, you likely need all
//     three. Five: also the move constructor and move assignment. Zero: own no raw
//     resources (use string/vector/smart pointers) and write none of them - preferred.
//
//  Q  Why does the copy constructor take a reference?
//  A  Passing by value would require making a copy to construct the copy -> infinite
//     recursion. Signature: Foo(const Foo&).
//
//  Q  static member function vs const member function?
//  A  static: no `this`, shared across objects, called as Class::f(), can only use static
//     members. const: has `this` as pointer-to-const, promises not to modify the object,
//     and is the only kind callable on a const object.
//
//  Q  What is `this`?
//  A  An implicit pointer to the current object; used to disambiguate names and to return
//     *this for chaining.
//
//  Q  Difference between struct and class in C++?
//  A  Only the default access level (struct public, class private) and default inheritance
//     mode. Both support constructors, virtual functions, and inheritance.
//
//  Q  Composition vs inheritance - which do you prefer and why?
//  A  Composition. It is swappable at run time, does not expose base internals, avoids the
//     subclass explosion, and is easier to test. Use inheritance only for a true IS-A.
//
//  Q  What is SOLID?
//  A  Single Responsibility, Open/Closed, Liskov Substitution, Interface Segregation,
//     Dependency Inversion. (Be able to give a one-line example of each.)
//
//  Q  What is dependency injection?
//  A  Passing an object's collaborators in from outside (usually via the constructor)
//     instead of creating them inside. It is how you implement Dependency Inversion, and
//     it is what makes a class unit-testable.
//
//  Q  unique_ptr vs shared_ptr vs weak_ptr?
//  A  unique = single owner, move-only, no overhead. shared = shared ownership with an
//     atomic reference count. weak = non-owning observer used to break reference cycles.
//
//  Q  What is RAII?
//  A  Resource Acquisition Is Initialization: acquire a resource in the constructor and
//     release it in the destructor, so every exit path (including exceptions) cleans up.
//
//  Q  Strategy vs Factory pattern?
//  A  Factory decides WHICH object to CREATE. Strategy decides WHICH ALGORITHM to RUN, and
//     it can be swapped on an existing object at run time.
//
//  Q  Why is Singleton often called an anti-pattern?
//  A  It is global mutable state: it hides dependencies, makes unit testing hard, and its
//     lifetime/initialisation order is awkward. Prefer dependency injection.
//
//  Q  Which functions cannot be virtual?
//  A  Constructors, static member functions, and friend / non-member functions.
//
//  Q  Access specifiers and inheritance - what happens to a public member under private
//     inheritance?
//  A  It becomes private in the derived class. (Know the full 3x3 table in section 06.)
// ===================================================================================

int main() {
    basics::demo();
    ctors::demo();
    statics::demo();
    encapsulation::demo();
    abstraction::demo();
    inheritance::demo();
    polymorphism::demo();
    operators::demo();
    friendship::demo();
    relationships::demo();
    smartpointers::demo();
    solid::demo();
    patterns::demo();

    banner("DONE");
    cout << "Section 14 is the rapid-fire Q&A - read it before the interview.\n";
    return 0;
}
