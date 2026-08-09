// ===================================================================================
//                    OOPS MASTER REVISION FILE  (C++17)  --  FOR LLD & INTERVIEWS
// ===================================================================================
//  Compile : g++ -std=c++17 -pthread OOPS_MASTER_REVISION.cpp -o oops && ./oops
//
//  CONTENTS
//  --------
//  PART 00  Class, Object, Access Specifiers, this, Struct vs Class
//  PART 01  Constructors, Destructors, Rule of 0/3/5, Copy vs Move, Shallow vs Deep
//  PART 02  Static members, const-correctness, mutable, initialization order
//  PART 03  ENCAPSULATION  (Pillar 1)
//  PART 04  ABSTRACTION    (Pillar 2) - abstract class vs interface
//  PART 05  INHERITANCE    (Pillar 3) - all 5 types, access table, diamond, virtual base
//  PART 06  POLYMORPHISM   (Pillar 4) - compile-time + run-time, vtable, pitfalls
//  PART 07  Operator overloading (full set)
//  PART 08  Friend functions & classes
//  PART 09  Templates, generics, CRTP (static polymorphism)
//  PART 10  RAII + smart pointers + pImpl
//  PART 11  Object relationships: Association / Aggregation / Composition / Dependency
//  PART 12  SOLID principles (bad vs good)
//  PART 13  Other design principles (DRY, KISS, YAGNI, LoD, favour composition...)
//  PART 14  UML cheat-sheet
//  PART 15  Design patterns - Creational
//  PART 16  Design patterns - Structural
//  PART 17  Design patterns - Behavioural
//  PART 18  Mini LLD walk-through (Parking Lot) + interview method
//  PART 19  Rapid-fire interview Q&A
// ===================================================================================

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <typeinfo>
#include <sstream>
#include <mutex>
#include <utility>
#include <cstring>

using namespace std;

static void banner(const string& title) {
    cout << "\n================ " << title << " ================\n";
}

// ===================================================================================
// PART 00 : CLASS, OBJECT, ACCESS SPECIFIERS, this, STRUCT vs CLASS
// ===================================================================================
//  CLASS  = blueprint / user-defined type (attributes + behaviour). No memory until used.
//  OBJECT = instance of a class, occupies memory.
//
//  ACCESS SPECIFIERS
//    private   -> only the class itself + its friends            (default for class)
//    protected -> class itself + friends + derived classes
//    public    -> everyone                                       (default for struct)
//
//  struct vs class in C++ : ONLY difference is default access (public vs private) and
//  default inheritance mode. Everything else (ctors, virtual, inheritance) is identical.
//
//  `this` = implicit pointer to the current object. Used to disambiguate names,
//           to return *this for chaining, and to pass the current object out.
// -----------------------------------------------------------------------------------
namespace part00 {

class Employee {
private:
    string name;            // private : hidden state
    double salary;

protected:
    string department;      // visible to derived classes

public:
    int employeeId;         // public : open

    Employee(string name, double salary, string department, int employeeId)
        : name(name), salary(salary), department(department), employeeId(employeeId) {}

    // `this` disambiguates member vs parameter
    Employee& setName(const string& name) {
        this->name = name;
        return *this;                       // return *this -> method chaining
    }

    Employee& setSalary(double salary) {
        this->salary = salary;
        return *this;
    }

    void print() const {
        cout << "[" << employeeId << "] " << name
             << " | " << department << " | " << salary << "\n";
    }
};

void demo() {
    banner("PART 00 : class / object / this");

    Employee e("Ravindra", 100000, "Engineering", 1);
    e.setName("Ravindra S").setSalary(120000).print();   // chaining via *this

    // e.name = "x";   // COMPILE ERROR : private
    e.employeeId = 7;                                    // ok : public
    e.print();

    cout << "sizeof(Employee) = " << sizeof(Employee)
         << "  (member functions live once in code segment, not per object)\n";
}

} // namespace part00

// ===================================================================================
// PART 01 : CONSTRUCTORS, DESTRUCTOR, RULE OF 0/3/5, COPY vs MOVE, SHALLOW vs DEEP
// ===================================================================================
//  CONSTRUCTOR : same name as class, no return type, runs automatically on creation.
//    - Default              Foo()
//    - Parameterized        Foo(int)
//    - Copy                 Foo(const Foo&)
//    - Move                 Foo(Foo&&)            [C++11]
//    - Delegating           Foo() : Foo(0) {}     [C++11]
//    - explicit             blocks implicit one-arg conversions
//    - = default / = delete
//    - Can be overloaded. CANNOT be virtual. CAN be private (Singleton / named ctor).
//
//  MEMBER INITIALIZER LIST is required for: const members, references, base classes
//  without default ctor, and members without default ctor. It is also faster
//  (direct-initialize instead of default-construct + assign).
//
//  DESTRUCTOR : ~Foo(), no args, cannot be overloaded, SHOULD be virtual in a base
//  class that is deleted polymorphically.
//
//  RULE OF THREE : if you write any of { destructor, copy ctor, copy assignment }
//                  you probably need all three.
//  RULE OF FIVE  : + move ctor + move assignment.
//  RULE OF ZERO  : best - own no raw resources, use smart pointers / containers,
//                  then write NONE of the five and let the compiler generate them.
// -----------------------------------------------------------------------------------
namespace part01 {

// ---------- Rule of Five : a class that owns a raw resource ----------
class Buffer {
private:
    char*  data;
    size_t size;

public:
    explicit Buffer(const char* text)                       // 1. parameterized
        : size(strlen(text)), data(new char[strlen(text) + 1]) {
        strcpy(data, text);
        cout << "  ctor        (" << data << ")\n";
    }

    Buffer(const Buffer& other)                             // 2. copy ctor (DEEP copy)
        : size(other.size), data(new char[other.size + 1]) {
        strcpy(data, other.data);
        cout << "  copy ctor   (" << data << ")\n";
    }

    Buffer& operator=(const Buffer& other) {                // 3. copy assignment
        cout << "  copy assign (" << other.data << ")\n";
        if (this == &other) return *this;                   // self-assignment guard
        char* fresh = new char[other.size + 1];             // allocate BEFORE freeing
        strcpy(fresh, other.data);
        delete[] data;
        data = fresh;
        size = other.size;
        return *this;
    }

    Buffer(Buffer&& other) noexcept                         // 4. move ctor (steal)
        : data(other.data), size(other.size) {
        other.data = nullptr;                               // leave source valid+empty
        other.size = 0;
        cout << "  move ctor\n";
    }

    Buffer& operator=(Buffer&& other) noexcept {            // 5. move assignment
        cout << "  move assign\n";
        if (this == &other) return *this;
        delete[] data;
        data = other.data;
        size = other.size;
        other.data = nullptr;
        other.size = 0;
        return *this;
    }

    ~Buffer() {                                             // destructor
        cout << "  dtor        (" << (data ? data : "moved-out") << ")\n";
        delete[] data;
    }

    const char* text() const { return data ? data : ""; }
};

// ---------- delegating ctor, explicit, = default, = delete ----------
class Temperature {
private:
    double celsius;

public:
    Temperature() : Temperature(0.0) {}                     // delegating ctor
    explicit Temperature(double celsius) : celsius(celsius) {}

    double value() const { return celsius; }
};

class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&)            = delete;    // copying forbidden
    NonCopyable& operator=(const NonCopyable&) = delete;
};

void demo() {
    banner("PART 01 : constructors / rule of five");

    cout << "-- deep copy --\n";
    Buffer a("hello");
    Buffer b = a;                       // copy ctor
    Buffer c("tmp");
    c = a;                              // copy assignment

    cout << "-- move (no allocation, pointer stolen) --\n";
    Buffer d = std::move(a);            // move ctor ; `a` is now empty but destructible

    cout << "-- explicit blocks implicit conversion --\n";
    Temperature t1;                     // delegates -> 0.0
    Temperature t2(36.6);
    // Temperature t3 = 36.6;           // COMPILE ERROR because ctor is explicit
    cout << "  t1=" << t1.value() << "  t2=" << t2.value() << "\n";

    cout << "-- scope exit : destructors run in REVERSE order of construction --\n";
}

} // namespace part01

// ===================================================================================
// PART 02 : STATIC MEMBERS, const-CORRECTNESS, mutable, INITIALIZATION ORDER
// ===================================================================================
//  static data member   : one copy shared by ALL objects. Belongs to the class.
//  static member func   : no `this`, can only touch static members. Called as Class::f().
//                         Used for factory methods, counters, Singleton::getInstance().
//  const member func    : promises not to modify observable state -> callable on const
//                         objects. Can be overloaded on constness.
//  mutable              : member allowed to change inside a const function (caches, mutex).
//
//  MEMBERS ARE INITIALIZED IN DECLARATION ORDER, *not* in initializer-list order.
// -----------------------------------------------------------------------------------
namespace part02 {

class IdGenerator {
private:
    static int counter;                                     // declaration
    int id;

public:
    IdGenerator() : id(++counter) {}

    int value() const { return id; }
    static int totalCreated() { return counter; }           // static: no `this`
};

int IdGenerator::counter = 0;                               // definition (once, outside)

class Report {
private:
    vector<int> rows;
    mutable string cache;                                   // mutable -> writable in const fn
    mutable bool cacheValid = false;

public:
    void add(int row) { rows.push_back(row); cacheValid = false; }

    int&       operator[](size_t i)       { return rows[i]; }        // non-const version
    const int& operator[](size_t i) const { return rows[i]; }        // const version

    const string& render() const {                          // const, yet fills the cache
        if (!cacheValid) {
            ostringstream out;
            for (int r : rows) out << r << " ";
            cache = out.str();
            cacheValid = true;
        }
        return cache;
    }
};

class OrderInitialization {
private:
    int first;
    int second;

public:
    // written second-then-first, but `first` is STILL initialized first
    OrderInitialization(int seed) : second(seed * 2), first(seed) {}

    void print() const { cout << "  first=" << first << " second=" << second << "\n"; }
};

void demo() {
    banner("PART 02 : static / const / mutable");

    IdGenerator a, b, c;
    cout << "  ids: " << a.value() << " " << b.value() << " " << c.value()
         << "  total=" << IdGenerator::totalCreated() << "\n";

    Report r;
    r.add(10); r.add(20); r.add(30);
    const Report& readOnly = r;
    cout << "  const render : " << readOnly.render() << "\n";
    cout << "  const [] : " << readOnly[1] << "\n";
    r[1] = 99;                                              // non-const overload
    cout << "  after write  : " << r.render() << "\n";

    OrderInitialization o(5);
    o.print();
}

} // namespace part02

// ===================================================================================
// PART 03 : ENCAPSULATION  (PILLAR 1)
// ===================================================================================
//  DEFINITION : bundling data + the methods that operate on it into one unit, and
//               restricting direct access to the internal state.
//  GOAL       : protect INVARIANTS. The object is always in a legal state because the
//               only way to change it is through methods that enforce the rules.
//  HOW        : private data + public methods (getters/setters/behaviour) + validation.
//
//  Encapsulation is NOT "write a getter and a setter for every field". That is just a
//  public field with extra steps. Expose BEHAVIOUR, hide DATA:
//      BAD  : account.setBalance(account.getBalance() - 500);
//      GOOD : account.withdraw(500);
//
//  Encapsulation vs Abstraction:
//      Abstraction    = design level, hide COMPLEXITY, expose WHAT ("what can it do")
//      Encapsulation  = implementation level, hide DATA,  restrict access ("how it hides")
// -----------------------------------------------------------------------------------
namespace part03 {

class BankAccount {
private:
    string accountNumber;
    double balance;
    vector<string> statement;                               // internal, never exposed raw

    void record(const string& line) { statement.push_back(line); }

public:
    BankAccount(string accountNumber, double openingBalance)
        : accountNumber(std::move(accountNumber)), balance(0) {
        deposit(openingBalance);                            // reuse the validated path
    }

    void deposit(double amount) {
        if (amount <= 0) throw invalid_argument("deposit must be positive");
        balance += amount;
        record("credit " + to_string(amount));
    }

    void withdraw(double amount) {
        if (amount <= 0)      throw invalid_argument("withdraw must be positive");
        if (amount > balance) throw runtime_error("insufficient balance");
        balance -= amount;
        record("debit " + to_string(amount));
    }

    void transferTo(BankAccount& target, double amount) {    // behaviour, not raw setters
        withdraw(amount);
        target.deposit(amount);
    }

    double getBalance() const { return balance; }            // read-only view

    vector<string> getStatement() const { return statement; }// copy, caller cannot mutate us
};

void demo() {
    banner("PART 03 : ENCAPSULATION");

    BankAccount source("AC-1", 1000);
    BankAccount target("AC-2", 100);

    source.transferTo(target, 400);
    cout << "  source=" << source.getBalance() << "  target=" << target.getBalance() << "\n";

    try {
        source.withdraw(999999);                            // invariant protected
    } catch (const exception& e) {
        cout << "  blocked: " << e.what() << "\n";
    }

    for (const string& line : source.getStatement()) cout << "  stmt: " << line << "\n";
}

} // namespace part03

// ===================================================================================
// PART 04 : ABSTRACTION  (PILLAR 2)
// ===================================================================================
//  DEFINITION : expose only the essential contract, hide the implementation detail.
//
//  In C++ there are two flavours:
//    1. Abstract class  : has >=1 pure virtual function (`virtual f() = 0;`).
//                         MAY also carry state + concrete helper methods.
//                         Cannot be instantiated; a derived class must override all
//                         pure virtuals to become concrete.
//    2. Interface       : an abstract class with ONLY pure virtual functions, no data,
//                         and a virtual (usually defaulted) destructor. C++ has no
//                         `interface` keyword - this is the idiom.
//
//  Notes / gotchas
//    - A pure virtual function CAN have a body : `void f() = 0;` + `void B::f(){}` .
//      Derived classes must still override it, but may call B::f() explicitly.
//    - A pure virtual DESTRUCTOR is legal and forces the class to be abstract, but you
//      MUST define it out of line, because the derived dtor always calls it.
//    - You cannot create an object of an abstract class, but you CAN have a pointer or
//      reference to it - that is exactly how polymorphism is used.
//    - Prefer depending on the abstraction (DIP), not the concrete class.
// -----------------------------------------------------------------------------------
namespace part04 {

// ---------- pure interface ----------
class PaymentGateway {
public:
    virtual ~PaymentGateway() = default;                    // virtual dtor is mandatory
    virtual bool  charge(double amount)  = 0;
    virtual void  refund(double amount)  = 0;
    virtual string name() const          = 0;
};

class RazorpayGateway : public PaymentGateway {
public:
    bool charge(double amount) override {
        cout << "  Razorpay charging " << amount << "\n";
        return true;
    }
    void refund(double amount) override { cout << "  Razorpay refunding " << amount << "\n"; }
    string name() const override { return "Razorpay"; }
};

class StripeGateway : public PaymentGateway {
public:
    bool charge(double amount) override {
        cout << "  Stripe charging " << amount << "\n";
        return true;
    }
    void refund(double amount) override { cout << "  Stripe refunding " << amount << "\n"; }
    string name() const override { return "Stripe"; }
};

// ---------- abstract class WITH shared state + a template-method style helper ----------
class Vehicle {
protected:
    string registration;
    int    wheels;

public:
    Vehicle(string registration, int wheels)
        : registration(std::move(registration)), wheels(wheels) {}

    virtual ~Vehicle() = default;

    virtual double tollCharge() const = 0;                  // pure virtual : subclass decides
    virtual string category()   const = 0;

    void printToll() const {                                // concrete, reused by all
        cout << "  " << registration << " (" << category() << ", " << wheels
             << "w) toll = " << tollCharge() << "\n";
    }
};

class Bike : public Vehicle {
public:
    explicit Bike(string reg) : Vehicle(std::move(reg), 2) {}
    double tollCharge() const override { return 20.0; }
    string category()   const override { return "Bike"; }
};

class Truck : public Vehicle {
private:
    double tonnage;

public:
    Truck(string reg, double tonnage) : Vehicle(std::move(reg), 6), tonnage(tonnage) {}
    double tollCharge() const override { return 100.0 + 15.0 * tonnage; }
    string category()   const override { return "Truck"; }
};

// ---------- pure virtual function WITH a body ----------
class Logger {
public:
    virtual ~Logger() = default;
    virtual void log(const string& message) = 0;            // pure, but has a definition
};

void Logger::log(const string& message) {                   // shared default behaviour
    cout << "  [base] " << message << "\n";
}

class TimestampLogger : public Logger {
public:
    void log(const string& message) override {
        Logger::log("2026-08-06 | " + message);             // explicitly call the pure base
    }
};

void checkout(PaymentGateway& gateway, double amount) {      // depends on the ABSTRACTION
    cout << "  using " << gateway.name() << "\n";
    gateway.charge(amount);
}

void demo() {
    banner("PART 04 : ABSTRACTION (abstract class vs interface)");

    RazorpayGateway razorpay;
    StripeGateway   stripe;
    checkout(razorpay, 499);
    checkout(stripe,   799);

    vector<unique_ptr<Vehicle>> vehicles;
    vehicles.push_back(make_unique<Bike>("MH12-AB-1234"));
    vehicles.push_back(make_unique<Truck>("MH14-XY-9999", 12.5));
    for (const auto& vehicle : vehicles) vehicle->printToll();

    TimestampLogger logger;
    logger.log("abstraction demo done");

    // Vehicle v("x", 4);   // COMPILE ERROR : abstract class cannot be instantiated
}

} // namespace part04

// ===================================================================================
// PART 05 : INHERITANCE  (PILLAR 3)
// ===================================================================================
//  DEFINITION : a derived class reuses / extends / specialises a base class. "IS-A".
//
//  5 TYPES
//    1. Single        A <- B
//    2. Multilevel    A <- B <- C
//    3. Hierarchical  A <- B, A <- C            (one base, many derived)
//    4. Multiple      A, B <- C                 (many bases, one derived)
//    5. Hybrid        combination -> can create the DIAMOND problem
//
//  ACCESS TABLE : what a member becomes inside the derived class
//  +----------------+-----------------+--------------------+------------------+
//  | base member    | public inherit  | protected inherit  | private inherit  |
//  +----------------+-----------------+--------------------+------------------+
//  | public         | public          | protected          | private          |
//  | protected      | protected       | protected          | private          |
//  | private        | NOT accessible  | NOT accessible     | NOT accessible   |
//  +----------------+-----------------+--------------------+------------------+
//  Default mode : `class` -> private, `struct` -> public.
//
//  MEANING OF THE MODES
//    public    inheritance = "IS-A"                     (substitutable, LSP applies)
//    private   inheritance = "IS-IMPLEMENTED-IN-TERMS-OF" (composition alternative;
//                            prefer real composition unless you must override a virtual
//                            or need the empty-base optimisation)
//    protected inheritance = same as private but derived-of-derived also sees it. Rare.
//
//  ORDER OF EXECUTION
//    construction : Base -> members (declaration order) -> Derived body
//    destruction  : exactly the reverse
//
//  NAME HIDING : a derived member with the same NAME hides ALL base overloads of that
//  name (not just the matching one). Fix with `using Base::name;`.
//
//  WHAT IS NOT INHERITED : constructors (until `using Base::Base;`), destructor,
//  copy/move assignment operator, friends, private members (present but inaccessible).
// -----------------------------------------------------------------------------------
namespace part05 {

// ---------- 1. single + 2. multilevel ----------
class Device {
protected:
    string serial;

public:
    explicit Device(string serial) : serial(std::move(serial)) {
        cout << "  Device ctor   " << this->serial << "\n";
    }
    virtual ~Device() { cout << "  Device dtor   " << serial << "\n"; }

    virtual void powerOn() const { cout << "  Device " << serial << " powering on\n"; }
};

class SmartDevice : public Device {                          // single
protected:
    string firmware;

public:
    SmartDevice(string serial, string firmware)
        : Device(std::move(serial)), firmware(std::move(firmware)) {
        cout << "  SmartDevice ctor\n";
    }
    ~SmartDevice() override { cout << "  SmartDevice dtor\n"; }

    void powerOn() const override {
        Device::powerOn();                                   // reuse base behaviour
        cout << "  ...booting firmware " << firmware << "\n";
    }
};

class SmartWatch : public SmartDevice {                       // multilevel
public:
    SmartWatch(string serial) : SmartDevice(std::move(serial), "wearOS-4") {
        cout << "  SmartWatch ctor\n";
    }
    ~SmartWatch() override { cout << "  SmartWatch dtor\n"; }

    void powerOn() const override {
        SmartDevice::powerOn();
        cout << "  ...showing watch face\n";
    }
};

// ---------- 3. hierarchical ----------
class Shape {
public:
    virtual ~Shape() = default;
    virtual double area() const = 0;
    virtual string name() const = 0;
};

class Rectangle : public Shape {
protected:
    double width, height;

public:
    Rectangle(double width, double height) : width(width), height(height) {}
    double area() const override { return width * height; }
    string name() const override { return "Rectangle"; }
};

class Circle : public Shape {
private:
    double radius;

public:
    explicit Circle(double radius) : radius(radius) {}
    double area() const override { return 3.14159265 * radius * radius; }
    string name() const override { return "Circle"; }
};

// ---------- 4. multiple inheritance + ambiguity resolution ----------
class Swimmer {
public:
    virtual ~Swimmer() = default;
    void move() const { cout << "  swimming\n"; }            // same name in both bases
};

class Flyer {
public:
    virtual ~Flyer() = default;
    void move() const { cout << "  flying\n"; }
};

class Duck : public Swimmer, public Flyer {
public:
    void move() const {                                      // must disambiguate
        Swimmer::move();
        Flyer::move();
    }
};

// ---------- 5. hybrid -> DIAMOND PROBLEM ----------
//  Person <- Student, Person <- Teacher, (Student, Teacher) <- TeachingAssistant
//  WITHOUT `virtual` : TeachingAssistant contains TWO Person sub-objects
//                      -> ambiguous `ta.name`, duplicated state, Person ctor runs twice.
//  WITH `virtual public Person` : exactly ONE shared Person sub-object.
//  RULE : with a virtual base, the MOST DERIVED class initializes the virtual base;
//         the intermediate classes' calls to Person(...) are ignored.
class Person {
public:
    string name;

    explicit Person(string name = "anonymous") : name(std::move(name)) {
        cout << "  Person ctor (" << this->name << ") - runs ONCE with virtual base\n";
    }
    virtual ~Person() = default;

    virtual void introduce() const { cout << "  I am " << name << "\n"; }
};

class Student : virtual public Person {
public:
    explicit Student(string name) : Person(std::move(name)) {}
    void study() const { cout << "  " << name << " is studying\n"; }
};

class Teacher : virtual public Person {
public:
    explicit Teacher(string name) : Person(std::move(name)) {}
    void teach() const { cout << "  " << name << " is teaching\n"; }
};

class TeachingAssistant : public Student, public Teacher {
public:
    explicit TeachingAssistant(string name)
        : Person(name), Student(name), Teacher(name) {}      // most-derived inits vbase

    void introduce() const override {
        cout << "  I am TA " << name << " (single Person sub-object)\n";
    }
};

// ---------- private inheritance : "implemented in terms of" ----------
class Timer {
protected:
    virtual void onTick() { cout << "  tick\n"; }

public:
    virtual ~Timer() = default;
    void run(int ticks) { for (int i = 0; i < ticks; ++i) onTick(); }
};

class Poller : private Timer {                                // NOT a Timer to the outside
public:
    void poll(int times) { run(times); }                      // reuse implementation

private:
    void onTick() override { cout << "  polling api...\n"; }
};

// ---------- name hiding + using-declaration ----------
class BasePrinter {
public:
    void print(int value)    const { cout << "  base int   " << value << "\n"; }
    void print(double value) const { cout << "  base double " << value << "\n"; }
};

class DerivedPrinter : public BasePrinter {
public:
    using BasePrinter::print;                                 // un-hide base overloads
    void print(const string& value) const { cout << "  derived string " << value << "\n"; }
};

// ---------- inheriting constructors ----------
class Money {
protected:
    long paise;

public:
    explicit Money(long paise) : paise(paise) {}
    long value() const { return paise; }
};

class Rupees : public Money {
public:
    using Money::Money;                                       // inherit base constructors
};

void demo() {
    banner("PART 05 : INHERITANCE");

    cout << "-- multilevel : ctor top-down, dtor bottom-up --\n";
    {
        SmartWatch watch("SW-001");
        watch.powerOn();
    }

    cout << "-- hierarchical --\n";
    vector<unique_ptr<Shape>> shapes;
    shapes.push_back(make_unique<Rectangle>(3, 4));
    shapes.push_back(make_unique<Circle>(2));
    for (const auto& shape : shapes)
        cout << "  " << shape->name() << " area=" << shape->area() << "\n";

    cout << "-- multiple inheritance ambiguity --\n";
    Duck duck;
    duck.move();
    // duck.Swimmer::move();  // fully-qualified call is also allowed

    cout << "-- diamond solved by virtual inheritance --\n";
    TeachingAssistant ta("Meera");
    ta.introduce();
    ta.study();
    ta.teach();

    cout << "-- private inheritance --\n";
    Poller poller;
    poller.poll(2);
    // Timer* t = &poller;   // COMPILE ERROR : conversion is private

    cout << "-- name hiding --\n";
    DerivedPrinter printer;
    printer.print(42);
    printer.print("hello");

    cout << "-- inherited constructors --\n";
    Rupees r(50000);
    cout << "  rupees paise=" << r.value() << "\n";
}

} // namespace part05

// ===================================================================================
// PART 06 : POLYMORPHISM  (PILLAR 4)
// ===================================================================================
//  "Many forms" - one interface, many implementations.
//
//  A. COMPILE-TIME (static / early binding)
//       - function overloading      (same name, different parameter list)
//       - operator overloading
//       - templates / CRTP
//     Resolved by the compiler. Zero runtime cost. No virtual needed.
//     NOTE: return type alone CANNOT overload. const-ness of a member fn CAN.
//
//  B. RUN-TIME (dynamic / late binding)
//       - virtual functions through a base pointer/reference
//     Resolved via the vtable at runtime.
//
//  HOW VTABLES WORK
//    - Any class with >=1 virtual function gets a static, per-CLASS table of function
//      pointers (the vtable). Each OBJECT gets a hidden pointer to it (the vptr), which
//      is why sizeof grows by one pointer.
//    - The vptr is set by each constructor as it runs, base-first. That is why calling a
//      virtual function inside a base constructor dispatches to the BASE version.
//    - `p->f()` becomes roughly: (*(p->vptr[index_of_f]))(p).
//
//  KEY RULES / PITFALLS
//    1. VIRTUAL DESTRUCTOR : mandatory if you ever `delete basePtr` where the object is
//       actually a derived type. Otherwise the derived dtor never runs -> resource leak
//       and undefined behaviour.
//    2. OBJECT SLICING : assigning/copying a Derived into a Base *by value* keeps only
//       the base part and the vptr becomes Base's. Always use pointers/references.
//    3. Constructors are never virtual (no object/vptr yet). Use a virtual clone() for
//       "virtual construction" (Prototype pattern).
//    4. Static, friend, and non-member functions cannot be virtual.
//    5. DEFAULT ARGUMENTS OF VIRTUAL FUNCTIONS ARE STATICALLY BOUND -> the base's default
//       is used even when the derived override runs. Never redeclare them.
//    6. `override` = compiler-checked; `final` = cannot be overridden further.
//    7. Covariant return types : an override may return Derived* where the base returns
//       Base* (pointer/reference only).
//    8. Overriding requires the SAME signature (incl. const). Different signature =
//       hiding, not overriding.
//    9. You cannot override a non-virtual function polymorphically - you only hide it.
//   10. Virtual calls inside a destructor also dispatch to the current (base) level.
// -----------------------------------------------------------------------------------
namespace part06 {

// ---------- A. compile-time : overloading ----------
class Calculator {
public:
    int    add(int a, int b)                 { return a + b; }
    double add(double a, double b)           { return a + b; }
    int    add(int a, int b, int c)          { return a + b + c; }
    string add(const string& a, const string& b) { return a + b; }

    void describe()       { cout << "  non-const describe\n"; }
    void describe() const { cout << "  const describe\n"; }      // const-overload is legal
};

// ---------- B. run-time : virtual dispatch, override, final, covariant return ----------
class Notification {
public:
    virtual ~Notification() = default;                            // rule 1

    virtual void send(const string& to) const = 0;
    virtual Notification* clone() const = 0;                      // rule 3 : virtual ctor idiom
    virtual string channel() const { return "generic"; }
};

class EmailNotification : public Notification {
private:
    string subject;

public:
    explicit EmailNotification(string subject) : subject(std::move(subject)) {}

    void send(const string& to) const override {
        cout << "  EMAIL to " << to << " : " << subject << "\n";
    }
    EmailNotification* clone() const override {                   // rule 7 : covariant
        return new EmailNotification(*this);
    }
    string channel() const final { return "email"; }              // rule 6 : no further override
};

class SmsNotification : public Notification {
public:
    void send(const string& to) const override { cout << "  SMS to " << to << "\n"; }
    SmsNotification* clone() const override { return new SmsNotification(*this); }
    string channel() const override { return "sms"; }
};

// ---------- pitfall : object slicing ----------
class BaseValue {
public:
    virtual ~BaseValue() = default;
    virtual string who() const { return "BaseValue"; }
};

class DerivedValue : public BaseValue {
public:
    string who() const override { return "DerivedValue"; }
};

// ---------- pitfall : virtual call inside a constructor ----------
class Widget {
public:
    Widget() { setup(); }                                          // calls Widget::setup !
    virtual ~Widget() = default;
    virtual void setup() { cout << "  Widget::setup (base version runs)\n"; }
};

class Button : public Widget {
private:
    int label = 42;

public:
    void setup() override { cout << "  Button::setup label=" << label << "\n"; }
};

// ---------- pitfall : default argument of a virtual function ----------
class DefaultArgBase {
public:
    virtual ~DefaultArgBase() = default;
    virtual void show(int value = 10) { cout << "  Base::show " << value << "\n"; }
};

class DefaultArgDerived : public DefaultArgBase {
public:
    void show(int value = 20) override { cout << "  Derived::show " << value << "\n"; }
};

// ---------- RTTI : dynamic_cast / typeid ----------
class Animal {
public:
    virtual ~Animal() = default;
    virtual void speak() const = 0;
};

class Dog : public Animal {
public:
    void speak() const override { cout << "  Woof\n"; }
    void fetch() const          { cout << "  fetching the ball\n"; }   // Dog-only API
};

class Cat : public Animal {
public:
    void speak() const override { cout << "  Meow\n"; }
};

void handle(Animal& animal) {
    animal.speak();                                            // polymorphic
    if (Dog* dog = dynamic_cast<Dog*>(&animal)) dog->fetch();  // safe downcast
    cout << "  typeid = " << typeid(animal).name() << "\n";
}

void demo() {
    banner("PART 06 : POLYMORPHISM");

    cout << "-- compile-time overloading --\n";
    Calculator calculator;
    cout << "  " << calculator.add(2, 3) << " " << calculator.add(2.5, 3.5)
         << " " << calculator.add(1, 2, 3) << " " << calculator.add("a", "b") << "\n";
    const Calculator& constCalculator = calculator;
    calculator.describe();
    constCalculator.describe();

    cout << "-- run-time dispatch through base pointer --\n";
    vector<unique_ptr<Notification>> notifications;
    notifications.push_back(make_unique<EmailNotification>("Your order shipped"));
    notifications.push_back(make_unique<SmsNotification>());
    for (const auto& notification : notifications) {
        cout << "  channel=" << notification->channel() << "\n";
        notification->send("ravindra@example.com");
    }

    cout << "-- virtual clone (virtual constructor idiom) --\n";
    unique_ptr<Notification> copy(notifications[0]->clone());
    copy->send("copy@example.com");

    cout << "-- object slicing --\n";
    DerivedValue derived;
    BaseValue    sliced = derived;                             // COPY -> derived part lost
    BaseValue&   reference = derived;                          // reference -> intact
    cout << "  by value     : " << sliced.who() << "   <-- sliced!\n";
    cout << "  by reference : " << reference.who() << "\n";

    cout << "-- virtual call inside constructor --\n";
    Button button;                                             // prints Widget::setup
    button.setup();                                            // now prints Button::setup

    cout << "-- default args are statically bound --\n";
    DefaultArgDerived defaultArgDerived;
    DefaultArgBase*   basePointer = &defaultArgDerived;
    basePointer->show();                                       // Derived::show but value=10

    cout << "-- RTTI --\n";
    Dog dog; Cat cat;
    handle(dog);
    handle(cat);

    cout << "  sizeof(Cat)=" << sizeof(Cat) << " (includes the hidden vptr)\n";
}

} // namespace part06

// ===================================================================================
// PART 07 : OPERATOR OVERLOADING
// ===================================================================================
//  Give your type natural syntax. Overloadable: arithmetic, comparison, [], (), ->, *,
//  ++/--, <<, >>, =, +=, new/delete, conversion operators, and more.
//  CANNOT overload : ::  .  .*  ?:  sizeof  typeid  and the alignof family.
//
//  RULES OF THUMB
//    - Symmetric binary operators (+ - * == <) : prefer NON-MEMBER (often friend) so that
//      implicit conversion works on BOTH sides (2 * money as well as money * 2).
//    - Compound assignment (+= -=) : MEMBER, returns *this by reference. Implement `+`
//      in terms of `+=`.
//    - << and >> : must be non-member (left operand is the stream), usually friend.
//    - Pre-increment returns Type& ; post-increment takes a dummy int and returns a copy.
//    - [] usually needs a const and a non-const overload.
//    - () makes a functor (used by STL algorithms, comparators, callbacks).
//    - -> must return a pointer (or something with its own ->). Used by smart pointers.
//    - Do not overload operators with surprising meaning. Keep semantics intuitive.
// -----------------------------------------------------------------------------------
namespace part07 {

class Money {
private:
    long paise;

public:
    Money(long rupees = 0, int paise = 0) : paise(rupees * 100 + paise) {}

    Money& operator+=(const Money& other) { paise += other.paise; return *this; }
    Money& operator-=(const Money& other) { paise -= other.paise; return *this; }

    Money& operator++()    { paise += 100; return *this; }                    // pre
    Money  operator++(int) { Money before = *this; paise += 100; return before; } // post

    Money operator-() const { Money negated; negated.paise = -paise; return negated; }

    explicit operator bool() const { return paise != 0; }                     // conversion
    explicit operator double() const { return paise / 100.0; }

    long raw() const { return paise; }

    friend Money operator+(Money left, const Money& right) { return left += right; }
    friend Money operator-(Money left, const Money& right) { return left -= right; }
    friend Money operator*(const Money& money, int times) { return Money(0, money.paise * times); }
    friend Money operator*(int times, const Money& money) { return money * times; }

    friend bool operator==(const Money& a, const Money& b) { return a.paise == b.paise; }
    friend bool operator!=(const Money& a, const Money& b) { return !(a == b); }
    friend bool operator< (const Money& a, const Money& b) { return a.paise <  b.paise; }
    friend bool operator> (const Money& a, const Money& b) { return b <  a; }
    friend bool operator<=(const Money& a, const Money& b) { return !(b < a); }
    friend bool operator>=(const Money& a, const Money& b) { return !(a < b); }

    friend ostream& operator<<(ostream& out, const Money& money) {
        return out << "Rs." << money.paise / 100 << "." << (money.paise % 100 < 10 ? "0" : "")
                   << money.paise % 100;
    }

    friend istream& operator>>(istream& in, Money& money) {
        long rupees = 0;
        in >> rupees;
        money = Money(rupees);
        return in;
    }
};

class Cart {
private:
    vector<Money> items;

public:
    void add(const Money& item) { items.push_back(item); }

    Money&       operator[](size_t index)       { return items[index]; }
    const Money& operator[](size_t index) const { return items.at(index); }

    size_t size() const { return items.size(); }
};

// functor : operator()
class DiscountPolicy {
private:
    int percentOff;

public:
    explicit DiscountPolicy(int percentOff) : percentOff(percentOff) {}
    Money operator()(const Money& price) const {
        return Money(0, price.raw() * (100 - percentOff) / 100);
    }
};

// operator-> and operator* : the smart-pointer shape
template <typename T>
class ScopedPointer {
private:
    T* pointee;

public:
    explicit ScopedPointer(T* pointee) : pointee(pointee) {}
    ~ScopedPointer() { delete pointee; }

    ScopedPointer(const ScopedPointer&)            = delete;
    ScopedPointer& operator=(const ScopedPointer&) = delete;

    T* operator->() const { return pointee; }
    T& operator*()  const { return *pointee; }
};

void demo() {
    banner("PART 07 : OPERATOR OVERLOADING");

    Money price(250, 50);
    Money shipping(40);

    cout << "  price=" << price << " shipping=" << shipping << "\n";
    cout << "  total=" << (price + shipping) << "\n";
    cout << "  3 * price=" << 3 * price << "  price * 3=" << price * 3 << "\n";
    cout << "  negate=" << -price << "\n";
    cout << "  price > shipping ? " << (price > shipping) << "\n";

    Money counter(10);
    cout << "  post++ returns " << counter++ << " then value is " << counter << "\n";
    cout << "  pre++  returns " << ++counter << "\n";

    cout << "  as double = " << static_cast<double>(price) << "\n";
    if (price) cout << "  non-zero money is truthy\n";

    Money parsed;
    istringstream input("999");
    input >> parsed;
    cout << "  parsed from stream = " << parsed << "\n";

    Cart cart;
    cart.add(price);
    cart.add(shipping);
    cart[0] += Money(10);
    cout << "  cart[0]=" << cart[0] << " size=" << cart.size() << "\n";

    DiscountPolicy tenPercentOff(10);
    cout << "  functor applied : " << tenPercentOff(price) << "\n";

    ScopedPointer<Cart> owned(new Cart());
    owned->add(Money(5));
    cout << "  via operator-> size=" << owned->size()
         << "  via operator* size=" << (*owned).size() << "\n";
}

} // namespace part07

// ===================================================================================
// PART 08 : FRIEND FUNCTIONS AND FRIEND CLASSES
// ===================================================================================
//  `friend` grants access to private/protected members of the granting class.
//  PROPERTIES
//    - Friendship is GRANTED, not taken (only the class can declare its friends).
//    - NOT symmetric   : A friend of B does not make B a friend of A.
//    - NOT transitive  : a friend of a friend is not a friend.
//    - NOT inherited   : a derived class does not inherit its base's friends.
//    - A friend function is NOT a member : it has no `this`, is not called with `.`,
//      and access specifiers do not apply to where you declare it.
//  WHEN IT IS THE RIGHT TOOL
//    - operator<< / operator>> (left operand must be the stream)
//    - symmetric binary operators needing conversions on both sides
//    - a tightly coupled helper/builder/iterator class
//    - unit-test access (better: design for testability instead)
//  Friendship is the tightest coupling in C++ - use it sparingly and deliberately.
// -----------------------------------------------------------------------------------
namespace part08 {

class Matrix;                                                  // forward declaration

class MatrixPrinter {                                          // friend CLASS
public:
    void dump(const Matrix& matrix) const;
};

class Matrix {
private:
    int rows, cols;
    vector<int> cells;

public:
    Matrix(int rows, int cols) : rows(rows), cols(cols), cells(rows * cols, 0) {}

    int& at(int row, int col) { return cells[row * cols + col]; }

    friend class MatrixPrinter;                                // whole class is a friend
    friend Matrix transpose(const Matrix& matrix);             // free function is a friend
    friend ostream& operator<<(ostream& out, const Matrix& matrix);
};

void MatrixPrinter::dump(const Matrix& matrix) const {
    cout << "  dump " << matrix.rows << "x" << matrix.cols << " -> ";
    for (int cell : matrix.cells) cout << cell << " ";          // touching private members
    cout << "\n";
}

Matrix transpose(const Matrix& matrix) {
    Matrix result(matrix.cols, matrix.rows);
    for (int row = 0; row < matrix.rows; ++row)
        for (int col = 0; col < matrix.cols; ++col)
            result.cells[col * matrix.rows + row] = matrix.cells[row * matrix.cols + col];
    return result;
}

ostream& operator<<(ostream& out, const Matrix& matrix) {
    for (int row = 0; row < matrix.rows; ++row) {
        out << "  ";
        for (int col = 0; col < matrix.cols; ++col) out << matrix.cells[row * matrix.cols + col] << " ";
        out << "\n";
    }
    return out;
}

void demo() {
    banner("PART 08 : FRIEND");

    Matrix matrix(2, 3);
    matrix.at(0, 0) = 1; matrix.at(0, 1) = 2; matrix.at(0, 2) = 3;
    matrix.at(1, 0) = 4; matrix.at(1, 1) = 5; matrix.at(1, 2) = 6;

    cout << matrix;
    MatrixPrinter().dump(matrix);
    cout << "  transposed:\n" << transpose(matrix);
}

} // namespace part08

// ===================================================================================
// PART 09 : TEMPLATES, GENERICS, SPECIALISATION, CRTP (STATIC POLYMORPHISM)
// ===================================================================================
//  Templates = compile-time genericity. The compiler generates a separate concrete
//  class/function per instantiation. Zero runtime dispatch cost.
//
//  virtual (dynamic polymorphism)      vs    template/CRTP (static polymorphism)
//    decided at runtime                      decided at compile time
//    vtable indirection, no inlining         fully inlinable
//    heterogeneous containers possible       one concrete type per instantiation
//    smaller binary                          code bloat
//    can cross a DLL/ABI boundary            header-only, recompiles on change
//  LLD interviews almost always want the virtual version (open/closed, plugin-able).
//  CRTP shows up for mixins, counters, and performance-critical hierarchies.
// -----------------------------------------------------------------------------------
namespace part09 {

// ---------- function template ----------
template <typename T>
T maxOf(const T& a, const T& b) { return (a < b) ? b : a; }

// ---------- class template ----------
template <typename T>
class Stack {
private:
    vector<T> items;

public:
    void push(const T& item) { items.push_back(item); }

    T pop() {
        if (items.empty()) throw runtime_error("stack underflow");
        T top = items.back();
        items.pop_back();
        return top;
    }

    bool   empty() const { return items.empty(); }
    size_t size()  const { return items.size(); }
};

// ---------- full specialisation ----------
template <>
class Stack<bool> {
private:
    unsigned long long bits = 0;
    int count = 0;

public:
    void push(bool bit) { if (bit) bits |= (1ULL << count); ++count; }

    bool pop() {
        --count;
        return (bits >> count) & 1ULL;
    }

    bool   empty() const { return count == 0; }
    size_t size()  const { return static_cast<size_t>(count); }
};

// ---------- template with a non-type parameter + default argument ----------
template <typename T, size_t Capacity = 4>
class RingBuffer {
private:
    T      slots[Capacity];
    size_t head = 0;
    size_t used = 0;

public:
    void push(const T& item) {
        slots[(head + used) % Capacity] = item;
        if (used == Capacity) head = (head + 1) % Capacity;
        else ++used;
    }

    void print() const {
        cout << "  ring: ";
        for (size_t i = 0; i < used; ++i) cout << slots[(head + i) % Capacity] << " ";
        cout << "\n";
    }
};

// ---------- variadic template ----------
template <typename... Args>
void logLine(const Args&... args) {
    cout << "  log:";
    ((cout << " " << args), ...);                              // C++17 fold expression
    cout << "\n";
}

// ---------- CRTP : static polymorphism, no vtable ----------
template <typename Derived>
class Serializable {
public:
    string serialize() const {
        return "{" + static_cast<const Derived*>(this)->fields() + "}";
    }
};

class User : public Serializable<User> {
private:
    string name;
    int    age;

public:
    User(string name, int age) : name(std::move(name)), age(age) {}
    string fields() const { return "\"name\":\"" + name + "\",\"age\":" + to_string(age); }
};

// ---------- CRTP mixin : per-type instance counter ----------
template <typename Derived>
class InstanceCounter {
private:
    static int liveCount;

public:
    InstanceCounter()  { ++liveCount; }
    ~InstanceCounter() { --liveCount; }
    static int live() { return liveCount; }
};

template <typename Derived>
int InstanceCounter<Derived>::liveCount = 0;

class Session : public InstanceCounter<Session> {};
class Request : public InstanceCounter<Request> {};

void demo() {
    banner("PART 09 : TEMPLATES + CRTP");

    cout << "  maxOf(3,7)=" << maxOf(3, 7)
         << "  maxOf(\"abc\",\"abd\")=" << maxOf(string("abc"), string("abd")) << "\n";

    Stack<string> names;
    names.push("a"); names.push("b");
    cout << "  Stack<string> pop=" << names.pop() << " size=" << names.size() << "\n";

    Stack<bool> flags;                                          // uses the specialisation
    flags.push(true); flags.push(false); flags.push(true);
    cout << "  Stack<bool> (bit-packed) pop=" << flags.pop() << " size=" << flags.size() << "\n";

    RingBuffer<int, 3> ring;
    for (int i = 1; i <= 5; ++i) ring.push(i);
    ring.print();

    logLine("order", 42, "created", 3.5);

    cout << "  CRTP serialize : " << User("Ravindra", 27).serialize() << "\n";

    {
        Session s1, s2;
        Request r1;
        cout << "  live sessions=" << Session::live() << " live requests=" << Request::live() << "\n";
    }
    cout << "  after scope : sessions=" << Session::live() << "\n";
}

} // namespace part09

// ===================================================================================
// PART 10 : RAII, SMART POINTERS, pImpl
// ===================================================================================
//  RAII (Resource Acquisition Is Initialization) : tie a resource's lifetime to an
//  object's lifetime. Acquire in the constructor, release in the destructor. The
//  destructor runs on every exit path - normal return, early return, or exception -
//  so there is no leak and no manual cleanup code.
//
//  SMART POINTERS
//    unique_ptr<T>  : sole ownership, move-only, zero overhead. THE DEFAULT.
//                     make_unique<T>(args...)
//    shared_ptr<T>  : shared ownership via an atomic reference count. Heavier.
//                     make_shared<T>(args...) (one allocation, exception-safe)
//    weak_ptr<T>    : non-owning observer of a shared_ptr. .lock() to use it.
//                     BREAKS REFERENCE CYCLES (parent<->child, observer lists).
//  Rules: never `delete` manually; never build two smart pointers from one raw pointer;
//  pass unique_ptr by value to transfer ownership, or pass T&/T* for plain use.
//
//  pImpl (pointer to implementation / opaque pointer) : keep the private members in a
//  forward-declared struct in the .cpp. The header stops leaking internals, callers stop
//  recompiling when internals change, and the ABI stays stable.
// -----------------------------------------------------------------------------------
namespace part10 {

class FileHandle {                                              // RAII wrapper
private:
    string name;

public:
    explicit FileHandle(string name) : name(std::move(name)) {
        cout << "  OPEN  " << this->name << "\n";
    }
    ~FileHandle() { cout << "  CLOSE " << name << "\n"; }

    FileHandle(const FileHandle&)            = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    void write(const string& line) { cout << "  WRITE " << name << " <- " << line << "\n"; }
};

void raiiSurvivesExceptions() {
    FileHandle file("audit.log");
    file.write("starting risky work");
    try {
        throw runtime_error("boom");
    } catch (const exception& e) {
        cout << "  caught: " << e.what() << " (file still closes below)\n";
    }
}

// ---------- shared_ptr cycle and the weak_ptr fix ----------
class Child;

class Parent {
public:
    string name;
    vector<shared_ptr<Child>> children;

    explicit Parent(string name) : name(std::move(name)) {}
    ~Parent() { cout << "  ~Parent " << name << "\n"; }
};

class Child {
public:
    string name;
    weak_ptr<Parent> parent;                                    // weak breaks the cycle

    explicit Child(string name) : name(std::move(name)) {}
    ~Child() { cout << "  ~Child " << name << "\n"; }

    void greetParent() const {
        if (auto locked = parent.lock()) cout << "  my parent is " << locked->name << "\n";
        else                             cout << "  parent already gone\n";
    }
};

// ---------- pImpl ----------
class SearchIndex {                                             // header-facing type
private:
    struct Impl;                                                // opaque
    unique_ptr<Impl> impl;

public:
    SearchIndex();
    ~SearchIndex();                                             // must be out of line

    void add(const string& document);
    int  count() const;
};

struct SearchIndex::Impl {
    vector<string> documents;
    unordered_map<string, int> termFrequency;
};

SearchIndex::SearchIndex() : impl(make_unique<Impl>()) {}
SearchIndex::~SearchIndex() = default;

void SearchIndex::add(const string& document) {
    impl->documents.push_back(document);
    impl->termFrequency[document] += 1;
}

int SearchIndex::count() const { return static_cast<int>(impl->documents.size()); }

void demo() {
    banner("PART 10 : RAII + SMART POINTERS + pImpl");

    raiiSurvivesExceptions();

    cout << "-- unique_ptr : move-only ownership --\n";
    unique_ptr<FileHandle> owner = make_unique<FileHandle>("unique.log");
    unique_ptr<FileHandle> newOwner = std::move(owner);
    cout << "  old owner is " << (owner ? "still set" : "null after move") << "\n";
    newOwner->write("owned by newOwner");

    cout << "-- shared_ptr reference counting --\n";
    shared_ptr<Parent> parent = make_shared<Parent>("Anil");
    cout << "  use_count=" << parent.use_count() << "\n";
    {
        shared_ptr<Parent> second = parent;
        cout << "  inside scope use_count=" << parent.use_count() << "\n";
    }
    cout << "  after scope use_count=" << parent.use_count() << "\n";

    cout << "-- weak_ptr breaks the parent/child cycle --\n";
    {
        auto child = make_shared<Child>("Riya");
        parent->children.push_back(child);
        child->parent = parent;                                 // weak, so no leak
        child->greetParent();
    }
    parent.reset();

    cout << "-- pImpl --\n";
    SearchIndex index;
    index.add("lld"); index.add("oops");
    cout << "  documents=" << index.count() << "\n";
}

} // namespace part10

// ===================================================================================
// PART 11 : OBJECT RELATIONSHIPS  (the vocabulary LLD interviews grade you on)
// ===================================================================================
//  INHERITANCE  "IS-A"        Dog is an Animal.
//  COMPOSITION  "PART-OF"     strong ownership, same lifetime. Car owns its Engine; kill
//                             the Car and the Engine dies. Modelled as a by-value member
//                             or a unique_ptr member.
//  AGGREGATION  "HAS-A"       weak ownership, independent lifetime. A Team has Players,
//                             but the Players exist without the Team. Modelled as
//                             shared_ptr / raw pointer / reference to something external.
//  ASSOCIATION  "USES/KNOWS"  two objects reference each other, no ownership at all.
//                             Doctor <-> Patient. Can be 1-1, 1-N, N-N, uni/bi-directional.
//  DEPENDENCY   "DEPENDS-ON"  the weakest: appears only as a parameter, local variable, or
//                             return type. OrderService depends on Logger passed per call.
//
//  UML strength order :  dependency < association < aggregation < composition < inheritance
//
//  FAVOUR COMPOSITION OVER INHERITANCE
//    Inheritance is compile-time, single-shot, exposes the base's protected surface, and
//    breaks encapsulation (the fragile base class problem). Composition is runtime
//    swappable, testable, and combines freely -> Strategy, Decorator, Bridge all exist to
//    replace an inheritance explosion. Use inheritance ONLY for a true IS-A that satisfies
//    the Liskov Substitution Principle.
// -----------------------------------------------------------------------------------
namespace part11 {

class Engine {                                                  // COMPOSITION part
private:
    int horsepower;

public:
    explicit Engine(int horsepower) : horsepower(horsepower) {
        cout << "    Engine built " << horsepower << "hp\n";
    }
    ~Engine() { cout << "    Engine destroyed\n"; }

    void start() const { cout << "    engine running at " << horsepower << "hp\n"; }
};

class Car {
private:
    string registration;
    Engine engine;                                              // COMPOSITION : same lifetime

public:
    Car(string registration, int horsepower)
        : registration(std::move(registration)), engine(horsepower) {}

    void drive() const {
        cout << "  " << registration << " driving\n";
        engine.start();
    }
};

class Player {                                                  // AGGREGATION part
private:
    string name;

public:
    explicit Player(string name) : name(std::move(name)) {}
    const string& getName() const { return name; }
};

class Team {
private:
    string name;
    vector<shared_ptr<Player>> players;                         // AGGREGATION : outlives team

public:
    explicit Team(string name) : name(std::move(name)) {}

    void sign(const shared_ptr<Player>& player) { players.push_back(player); }

    void roster() const {
        cout << "  " << name << ":";
        for (const auto& player : players) cout << " " << player->getName();
        cout << "\n";
    }
};

class AuditTrail {                                              // DEPENDENCY
public:
    void write(const string& line) const { cout << "  audit: " << line << "\n"; }
};

class OrderService {
public:
    void placeOrder(const string& item, const AuditTrail& audit) const {  // param only
        audit.write("order placed for " + item);
    }
};

void demo() {
    banner("PART 11 : COMPOSITION / AGGREGATION / ASSOCIATION / DEPENDENCY");

    cout << "-- composition : engine dies with the car --\n";
    {
        Car car("MH12-CD-4321", 150);
        car.drive();
    }

    cout << "-- aggregation : players survive the team --\n";
    auto virat = make_shared<Player>("Virat");
    auto rohit = make_shared<Player>("Rohit");
    {
        Team team("Mumbai");
        team.sign(virat);
        team.sign(rohit);
        team.roster();
    }
    cout << "  after team destroyed, player still alive: " << virat->getName() << "\n";

    cout << "-- dependency --\n";
    OrderService().placeOrder("iPhone", AuditTrail());
}

} // namespace part11

// ===================================================================================
// PART 12 : SOLID PRINCIPLES
// ===================================================================================
//  S - Single Responsibility : a class should have ONE reason to change / one actor it
//      answers to. Symptom of violation: "and" in the class description, a class that
//      does business logic + persistence + notification + formatting.
//
//  O - Open/Closed : open for extension, closed for modification. Adding a new variant
//      should mean adding a class, not editing an existing switch/if-else chain.
//      Enabled by polymorphism (Strategy, Factory, Visitor).
//
//  L - Liskov Substitution : a subtype must be usable anywhere the base is expected
//      WITHOUT the caller knowing. Violations: strengthening preconditions, weakening
//      postconditions, throwing "not supported", breaking a base-class invariant.
//      Classic examples: Square deriving from Rectangle (setWidth breaks the invariant),
//      Penguin deriving from a Bird that can fly, ReadOnlyList deriving from List.
//
//  I - Interface Segregation : many small role-specific interfaces beat one fat one.
//      No client should be forced to depend on methods it does not use (and no class
//      should have to stub out methods it cannot support - which is also an LSP fix).
//
//  D - Dependency Inversion : high-level policy must not depend on low-level detail;
//      both depend on an abstraction. Concretely: depend on interfaces, and inject the
//      implementation from outside (constructor injection) instead of `new`-ing it inside.
//      DIP = the principle. Dependency Injection = the technique. IoC container = a tool.
// -----------------------------------------------------------------------------------
namespace part12 {

// ---------------- S : Single Responsibility ----------------
//  BAD : one class formats, saves, emails, and validates -> 4 reasons to change.
namespace srp_bad {
class Invoice {
public:
    double amount = 0;
    void calculateTotal() {}
    void saveToDatabase() {}                                    // persistence concern
    string renderAsPdf() { return ""; }                         // presentation concern
    void emailToCustomer() {}                                   // notification concern
};
}

namespace srp_good {
class Invoice {                                                 // business data + rules only
private:
    double amount;
    double taxRate;

public:
    Invoice(double amount, double taxRate) : amount(amount), taxRate(taxRate) {}
    double total() const { return amount * (1 + taxRate); }
};

class InvoiceRepository {
public:
    void save(const Invoice& invoice) { cout << "  saved invoice total=" << invoice.total() << "\n"; }
};

class InvoicePdfRenderer {
public:
    string render(const Invoice& invoice) const {
        return "PDF[total=" + to_string(invoice.total()) + "]";
    }
};

class InvoiceMailer {
public:
    void send(const string& document, const string& to) const {
        cout << "  mailed " << document << " to " << to << "\n";
    }
};
}

// ---------------- O : Open/Closed ----------------
//  BAD : every new shipping method edits this function.
namespace ocp_bad {
enum class Courier { Bluedart, Delhivery };

double shippingCost(Courier courier, double weight) {
    if (courier == Courier::Bluedart)  return 50 + 10 * weight;
    if (courier == Courier::Delhivery) return 40 + 12 * weight;
    return 0;                                                   // add DTDC -> edit here
}
}

namespace ocp_good {
class ShippingStrategy {
public:
    virtual ~ShippingStrategy() = default;
    virtual double cost(double weight) const = 0;
    virtual string name() const = 0;
};

class Bluedart : public ShippingStrategy {
public:
    double cost(double weight) const override { return 50 + 10 * weight; }
    string name() const override { return "Bluedart"; }
};

class Delhivery : public ShippingStrategy {
public:
    double cost(double weight) const override { return 40 + 12 * weight; }
    string name() const override { return "Delhivery"; }
};

class Dtdc : public ShippingStrategy {                          // NEW class, nothing edited
public:
    double cost(double weight) const override { return 35 + 14 * weight; }
    string name() const override { return "DTDC"; }
};
}

// ---------------- L : Liskov Substitution ----------------
//  BAD : Square IS-A Rectangle mathematically, but not behaviourally.
namespace lsp_bad {
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
    void setWidth(int side)  override { width = side; height = side; }   // surprises callers
    void setHeight(int side) override { width = side; height = side; }
};

void clientExpectingRectangle(Rectangle& rectangle) {
    rectangle.setWidth(5);
    rectangle.setHeight(4);
    cout << "  expected area 20, got " << rectangle.area()
         << (rectangle.area() == 20 ? "  (ok)" : "  <-- LSP VIOLATED")  << "\n";
}
}

namespace lsp_good {
class Shape {                                                   // no mutable-dimension contract
public:
    virtual ~Shape() = default;
    virtual int area() const = 0;
};

class Rectangle : public Shape {
private:
    int width, height;

public:
    Rectangle(int width, int height) : width(width), height(height) {}
    int area() const override { return width * height; }
};

class Square : public Shape {                                   // sibling, not a subclass
private:
    int side;

public:
    explicit Square(int side) : side(side) {}
    int area() const override { return side * side; }
};
}

// ---------------- I : Interface Segregation ----------------
//  BAD : a fat interface forces meaningless overrides.
namespace isp_bad {
class MultiFunctionDevice {
public:
    virtual ~MultiFunctionDevice() = default;
    virtual void print(const string&) = 0;
    virtual void scan(const string&)  = 0;
    virtual void fax(const string&)   = 0;
};

class SimplePrinter : public MultiFunctionDevice {
public:
    void print(const string& document) override { cout << "  printing " << document << "\n"; }
    void scan(const string&) override { throw logic_error("scan not supported"); }  // smell
    void fax(const string&)  override { throw logic_error("fax not supported");  }
};
}

namespace isp_good {
class Printer {
public:
    virtual ~Printer() = default;
    virtual void print(const string& document) = 0;
};

class Scanner {
public:
    virtual ~Scanner() = default;
    virtual void scan(const string& document) = 0;
};

class Fax {
public:
    virtual ~Fax() = default;
    virtual void fax(const string& document) = 0;
};

class SimplePrinter : public Printer {                          // implements only what it can
public:
    void print(const string& document) override { cout << "  printing " << document << "\n"; }
};

class OfficeMachine : public Printer, public Scanner, public Fax {
public:
    void print(const string& document) override { cout << "  office printing " << document << "\n"; }
    void scan(const string& document)  override { cout << "  office scanning " << document << "\n"; }
    void fax(const string& document)   override { cout << "  office faxing "   << document << "\n"; }
};
}

// ---------------- D : Dependency Inversion ----------------
//  BAD : the high-level service hard-codes and constructs the low-level detail.
namespace dip_bad {
class MySqlUserRepository {
public:
    void save(const string& user) { cout << "  mysql save " << user << "\n"; }
};

class SignupService {
private:
    MySqlUserRepository repository;                             // concrete + untestable

public:
    void signup(const string& user) { repository.save(user); }
};
}

namespace dip_good {
class UserRepository {                                          // the abstraction both share
public:
    virtual ~UserRepository() = default;
    virtual void save(const string& user) = 0;
};

class MySqlUserRepository : public UserRepository {
public:
    void save(const string& user) override { cout << "  mysql save " << user << "\n"; }
};

class InMemoryUserRepository : public UserRepository {          // trivial test double
private:
    vector<string> users;

public:
    void save(const string& user) override {
        users.push_back(user);
        cout << "  in-memory save " << user << " (count=" << users.size() << ")\n";
    }
};

class SignupService {
private:
    UserRepository& repository;                                 // injected abstraction

public:
    explicit SignupService(UserRepository& repository) : repository(repository) {}
    void signup(const string& user) { repository.save(user); }
};
}

void demo() {
    banner("PART 12 : SOLID");

    cout << "-- S : responsibilities split --\n";
    srp_good::Invoice invoice(1000, 0.18);
    srp_good::InvoiceRepository().save(invoice);
    srp_good::InvoiceMailer().send(srp_good::InvoicePdfRenderer().render(invoice), "a@b.com");

    cout << "-- O : extend by adding a class --\n";
    vector<unique_ptr<ocp_good::ShippingStrategy>> couriers;
    couriers.push_back(make_unique<ocp_good::Bluedart>());
    couriers.push_back(make_unique<ocp_good::Delhivery>());
    couriers.push_back(make_unique<ocp_good::Dtdc>());
    for (const auto& courier : couriers)
        cout << "  " << courier->name() << " cost(2kg)=" << courier->cost(2) << "\n";

    cout << "-- L : violation vs fix --\n";
    lsp_bad::Rectangle rectangle;
    lsp_bad::Square    square;
    lsp_bad::clientExpectingRectangle(rectangle);
    lsp_bad::clientExpectingRectangle(square);
    vector<unique_ptr<lsp_good::Shape>> shapes;
    shapes.push_back(make_unique<lsp_good::Rectangle>(5, 4));
    shapes.push_back(make_unique<lsp_good::Square>(5));
    for (const auto& shape : shapes) cout << "  fixed area=" << shape->area() << "\n";

    cout << "-- I : segregated roles --\n";
    isp_good::SimplePrinter simplePrinter;
    simplePrinter.print("resume.pdf");
    isp_good::OfficeMachine officeMachine;
    officeMachine.scan("contract.pdf");

    cout << "-- D : inject the abstraction --\n";
    dip_good::MySqlUserRepository    production;
    dip_good::InMemoryUserRepository test;
    dip_good::SignupService(production).signup("ravindra");
    dip_good::SignupService(test).signup("test-user");
}

} // namespace part12

// ===================================================================================
// PART 13 : OTHER DESIGN PRINCIPLES WORTH QUOTING
// ===================================================================================
//  DRY   (Don't Repeat Yourself)  - every piece of knowledge has ONE authoritative home.
//                                   Beware the wrong DRY: two things that merely LOOK the
//                                   same but change for different reasons should stay apart.
//  KISS  (Keep It Simple)         - the simplest design that satisfies the requirements.
//  YAGNI (You Aren't Gonna Need It)- do not build for imagined future requirements. In an
//                                   LLD interview, state extension POINTS instead of
//                                   implementing every extension.
//  SoC   (Separation of Concerns) - layering: controller / service / repository / model.
//  Law of Demeter ("don't talk to strangers") - a method may call methods on: itself, its
//        own fields, its parameters, and objects it creates. `a.getB().getC().doD()` is a
//        train wreck; add `a.doD()` instead. Reduces coupling.
//  Program to an interface, not an implementation.
//  Favour composition over inheritance.
//  Encapsulate what varies - find the changing part and put it behind an interface.
//  Tell, Don't Ask - tell an object to do work, do not pull its data out and decide for it.
//  Principle of Least Astonishment - names and behaviour should hold no surprises.
//  Fail fast - validate at the boundary, throw early, keep invariants always true.
//  High cohesion, low coupling - the single sentence that summarises all of the above.
//        COHESION = how focused one class is (want HIGH).
//        COUPLING = how much classes know about each other (want LOW).
//  GRASP  - Information Expert, Creator, Controller, Low Coupling, High Cohesion,
//           Polymorphism, Pure Fabrication, Indirection, Protected Variations.
//  Composition root / dependency injection - build the object graph in ONE place (main),
//           so the rest of the code only ever receives its collaborators.
// ===================================================================================

// ===================================================================================
// PART 14 : UML CHEAT-SHEET (what to draw on the whiteboard)
// ===================================================================================
//  CLASS BOX
//      +---------------------------+
//      |        ClassName          |   <- italic name = abstract, <<interface>> stereotype
//      +---------------------------+
//      | - privateField : Type     |   - private   # protected   + public   ~ package
//      | + publicField  : Type     |   underlined  = static
//      +---------------------------+
//      | + method(arg : Type) : Ret|
//      +---------------------------+
//
//  RELATIONSHIP ARROWS
//      Inheritance / generalisation   A <|---- B        solid line, hollow triangle
//      Interface realisation          A <|- - - B       dashed line, hollow triangle
//      Composition (strong owner)     A <>---- B        FILLED diamond at the owner
//      Aggregation (weak owner)       A <>---- B        HOLLOW diamond at the owner
//      Association                    A ------- B       solid line (arrow if directed)
//      Dependency                     A - - - -> B      dashed arrow
//      Multiplicity                   1, 0..1, 1..*, *, 2..5   written at each end
//
//  SEQUENCE DIAGRAM (for the flow question that always follows)
//      Actor -> Controller -> Service -> Repository, lifelines vertical, time downward,
//      solid arrow = call, dashed arrow = return, `alt`/`loop` boxes for branching.
//
//  STATE DIAGRAM : useful for Order, Booking, Ride, Payment lifecycles.
//      CREATED -> CONFIRMED -> SHIPPED -> DELIVERED, plus CANCELLED / REFUNDED edges.
// ===================================================================================

// ===================================================================================
// PART 15 : DESIGN PATTERNS - CREATIONAL  (how objects get created)
// ===================================================================================
//  Singleton        one instance, global access point
//  Factory Method   a virtual method decides WHICH concrete class to instantiate
//  Simple Factory   (not GoF) one place with a switch that returns the right product
//  Abstract Factory a family of related products created together, consistently
//  Builder          construct a complex object step by step, avoid a telescoping ctor
//  Prototype        create by CLONING an existing configured instance
//  Object Pool      reuse expensive objects instead of recreating them
// -----------------------------------------------------------------------------------
namespace part15 {

// ---------------- SINGLETON ----------------
//  Use for: config, logger, connection pool, cache. Abuse warning: it is global state,
//  hides dependencies, and hurts testability - prefer DI unless you truly need one.
//  Meyers singleton (below) is lazy AND thread-safe since C++11 (static local init is
//  guaranteed to run exactly once).
class AppConfig {
private:
    map<string, string> values;

    AppConfig() { values["env"] = "production"; }               // private ctor

public:
    AppConfig(const AppConfig&)            = delete;            // no copies
    AppConfig& operator=(const AppConfig&) = delete;

    static AppConfig& instance() {                              // Meyers singleton
        static AppConfig only;
        return only;
    }

    void set(const string& key, const string& value) { values[key] = value; }
    string get(const string& key) const {
        auto found = values.find(key);
        return found == values.end() ? "" : found->second;
    }
};

//  Alternative when you need a pointer + explicit init: double-checked locking / call_once
class Registry {
private:
    static unique_ptr<Registry> only;
    static once_flag initialised;

    Registry() = default;

public:
    static Registry& instance() {
        call_once(initialised, [] { only.reset(new Registry()); });
        return *only;
    }

    void ping() const { cout << "  Registry singleton via call_once\n"; }
};

unique_ptr<Registry> Registry::only;
once_flag Registry::initialised;

// ---------------- FACTORY METHOD ----------------
//  The creator declares a virtual factory method; subclasses decide the concrete product.
class Transport {
public:
    virtual ~Transport() = default;
    virtual void deliver(const string& parcel) const = 0;
};

class Truck : public Transport {
public:
    void deliver(const string& parcel) const override { cout << "  road delivery: " << parcel << "\n"; }
};

class Ship : public Transport {
public:
    void deliver(const string& parcel) const override { cout << "  sea delivery: " << parcel << "\n"; }
};

class Logistics {                                               // the Creator
public:
    virtual ~Logistics() = default;

    virtual unique_ptr<Transport> createTransport() const = 0;   // the factory METHOD

    void planDelivery(const string& parcel) const {              // template-method style flow
        unique_ptr<Transport> transport = createTransport();
        cout << "  planning...\n";
        transport->deliver(parcel);
    }
};

class RoadLogistics : public Logistics {
public:
    unique_ptr<Transport> createTransport() const override { return make_unique<Truck>(); }
};

class SeaLogistics : public Logistics {
public:
    unique_ptr<Transport> createTransport() const override { return make_unique<Ship>(); }
};

// ---------------- SIMPLE FACTORY + SELF-REGISTERING FACTORY ----------------
//  A registry map keeps the factory itself open/closed : new types register themselves
//  instead of being added to a switch.
class TransportFactory {
private:
    map<string, function<unique_ptr<Transport>()>> creators;

public:
    static TransportFactory& instance() {
        static TransportFactory only;
        return only;
    }

    void registerType(const string& key, function<unique_ptr<Transport>()> creator) {
        creators[key] = std::move(creator);
    }

    unique_ptr<Transport> create(const string& key) const {
        auto found = creators.find(key);
        if (found == creators.end()) throw invalid_argument("unknown transport: " + key);
        return found->second();
    }
};

// ---------------- ABSTRACT FACTORY ----------------
//  A factory of factories: guarantees the produced objects belong to the same family.
class Button {
public:
    virtual ~Button() = default;
    virtual void render() const = 0;
};

class Checkbox {
public:
    virtual ~Checkbox() = default;
    virtual void render() const = 0;
};

class WindowsButton   : public Button   { public: void render() const override { cout << "  [Windows button]\n"; } };
class WindowsCheckbox : public Checkbox { public: void render() const override { cout << "  [Windows checkbox]\n"; } };
class MacButton       : public Button   { public: void render() const override { cout << "  (Mac button)\n"; } };
class MacCheckbox     : public Checkbox { public: void render() const override { cout << "  (Mac checkbox)\n"; } };

class UiFactory {                                               // the ABSTRACT factory
public:
    virtual ~UiFactory() = default;
    virtual unique_ptr<Button>   createButton()   const = 0;
    virtual unique_ptr<Checkbox> createCheckbox() const = 0;
};

class WindowsFactory : public UiFactory {
public:
    unique_ptr<Button>   createButton()   const override { return make_unique<WindowsButton>(); }
    unique_ptr<Checkbox> createCheckbox() const override { return make_unique<WindowsCheckbox>(); }
};

class MacFactory : public UiFactory {
public:
    unique_ptr<Button>   createButton()   const override { return make_unique<MacButton>(); }
    unique_ptr<Checkbox> createCheckbox() const override { return make_unique<MacCheckbox>(); }
};

void renderDialog(const UiFactory& factory) {                   // client sees only abstractions
    factory.createButton()->render();
    factory.createCheckbox()->render();
}

// ---------------- BUILDER ----------------
//  Solves the telescoping-constructor / too-many-optional-params problem and lets you
//  validate the whole object once at build() time. Fluent interface returns *this.
struct HttpRequestParts {
    string method = "GET";
    string url;
    string body;
    map<string, string> headers;
    int    timeoutSeconds = 30;
};

class HttpRequest {
private:
    HttpRequestParts parts;

    explicit HttpRequest(HttpRequestParts parts) : parts(std::move(parts)) {}

public:
    class Builder {
    private:
        HttpRequestParts parts;

    public:
        Builder& method(const string& method) { parts.method = method; return *this; }
        Builder& url(const string& url)       { parts.url = url;       return *this; }
        Builder& body(const string& body)     { parts.body = body;     return *this; }
        Builder& timeout(int seconds)         { parts.timeoutSeconds = seconds; return *this; }
        Builder& header(const string& key, const string& value) {
            parts.headers[key] = value;
            return *this;
        }

        HttpRequest build() const {                              // validate once, at the end
            if (parts.url.empty()) throw invalid_argument("url is required");
            if (parts.method == "POST" && parts.body.empty())
                throw invalid_argument("POST requires a body");
            return HttpRequest(parts);
        }
    };

    void describe() const {
        cout << "  " << parts.method << " " << parts.url << " timeout=" << parts.timeoutSeconds
             << " headers=" << parts.headers.size()
             << " body=" << (parts.body.empty() ? "-" : parts.body) << "\n";
    }
};

// ---------------- PROTOTYPE ----------------
//  Copy a fully configured object instead of rebuilding it. Needs a virtual clone().
class Document {
protected:
    string title;
    vector<string> sections;

public:
    virtual ~Document() = default;

    virtual unique_ptr<Document> clone() const = 0;
    virtual string kind() const = 0;

    void setTitle(const string& title) { this->title = title; }
    void addSection(const string& section) { sections.push_back(section); }

    void print() const {
        cout << "  " << kind() << " \"" << title << "\" sections=" << sections.size() << "\n";
    }
};

class Contract : public Document {
public:
    unique_ptr<Document> clone() const override { return make_unique<Contract>(*this); }
    string kind() const override { return "Contract"; }
};

// ---------------- OBJECT POOL ----------------
class DatabaseConnection {
private:
    int id;

public:
    explicit DatabaseConnection(int id) : id(id) { cout << "  expensive connect #" << id << "\n"; }
    void query(const string& sql) const { cout << "  conn#" << id << " runs " << sql << "\n"; }
};

class ConnectionPool {
private:
    vector<shared_ptr<DatabaseConnection>> available;
    int created = 0;
    int maxSize;

public:
    explicit ConnectionPool(int maxSize) : maxSize(maxSize) {}

    shared_ptr<DatabaseConnection> acquire() {
        if (!available.empty()) {
            shared_ptr<DatabaseConnection> reused = available.back();
            available.pop_back();
            cout << "  reused a pooled connection\n";
            return reused;
        }
        if (created >= maxSize) throw runtime_error("pool exhausted");
        return make_shared<DatabaseConnection>(++created);
    }

    void release(shared_ptr<DatabaseConnection> connection) { available.push_back(std::move(connection)); }
};

void demo() {
    banner("PART 15 : CREATIONAL PATTERNS");

    cout << "-- Singleton --\n";
    AppConfig::instance().set("region", "ap-south-1");
    cout << "  env=" << AppConfig::instance().get("env")
         << " region=" << AppConfig::instance().get("region") << "\n";
    cout << "  same instance? "
         << (&AppConfig::instance() == &AppConfig::instance() ? "yes" : "no") << "\n";
    Registry::instance().ping();

    cout << "-- Factory Method --\n";
    RoadLogistics().planDelivery("laptop");
    SeaLogistics().planDelivery("container");

    cout << "-- Self-registering factory --\n";
    TransportFactory::instance().registerType("truck", [] { return unique_ptr<Transport>(make_unique<Truck>()); });
    TransportFactory::instance().registerType("ship",  [] { return unique_ptr<Transport>(make_unique<Ship>()); });
    TransportFactory::instance().create("ship")->deliver("machinery");

    cout << "-- Abstract Factory --\n";
    renderDialog(WindowsFactory());
    renderDialog(MacFactory());

    cout << "-- Builder --\n";
    HttpRequest request = HttpRequest::Builder()
                              .method("POST")
                              .url("https://api.emoha.com/v1/leads")
                              .header("Authorization", "Bearer xyz")
                              .header("Content-Type", "application/json")
                              .body("{\"name\":\"test\"}")
                              .timeout(15)
                              .build();
    request.describe();
    try {
        HttpRequest::Builder().method("POST").url("/x").build();
    } catch (const exception& e) {
        cout << "  builder validation: " << e.what() << "\n";
    }

    cout << "-- Prototype --\n";
    Contract templateContract;
    templateContract.setTitle("Standard MSA");
    templateContract.addSection("Scope");
    templateContract.addSection("Payment terms");
    unique_ptr<Document> copyForClient = templateContract.clone();
    copyForClient->setTitle("MSA - Acme Corp");
    copyForClient->addSection("Acme SLA");
    templateContract.print();
    copyForClient->print();

    cout << "-- Object Pool --\n";
    ConnectionPool pool(2);
    shared_ptr<DatabaseConnection> first = pool.acquire();
    first->query("SELECT 1");
    pool.release(first);
    pool.acquire()->query("SELECT 2");
}

} // namespace part15

// ===================================================================================
// PART 16 : DESIGN PATTERNS - STRUCTURAL  (how objects are composed)
// ===================================================================================
//  Adapter    make an incompatible interface fit the one the client expects
//  Decorator  add responsibilities to one object dynamically, by wrapping it
//  Facade     one simple entry point over a complicated subsystem
//  Composite  treat a single object and a tree of objects uniformly
//  Proxy      a stand-in that controls access (lazy, remote, protection, caching)
//  Bridge     split abstraction from implementation so both vary independently
//  Flyweight  share immutable intrinsic state across many objects to save memory
//
//  DECORATOR vs PROXY vs ADAPTER (a favourite interview question)
//    Adapter   changes the interface, keeps the behaviour.
//    Decorator keeps the interface, adds behaviour, stackable, client passes the wrappee in.
//    Proxy     keeps the interface, controls ACCESS, usually creates/owns the real subject.
// -----------------------------------------------------------------------------------
namespace part16 {

// ---------------- ADAPTER ----------------
class JsonAnalytics {                                            // what our code wants
public:
    virtual ~JsonAnalytics() = default;
    virtual void track(const string& jsonEvent) = 0;
};

class LegacyXmlAnalytics {                                       // the incompatible legacy SDK
public:
    void sendXml(const string& xmlEvent) const { cout << "  legacy XML sent: " << xmlEvent << "\n"; }
};

class XmlAnalyticsAdapter : public JsonAnalytics {                // object adapter (composition)
private:
    LegacyXmlAnalytics& legacy;

public:
    explicit XmlAnalyticsAdapter(LegacyXmlAnalytics& legacy) : legacy(legacy) {}

    void track(const string& jsonEvent) override {
        legacy.sendXml("<event>" + jsonEvent + "</event>");       // translate the call
    }
};

// ---------------- DECORATOR ----------------
class Coffee {
public:
    virtual ~Coffee() = default;
    virtual double cost()        const = 0;
    virtual string description() const = 0;
};

class Espresso : public Coffee {
public:
    double cost()        const override { return 80; }
    string description() const override { return "Espresso"; }
};

class CoffeeDecorator : public Coffee {                           // base wrapper
protected:
    unique_ptr<Coffee> inner;

public:
    explicit CoffeeDecorator(unique_ptr<Coffee> inner) : inner(std::move(inner)) {}
};

class WithMilk : public CoffeeDecorator {
public:
    using CoffeeDecorator::CoffeeDecorator;
    double cost()        const override { return inner->cost() + 20; }
    string description() const override { return inner->description() + " + milk"; }
};

class WithCaramel : public CoffeeDecorator {
public:
    using CoffeeDecorator::CoffeeDecorator;
    double cost()        const override { return inner->cost() + 35; }
    string description() const override { return inner->description() + " + caramel"; }
};

class WithExtraShot : public CoffeeDecorator {
public:
    using CoffeeDecorator::CoffeeDecorator;
    double cost()        const override { return inner->cost() + 40; }
    string description() const override { return inner->description() + " + extra shot"; }
};

// ---------------- FACADE ----------------
class VideoDecoder   { public: void decode(const string& file) const { cout << "  decoding " << file << "\n"; } };
class AudioExtractor { public: void extract(const string& file) const { cout << "  extracting audio from " << file << "\n"; } };
class Compressor     { public: void compress(const string& codec) const { cout << "  compressing with " << codec << "\n"; } };
class Uploader       { public: void upload(const string& target) const { cout << "  uploading to " << target << "\n"; } };

class VideoConversionFacade {                                     // one call hides four systems
private:
    VideoDecoder   decoder;
    AudioExtractor audioExtractor;
    Compressor     compressor;
    Uploader       uploader;

public:
    void convertAndPublish(const string& file, const string& codec) const {
        decoder.decode(file);
        audioExtractor.extract(file);
        compressor.compress(codec);
        uploader.upload("s3://emoha-media");
    }
};

// ---------------- COMPOSITE ----------------
class FileSystemNode {
protected:
    string name;

public:
    explicit FileSystemNode(string name) : name(std::move(name)) {}
    virtual ~FileSystemNode() = default;

    virtual long size() const = 0;
    virtual void print(int depth = 0) const = 0;

    const string& getName() const { return name; }
};

class FileNode : public FileSystemNode {                          // LEAF
private:
    long bytes;

public:
    FileNode(string name, long bytes) : FileSystemNode(std::move(name)), bytes(bytes) {}

    long size() const override { return bytes; }
    void print(int depth = 0) const override {
        cout << "  " << string(depth * 2, ' ') << "- " << name << " (" << bytes << "B)\n";
    }
};

class DirectoryNode : public FileSystemNode {                     // COMPOSITE
private:
    vector<unique_ptr<FileSystemNode>> children;

public:
    using FileSystemNode::FileSystemNode;

    DirectoryNode& add(unique_ptr<FileSystemNode> child) {
        children.push_back(std::move(child));
        return *this;
    }

    long size() const override {                                  // recursion over the tree
        long total = 0;
        for (const auto& child : children) total += child->size();
        return total;
    }

    void print(int depth = 0) const override {
        cout << "  " << string(depth * 2, ' ') << "+ " << name << "/ (" << size() << "B)\n";
        for (const auto& child : children) child->print(depth + 1);
    }
};

// ---------------- PROXY ----------------
class Image {
public:
    virtual ~Image() = default;
    virtual void display() = 0;
};

class HighResolutionImage : public Image {                        // the expensive real subject
private:
    string path;

public:
    explicit HighResolutionImage(string path) : path(std::move(path)) {
        cout << "  loading " << this->path << " from disk (expensive)\n";
    }
    void display() override { cout << "  showing " << path << "\n"; }
};

class LazyImageProxy : public Image {                             // virtual (lazy-load) proxy
private:
    string path;
    unique_ptr<HighResolutionImage> real;

public:
    explicit LazyImageProxy(string path) : path(std::move(path)) {}

    void display() override {
        if (!real) real = make_unique<HighResolutionImage>(path);  // created on first use
        real->display();
    }
};

class ProtectedImageProxy : public Image {                        // protection proxy
private:
    unique_ptr<Image> real;
    string role;

public:
    ProtectedImageProxy(unique_ptr<Image> real, string role)
        : real(std::move(real)), role(std::move(role)) {}

    void display() override {
        if (role != "admin") { cout << "  access denied for role " << role << "\n"; return; }
        real->display();
    }
};

// ---------------- BRIDGE ----------------
//  Two independent dimensions (remote type x device type) would need M*N subclasses with
//  inheritance. Bridge keeps them separate: M + N classes, combined at runtime.
class DeviceImplementation {                                     // the IMPLEMENTOR dimension
public:
    virtual ~DeviceImplementation() = default;
    virtual void powerOn()  = 0;
    virtual void setVolume(int level) = 0;
    virtual string label() const = 0;
};

class Television : public DeviceImplementation {
public:
    void powerOn() override { cout << "  TV on\n"; }
    void setVolume(int level) override { cout << "  TV volume " << level << "\n"; }
    string label() const override { return "TV"; }
};

class Speaker : public DeviceImplementation {
public:
    void powerOn() override { cout << "  Speaker on\n"; }
    void setVolume(int level) override { cout << "  Speaker volume " << level << "\n"; }
    string label() const override { return "Speaker"; }
};

class RemoteControl {                                            // the ABSTRACTION dimension
protected:
    DeviceImplementation& device;                                // the "bridge"

public:
    explicit RemoteControl(DeviceImplementation& device) : device(device) {}
    virtual ~RemoteControl() = default;

    virtual void turnOn() { device.powerOn(); }
};

class AdvancedRemote : public RemoteControl {
public:
    using RemoteControl::RemoteControl;

    void turnOn() override {
        cout << "  advanced remote controlling a " << device.label() << "\n";
        device.powerOn();
    }
    void mute() { device.setVolume(0); }
};

// ---------------- FLYWEIGHT ----------------
//  Split state: INTRINSIC (shared, immutable -> the flyweight) and EXTRINSIC (passed in).
class TreeType {                                                 // intrinsic, shared
private:
    string species;
    string texture;

public:
    TreeType(string species, string texture)
        : species(std::move(species)), texture(std::move(texture)) {
        cout << "  created heavy TreeType " << this->species << "\n";
    }

    void draw(int x, int y) const {                              // extrinsic passed as args
        cout << "  drawing " << species << " at (" << x << "," << y << ")\n";
    }
};

class TreeTypeFactory {
private:
    static unordered_map<string, shared_ptr<TreeType>> cache;

public:
    static shared_ptr<TreeType> get(const string& species, const string& texture) {
        string key = species + "|" + texture;
        auto found = cache.find(key);
        if (found != cache.end()) return found->second;
        return cache[key] = make_shared<TreeType>(species, texture);
    }

    static size_t distinctTypes() { return cache.size(); }
};

unordered_map<string, shared_ptr<TreeType>> TreeTypeFactory::cache;

class Tree {                                                     // light : coords + shared ptr
private:
    int x, y;
    shared_ptr<TreeType> type;

public:
    Tree(int x, int y, shared_ptr<TreeType> type) : x(x), y(y), type(std::move(type)) {}
    void draw() const { type->draw(x, y); }
};

void demo() {
    banner("PART 16 : STRUCTURAL PATTERNS");

    cout << "-- Adapter --\n";
    LegacyXmlAnalytics legacy;
    XmlAnalyticsAdapter analytics(legacy);
    analytics.track("{\"event\":\"login\"}");

    cout << "-- Decorator (stackable, runtime) --\n";
    unique_ptr<Coffee> order = make_unique<Espresso>();
    order = make_unique<WithMilk>(std::move(order));
    order = make_unique<WithCaramel>(std::move(order));
    order = make_unique<WithExtraShot>(std::move(order));
    cout << "  " << order->description() << " = " << order->cost() << "\n";

    cout << "-- Facade --\n";
    VideoConversionFacade().convertAndPublish("elder-yoga.mov", "h264");

    cout << "-- Composite --\n";
    auto root = make_unique<DirectoryNode>("project");
    auto sourceDir = make_unique<DirectoryNode>("src");
    sourceDir->add(make_unique<FileNode>("main.cpp", 1200));
    sourceDir->add(make_unique<FileNode>("util.cpp", 800));
    auto docsDir = make_unique<DirectoryNode>("docs");
    docsDir->add(make_unique<FileNode>("README.md", 300));
    root->add(std::move(sourceDir));
    root->add(std::move(docsDir));
    root->add(make_unique<FileNode>("CMakeLists.txt", 150));
    root->print();

    cout << "-- Proxy --\n";
    LazyImageProxy lazy("banner-4k.png");
    cout << "  proxy created, nothing loaded yet\n";
    lazy.display();
    lazy.display();
    ProtectedImageProxy guarded(make_unique<LazyImageProxy>("payroll.png"), "viewer");
    guarded.display();

    cout << "-- Bridge --\n";
    Television television;
    Speaker    speaker;
    AdvancedRemote televisionRemote(television);
    AdvancedRemote speakerRemote(speaker);
    televisionRemote.turnOn();
    speakerRemote.turnOn();
    speakerRemote.mute();

    cout << "-- Flyweight --\n";
    vector<Tree> forest;
    for (int i = 0; i < 4; ++i) forest.emplace_back(i, i * 2, TreeTypeFactory::get("Neem", "neem.png"));
    forest.emplace_back(99, 99, TreeTypeFactory::get("Banyan", "banyan.png"));
    for (const Tree& tree : forest) tree.draw();
    cout << "  " << forest.size() << " trees share only "
         << TreeTypeFactory::distinctTypes() << " heavy TreeType objects\n";
}

} // namespace part16

// ===================================================================================
// PART 17 : DESIGN PATTERNS - BEHAVIOURAL  (how objects communicate)
// ===================================================================================
//  Strategy                swap an algorithm at runtime
//  Observer                publish/subscribe, one-to-many notification
//  Command                 wrap a request as an object -> queue, log, undo
//  State                   behaviour changes with internal state; kills the giant switch
//  Template Method         base fixes the algorithm's skeleton, hooks vary
//  Chain of Responsibility pass a request along handlers until one handles it
//  Iterator                traverse a collection without exposing its structure
//  Mediator                central hub replaces N-to-N direct references
//  Memento                 capture/restore state without breaking encapsulation
//  Visitor                 add new operations to a class hierarchy without editing it
//  Null Object             a do-nothing implementation removes null checks
//
//  STRATEGY vs STATE : structurally identical (composition + interface + delegation).
//    Strategy: the CLIENT picks the algorithm; strategies are independent of each other.
//    State   : the STATE objects trigger the transitions themselves and know each other.
// -----------------------------------------------------------------------------------
namespace part17 {

// ---------------- STRATEGY ----------------
class SortStrategy {
public:
    virtual ~SortStrategy() = default;
    virtual void sort(vector<int>& data) const = 0;
    virtual string name() const = 0;
};

class QuickSort : public SortStrategy {
public:
    void sort(vector<int>& data) const override { std::sort(data.begin(), data.end()); }
    string name() const override { return "QuickSort"; }
};

class BubbleSort : public SortStrategy {
public:
    void sort(vector<int>& data) const override {
        for (size_t i = 0; i + 1 < data.size(); ++i)
            for (size_t j = 0; j + 1 < data.size() - i; ++j)
                if (data[j] > data[j + 1]) std::swap(data[j], data[j + 1]);
    }
    string name() const override { return "BubbleSort"; }
};

class Sorter {                                                   // the CONTEXT
private:
    unique_ptr<SortStrategy> strategy;

public:
    explicit Sorter(unique_ptr<SortStrategy> strategy) : strategy(std::move(strategy)) {}

    void setStrategy(unique_ptr<SortStrategy> replacement) { strategy = std::move(replacement); }

    void run(vector<int> data) const {                           // copy, sort, show
        strategy->sort(data);
        cout << "  " << strategy->name() << " ->";
        for (int value : data) cout << " " << value;
        cout << "\n";
    }
};

// ---------------- OBSERVER ----------------
class OrderObserver {
public:
    virtual ~OrderObserver() = default;
    virtual void onOrderShipped(const string& orderId) = 0;
    virtual string observerName() const = 0;
};

class OrderPublisher {                                           // the SUBJECT
private:
    vector<weak_ptr<OrderObserver>> observers;                   // weak -> no dangling, no cycle

public:
    void subscribe(const shared_ptr<OrderObserver>& observer) { observers.push_back(observer); }

    void unsubscribe(const shared_ptr<OrderObserver>& target) {
        observers.erase(remove_if(observers.begin(), observers.end(),
                                  [&target](const weak_ptr<OrderObserver>& stored) {
                                      return stored.expired() || stored.lock() == target;
                                  }),
                        observers.end());
    }

    void shipOrder(const string& orderId) {
        cout << "  order " << orderId << " shipped -> notifying\n";
        for (const weak_ptr<OrderObserver>& stored : observers)
            if (shared_ptr<OrderObserver> observer = stored.lock())
                observer->onOrderShipped(orderId);
    }
};

class EmailObserver : public OrderObserver {
public:
    void onOrderShipped(const string& orderId) override { cout << "    email for " << orderId << "\n"; }
    string observerName() const override { return "email"; }
};

class SmsObserver : public OrderObserver {
public:
    void onOrderShipped(const string& orderId) override { cout << "    sms for " << orderId << "\n"; }
    string observerName() const override { return "sms"; }
};

class WarehouseObserver : public OrderObserver {
public:
    void onOrderShipped(const string& orderId) override { cout << "    stock updated for " << orderId << "\n"; }
    string observerName() const override { return "warehouse"; }
};

// ---------------- COMMAND (with undo) ----------------
class TextEditor {
private:
    string content;

public:
    void append(const string& text) { content += text; }
    void removeLast(size_t count)   { content.erase(content.size() - count); }
    const string& text() const { return content; }
};

class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo()    = 0;
    virtual string label() const = 0;
};

class AppendCommand : public Command {
private:
    TextEditor& editor;
    string      text;

public:
    AppendCommand(TextEditor& editor, string text) : editor(editor), text(std::move(text)) {}

    void execute() override { editor.append(text); }
    void undo()    override { editor.removeLast(text.size()); }
    string label() const override { return "append(\"" + text + "\")"; }
};

class CommandInvoker {                                           // history enables undo
private:
    vector<unique_ptr<Command>> history;

public:
    void run(unique_ptr<Command> command) {
        command->execute();
        cout << "  ran " << command->label() << "\n";
        history.push_back(std::move(command));
    }

    void undoLast() {
        if (history.empty()) { cout << "  nothing to undo\n"; return; }
        cout << "  undoing " << history.back()->label() << "\n";
        history.back()->undo();
        history.pop_back();
    }
};

// ---------------- STATE ----------------
class VendingMachine;

class VendingState {
public:
    virtual ~VendingState() = default;
    virtual void insertCoin(VendingMachine& machine) = 0;
    virtual void selectItem(VendingMachine& machine) = 0;
    virtual string name() const = 0;
};

class VendingMachine {
private:
    unique_ptr<VendingState> state;

public:
    explicit VendingMachine(unique_ptr<VendingState> initial) : state(std::move(initial)) {}

    void transitionTo(unique_ptr<VendingState> next) {
        cout << "    state: " << state->name() << " -> " << next->name() << "\n";
        state = std::move(next);
    }

    void insertCoin() { state->insertCoin(*this); }
    void selectItem() { state->selectItem(*this); }
};

class IdleState;
class HasCoinState;

class IdleState : public VendingState {
public:
    void insertCoin(VendingMachine& machine) override;
    void selectItem(VendingMachine&) override { cout << "    insert a coin first\n"; }
    string name() const override { return "Idle"; }
};

class HasCoinState : public VendingState {
public:
    void insertCoin(VendingMachine&) override { cout << "    coin already inserted\n"; }
    void selectItem(VendingMachine& machine) override;
    string name() const override { return "HasCoin"; }
};

void IdleState::insertCoin(VendingMachine& machine) {
    cout << "    coin accepted\n";
    machine.transitionTo(make_unique<HasCoinState>());
}

void HasCoinState::selectItem(VendingMachine& machine) {
    cout << "    dispensing item\n";
    machine.transitionTo(make_unique<IdleState>());
}

// ---------------- TEMPLATE METHOD ----------------
class DataImporter {
public:
    virtual ~DataImporter() = default;

    void import(const string& source) {                          // the TEMPLATE METHOD: final flow
        openSource(source);
        vector<string> rows = readRows();
        for (const string& row : rows) if (validate(row)) persist(row);
        afterImport();                                           // hook with a default
        closeSource();
    }

protected:
    virtual void openSource(const string& source) { cout << "  opening " << source << "\n"; }
    virtual vector<string> readRows() = 0;                       // subclass MUST supply
    virtual bool validate(const string& row) { return !row.empty(); }   // overridable default
    virtual void persist(const string& row) { cout << "  persisted " << row << "\n"; }
    virtual void afterImport() {}                                // optional hook
    virtual void closeSource() { cout << "  closed\n"; }
};

class CsvImporter : public DataImporter {
protected:
    vector<string> readRows() override { return {"a,1", "b,2", ""}; }
    void afterImport() override { cout << "  csv summary written\n"; }
};

// ---------------- CHAIN OF RESPONSIBILITY ----------------
struct ExpenseRequest {
    string purpose;
    double amount;
};

class Approver {
private:
    unique_ptr<Approver> next;

protected:
    virtual double limit() const = 0;
    virtual string role()  const = 0;

public:
    virtual ~Approver() = default;

    Approver* setNext(unique_ptr<Approver> nextApprover) {
        next = std::move(nextApprover);
        return next.get();
    }

    void handle(const ExpenseRequest& request) const {
        if (request.amount <= limit()) {
            cout << "  " << role() << " approved " << request.amount
                 << " for " << request.purpose << "\n";
            return;
        }
        if (next) { next->handle(request); return; }
        cout << "  nobody can approve " << request.amount << "\n";
    }
};

class TeamLead : public Approver {
protected:
    double limit() const override { return 5000; }
    string role()  const override { return "TeamLead"; }
};

class Manager : public Approver {
protected:
    double limit() const override { return 50000; }
    string role()  const override { return "Manager"; }
};

class Director : public Approver {
protected:
    double limit() const override { return 500000; }
    string role()  const override { return "Director"; }
};

// ---------------- ITERATOR ----------------
template <typename T>
class Playlist {
private:
    vector<T> tracks;

public:
    void add(const T& track) { tracks.push_back(track); }

    class Iterator {
    private:
        const vector<T>& tracks;
        size_t index;

    public:
        Iterator(const vector<T>& tracks, size_t index) : tracks(tracks), index(index) {}

        const T& operator*() const { return tracks[index]; }
        Iterator& operator++() { ++index; return *this; }
        bool operator!=(const Iterator& other) const { return index != other.index; }
    };

    Iterator begin() const { return Iterator(tracks, 0); }
    Iterator end()   const { return Iterator(tracks, tracks.size()); }
};

// ---------------- MEDIATOR ----------------
class ChatUser;

class ChatRoom {                                                 // the MEDIATOR
private:
    vector<ChatUser*> members;

public:
    void join(ChatUser* user) { members.push_back(user); }
    void broadcast(const string& message, ChatUser* sender);
};

class ChatUser {
private:
    string   name;
    ChatRoom& room;

public:
    ChatUser(string name, ChatRoom& room) : name(std::move(name)), room(room) { room.join(this); }

    const string& getName() const { return name; }

    void send(const string& message) {                           // no direct user-to-user refs
        cout << "  " << name << " sends: " << message << "\n";
        room.broadcast(message, this);
    }

    void receive(const string& message, const string& from) const {
        cout << "    " << name << " received from " << from << ": " << message << "\n";
    }
};

void ChatRoom::broadcast(const string& message, ChatUser* sender) {
    for (ChatUser* member : members)
        if (member != sender) member->receive(message, sender->getName());
}

// ---------------- MEMENTO ----------------
class EditorMemento {                                            // opaque snapshot
private:
    string savedContent;

    friend class VersionedEditor;
    explicit EditorMemento(string savedContent) : savedContent(std::move(savedContent)) {}
};

class VersionedEditor {
private:
    string content;

public:
    void type(const string& text) { content += text; }
    const string& text() const { return content; }

    EditorMemento save() const { return EditorMemento(content); }
    void restore(const EditorMemento& snapshot) { content = snapshot.savedContent; }
};

// ---------------- VISITOR ----------------
class TextElement;
class ImageElement;
class TableElement;

class DocumentVisitor {
public:
    virtual ~DocumentVisitor() = default;
    virtual void visit(const TextElement&  element) = 0;
    virtual void visit(const ImageElement& element) = 0;
    virtual void visit(const TableElement& element) = 0;
};

class DocumentElement {
public:
    virtual ~DocumentElement() = default;
    virtual void accept(DocumentVisitor& visitor) const = 0;      // DOUBLE DISPATCH
};

class TextElement : public DocumentElement {
public:
    string content = "hello world";
    void accept(DocumentVisitor& visitor) const override { visitor.visit(*this); }
};

class ImageElement : public DocumentElement {
public:
    string path = "logo.png";
    void accept(DocumentVisitor& visitor) const override { visitor.visit(*this); }
};

class TableElement : public DocumentElement {
public:
    int rows = 3, cols = 4;
    void accept(DocumentVisitor& visitor) const override { visitor.visit(*this); }
};

class HtmlExportVisitor : public DocumentVisitor {                // new operation, zero edits
public:
    void visit(const TextElement& element)  override { cout << "  <p>" << element.content << "</p>\n"; }
    void visit(const ImageElement& element) override { cout << "  <img src=\"" << element.path << "\">\n"; }
    void visit(const TableElement& element) override {
        cout << "  <table rows=" << element.rows << " cols=" << element.cols << ">\n";
    }
};

class WordCountVisitor : public DocumentVisitor {
private:
    int words = 0;

public:
    void visit(const TextElement& element) override { words += 1 + static_cast<int>(count(element.content.begin(), element.content.end(), ' ')); }
    void visit(const ImageElement&) override {}
    void visit(const TableElement& element) override { words += element.rows * element.cols; }

    int total() const { return words; }
};

// ---------------- NULL OBJECT ----------------
class Metrics {
public:
    virtual ~Metrics() = default;
    virtual void increment(const string& key) = 0;
};

class StatsdMetrics : public Metrics {
public:
    void increment(const string& key) override { cout << "  statsd++ " << key << "\n"; }
};

class NoOpMetrics : public Metrics {                              // removes `if (metrics)`
public:
    void increment(const string&) override {}
};

void demo() {
    banner("PART 17 : BEHAVIOURAL PATTERNS");

    cout << "-- Strategy --\n";
    Sorter sorter(make_unique<QuickSort>());
    sorter.run({5, 2, 9, 1});
    sorter.setStrategy(make_unique<BubbleSort>());
    sorter.run({5, 2, 9, 1});

    cout << "-- Observer --\n";
    OrderPublisher publisher;
    auto email     = make_shared<EmailObserver>();
    auto sms       = make_shared<SmsObserver>();
    auto warehouse = make_shared<WarehouseObserver>();
    publisher.subscribe(email);
    publisher.subscribe(sms);
    publisher.subscribe(warehouse);
    publisher.shipOrder("ORD-1001");
    publisher.unsubscribe(sms);
    publisher.shipOrder("ORD-1002");

    cout << "-- Command + undo --\n";
    TextEditor editor;
    CommandInvoker invoker;
    invoker.run(make_unique<AppendCommand>(editor, "Hello "));
    invoker.run(make_unique<AppendCommand>(editor, "World"));
    cout << "  text = \"" << editor.text() << "\"\n";
    invoker.undoLast();
    cout << "  text = \"" << editor.text() << "\"\n";

    cout << "-- State --\n";
    VendingMachine machine(make_unique<IdleState>());
    machine.selectItem();
    machine.insertCoin();
    machine.insertCoin();
    machine.selectItem();

    cout << "-- Template Method --\n";
    CsvImporter().import("leads.csv");

    cout << "-- Chain of Responsibility --\n";
    auto teamLead = make_unique<TeamLead>();
    Approver* chainHead = teamLead.get();
    chainHead->setNext(make_unique<Manager>())->setNext(make_unique<Director>());
    chainHead->handle({"stationery", 900});
    chainHead->handle({"laptop", 90000});
    chainHead->handle({"office lease", 9000000});

    cout << "-- Iterator (range-for on a custom container) --\n";
    Playlist<string> playlist;
    playlist.add("track-a");
    playlist.add("track-b");
    playlist.add("track-c");
    for (const string& track : playlist) cout << "  playing " << track << "\n";

    cout << "-- Mediator --\n";
    ChatRoom room;
    ChatUser amit("Amit", room);
    ChatUser bela("Bela", room);
    ChatUser chetan("Chetan", room);
    amit.send("standup in 5");

    cout << "-- Memento --\n";
    VersionedEditor versioned;
    versioned.type("draft one");
    EditorMemento checkpoint = versioned.save();
    versioned.type(" ... plus edits I regret");
    cout << "  before restore : " << versioned.text() << "\n";
    versioned.restore(checkpoint);
    cout << "  after restore  : " << versioned.text() << "\n";

    cout << "-- Visitor --\n";
    vector<unique_ptr<DocumentElement>> document;
    document.push_back(make_unique<TextElement>());
    document.push_back(make_unique<ImageElement>());
    document.push_back(make_unique<TableElement>());
    HtmlExportVisitor htmlExporter;
    WordCountVisitor  wordCounter;
    for (const auto& element : document) element->accept(htmlExporter);
    for (const auto& element : document) element->accept(wordCounter);
    cout << "  word count = " << wordCounter.total() << "\n";

    cout << "-- Null Object --\n";
    vector<unique_ptr<Metrics>> metricSinks;
    metricSinks.push_back(make_unique<StatsdMetrics>());
    metricSinks.push_back(make_unique<NoOpMetrics>());
    for (const auto& sink : metricSinks) sink->increment("signup.success");
    cout << "  no null checks needed anywhere\n";
}

} // namespace part17

// ===================================================================================
// PART 18 : MINI LLD WALK-THROUGH - PARKING LOT
// ===================================================================================
//  HOW TO ATTACK ANY LLD QUESTION (say this out loud, in this order)
//   1. CLARIFY + SCOPE      : ask 3-5 questions, then state assumptions and freeze scope.
//                             "Single lot or multi-lot? Payment online or at exit?
//                              Reservations? Do we need concurrency?"
//   2. LIST REQUIREMENTS    : functional bullets, then non-functional (concurrency,
//                             extensibility, scale). Say what is OUT of scope.
//   3. IDENTIFY ENTITIES    : pull the NOUNS out of the requirements -> candidate classes.
//                             Pull the VERBS -> methods.
//   4. DEFINE RELATIONSHIPS : is-a / has-a / uses, plus multiplicity. Draw the class diagram.
//   5. FIND WHAT VARIES     : each variation point becomes an interface
//                             (pricing -> Strategy, slot choice -> Strategy,
//                              vehicle creation -> Factory, gate events -> Observer).
//   6. WRITE THE INTERFACES FIRST, then one concrete implementation each.
//   7. WALK A FLOW END TO END : park -> ticket -> exit -> price -> pay -> free the slot.
//   8. STATE TRADE-OFFS + EXTENSIONS : "adding an EV slot type means one new enum value and
//                                       one new strategy; nothing else changes."
//   9. MENTION CONCURRENCY  : which state is shared, what lock or atomic protects it.
//  10. MENTION TESTING      : the interfaces are the seams; inject fakes.
//
//  ENTITIES BELOW : Vehicle (hierarchy), SlotType, ParkingSlot, ParkingFloor,
//                   Ticket, PricingStrategy (Strategy), SlotAllocationStrategy (Strategy),
//                   ParkingLot (facade + aggregate root).
// -----------------------------------------------------------------------------------
namespace part18 {

enum class SlotType { Motorcycle, Compact, Large, Electric };

static string slotTypeName(SlotType type) {
    switch (type) {
        case SlotType::Motorcycle: return "Motorcycle";
        case SlotType::Compact:    return "Compact";
        case SlotType::Large:      return "Large";
        case SlotType::Electric:   return "Electric";
    }
    return "Unknown";
}

class Vehicle {
private:
    string   plate;
    SlotType requiredSlot;

public:
    Vehicle(string plate, SlotType requiredSlot)
        : plate(std::move(plate)), requiredSlot(requiredSlot) {}
    virtual ~Vehicle() = default;

    const string& getPlate() const { return plate; }
    SlotType getRequiredSlot() const { return requiredSlot; }
    virtual string kind() const = 0;
};

class Motorcycle : public Vehicle {
public:
    explicit Motorcycle(string plate) : Vehicle(std::move(plate), SlotType::Motorcycle) {}
    string kind() const override { return "Motorcycle"; }
};

class Car : public Vehicle {
public:
    explicit Car(string plate) : Vehicle(std::move(plate), SlotType::Compact) {}
    string kind() const override { return "Car"; }
};

class Bus : public Vehicle {
public:
    explicit Bus(string plate) : Vehicle(std::move(plate), SlotType::Large) {}
    string kind() const override { return "Bus"; }
};

class ParkingSlot {
private:
    string   id;
    SlotType type;
    bool     occupied = false;

public:
    ParkingSlot(string id, SlotType type) : id(std::move(id)), type(type) {}

    const string& getId() const { return id; }
    SlotType getType() const { return type; }
    bool isFree() const { return !occupied; }

    bool canFit(const Vehicle& vehicle) const {
        return isFree() && type == vehicle.getRequiredSlot();
    }

    void occupy() { occupied = true; }
    void release() { occupied = false; }
};

class ParkingFloor {
private:
    int floorNumber;
    vector<shared_ptr<ParkingSlot>> slots;

public:
    explicit ParkingFloor(int floorNumber) : floorNumber(floorNumber) {}

    void addSlot(SlotType type, int count) {
        for (int i = 1; i <= count; ++i)
            slots.push_back(make_shared<ParkingSlot>(
                "F" + to_string(floorNumber) + "-" + slotTypeName(type).substr(0, 1) + to_string(i),
                type));
    }

    int getFloorNumber() const { return floorNumber; }
    const vector<shared_ptr<ParkingSlot>>& getSlots() const { return slots; }

    int freeCount() const {
        return static_cast<int>(count_if(slots.begin(), slots.end(),
                                         [](const shared_ptr<ParkingSlot>& slot) { return slot->isFree(); }));
    }
};

class SlotAllocationStrategy {                                   // VARIATION POINT 1
public:
    virtual ~SlotAllocationStrategy() = default;
    virtual shared_ptr<ParkingSlot> findSlot(const vector<ParkingFloor>& floors,
                                             const Vehicle& vehicle) const = 0;
};

class NearestFirstAllocation : public SlotAllocationStrategy {
public:
    shared_ptr<ParkingSlot> findSlot(const vector<ParkingFloor>& floors,
                                     const Vehicle& vehicle) const override {
        for (const ParkingFloor& floor : floors)
            for (const shared_ptr<ParkingSlot>& slot : floor.getSlots())
                if (slot->canFit(vehicle)) return slot;
        return nullptr;
    }
};

class MostFreeFloorAllocation : public SlotAllocationStrategy {
public:
    shared_ptr<ParkingSlot> findSlot(const vector<ParkingFloor>& floors,
                                     const Vehicle& vehicle) const override {
        const ParkingFloor* best = nullptr;
        for (const ParkingFloor& floor : floors)
            if (!best || floor.freeCount() > best->freeCount()) best = &floor;
        if (!best) return nullptr;

        for (const shared_ptr<ParkingSlot>& slot : best->getSlots())
            if (slot->canFit(vehicle)) return slot;
        return nullptr;
    }
};

class Ticket {
private:
    string ticketId;
    string plate;
    shared_ptr<ParkingSlot> slot;
    int    entryHour;

public:
    Ticket(string ticketId, string plate, shared_ptr<ParkingSlot> slot, int entryHour)
        : ticketId(std::move(ticketId)), plate(std::move(plate)),
          slot(std::move(slot)), entryHour(entryHour) {}

    const string& getId() const { return ticketId; }
    const string& getPlate() const { return plate; }
    const shared_ptr<ParkingSlot>& getSlot() const { return slot; }
    int getEntryHour() const { return entryHour; }
};

class PricingStrategy {                                          // VARIATION POINT 2
public:
    virtual ~PricingStrategy() = default;
    virtual double price(const Ticket& ticket, int exitHour) const = 0;
    virtual string name() const = 0;
};

class FlatHourlyPricing : public PricingStrategy {
private:
    double ratePerHour;

public:
    explicit FlatHourlyPricing(double ratePerHour) : ratePerHour(ratePerHour) {}

    double price(const Ticket& ticket, int exitHour) const override {
        int hours = max(1, exitHour - ticket.getEntryHour());
        return hours * ratePerHour;
    }
    string name() const override { return "FlatHourly"; }
};

class SlabPricing : public PricingStrategy {
public:
    double price(const Ticket& ticket, int exitHour) const override {
        int hours = max(1, exitHour - ticket.getEntryHour());
        if (hours <= 2) return 50;
        if (hours <= 5) return 50 + (hours - 2) * 30;
        return 50 + 3 * 30 + (hours - 5) * 20;
    }
    string name() const override { return "Slab"; }
};

class ParkingLotObserver {                                       // VARIATION POINT 3
public:
    virtual ~ParkingLotObserver() = default;
    virtual void onFull(SlotType type) = 0;
};

class DisplayBoard : public ParkingLotObserver {
public:
    void onFull(SlotType type) override {
        cout << "    [display board] " << slotTypeName(type) << " slots FULL\n";
    }
};

class ParkingLot {                                               // aggregate root + facade
private:
    vector<ParkingFloor> floors;
    unordered_map<string, shared_ptr<Ticket>> activeTickets;
    unique_ptr<SlotAllocationStrategy> allocation;
    unique_ptr<PricingStrategy> pricing;
    vector<shared_ptr<ParkingLotObserver>> observers;
    int nextTicketNumber = 1;

public:
    ParkingLot(unique_ptr<SlotAllocationStrategy> allocation, unique_ptr<PricingStrategy> pricing)
        : allocation(std::move(allocation)), pricing(std::move(pricing)) {}

    ParkingFloor& addFloor(int floorNumber) {
        floors.emplace_back(floorNumber);
        return floors.back();
    }

    void subscribe(const shared_ptr<ParkingLotObserver>& observer) { observers.push_back(observer); }

    void setPricing(unique_ptr<PricingStrategy> replacement) { pricing = std::move(replacement); }

    shared_ptr<Ticket> park(const Vehicle& vehicle, int entryHour) {
        shared_ptr<ParkingSlot> slot = allocation->findSlot(floors, vehicle);
        if (!slot) {
            cout << "  no slot for " << vehicle.kind() << " " << vehicle.getPlate() << "\n";
            for (const auto& observer : observers) observer->onFull(vehicle.getRequiredSlot());
            return nullptr;
        }

        slot->occupy();
        shared_ptr<Ticket> ticket = make_shared<Ticket>(
            "TKT-" + to_string(nextTicketNumber++), vehicle.getPlate(), slot, entryHour);
        activeTickets[ticket->getId()] = ticket;

        cout << "  " << vehicle.kind() << " " << vehicle.getPlate()
             << " parked at " << slot->getId() << " ticket=" << ticket->getId() << "\n";
        return ticket;
    }

    double unpark(const string& ticketId, int exitHour) {
        auto found = activeTickets.find(ticketId);
        if (found == activeTickets.end()) throw invalid_argument("invalid ticket " + ticketId);

        shared_ptr<Ticket> ticket = found->second;
        double amount = pricing->price(*ticket, exitHour);

        ticket->getSlot()->release();
        activeTickets.erase(found);

        cout << "  " << ticketId << " (" << ticket->getPlate() << ") exited from "
             << ticket->getSlot()->getId() << " -> " << pricing->name()
             << " charge Rs." << amount << "\n";
        return amount;
    }

    void status() const {
        cout << "  occupancy:";
        for (const ParkingFloor& floor : floors)
            cout << " floor" << floor.getFloorNumber() << "=" << floor.freeCount() << " free";
        cout << "  activeTickets=" << activeTickets.size() << "\n";
    }
};

void demo() {
    banner("PART 18 : MINI LLD - PARKING LOT");

    ParkingLot lot(make_unique<NearestFirstAllocation>(), make_unique<FlatHourlyPricing>(40));
    lot.subscribe(make_shared<DisplayBoard>());

    ParkingFloor& ground = lot.addFloor(0);
    ground.addSlot(SlotType::Motorcycle, 1);
    ground.addSlot(SlotType::Compact, 2);

    ParkingFloor& first = lot.addFloor(1);
    first.addSlot(SlotType::Compact, 1);
    first.addSlot(SlotType::Large, 1);

    Motorcycle motorcycle("MH12-BK-0001");
    Car        firstCar("MH12-CR-0002");
    Car        secondCar("MH12-CR-0003");
    Car        thirdCar("MH12-CR-0004");
    Car        fourthCar("MH12-CR-0005");
    Bus        bus("MH12-BS-0006");

    lot.status();
    shared_ptr<Ticket> bikeTicket = lot.park(motorcycle, 9);
    shared_ptr<Ticket> carTicket  = lot.park(firstCar, 10);
    lot.park(secondCar, 10);
    lot.park(thirdCar, 11);
    lot.park(fourthCar, 11);                                      // all compact slots gone
    lot.park(bus, 12);
    lot.status();

    lot.unpark(carTicket->getId(), 14);

    cout << "  switching the pricing strategy at runtime (open/closed)\n";
    lot.setPricing(make_unique<SlabPricing>());
    lot.unpark(bikeTicket->getId(), 17);
    lot.status();

    try {
        lot.unpark("TKT-999", 18);
    } catch (const exception& e) {
        cout << "  " << e.what() << "\n";
    }

    cout << "\n  Extension points, unchanged core:\n"
         << "   - new vehicle type      -> new Vehicle subclass (+ SlotType value)\n"
         << "   - new allocation rule   -> new SlotAllocationStrategy\n"
         << "   - new pricing rule      -> new PricingStrategy\n"
         << "   - new side effect       -> new ParkingLotObserver\n"
         << "   - concurrency           -> guard activeTickets + slot flags with a mutex,\n"
         << "                              or make ParkingSlot::occupied an atomic CAS\n";
}

} // namespace part18

// ===================================================================================
// PART 19 : RAPID-FIRE INTERVIEW Q&A
// ===================================================================================
//  Q  4 pillars of OOP?
//  A  Encapsulation, Abstraction, Inheritance, Polymorphism.
//     (Some add: Message Passing / Coupling+Cohesion as supporting ideas.)
//
//  Q  Abstraction vs Encapsulation?
//  A  Abstraction hides COMPLEXITY and exposes intent (design level, achieved with
//     abstract classes/interfaces). Encapsulation hides DATA and restricts access
//     (implementation level, achieved with access specifiers).
//
//  Q  Overloading vs Overriding?
//  A  Overloading: same name, DIFFERENT signature, same scope, resolved at COMPILE time.
//     Overriding : same name, SAME signature, base virtual + derived, resolved at RUNTIME.
//     Bonus - "hiding": a derived member hides a same-named base member that is NOT virtual.
//
//  Q  Can a constructor be virtual? Why not?
//  A  No. The vptr is set up BY the constructor, so no dynamic type exists yet. Use a
//     virtual clone() (Prototype) or a factory for "virtual construction".
//
//  Q  Can a destructor be virtual? Should it?
//  A  Yes, and it MUST be virtual in any class you delete through a base pointer,
//     otherwise the derived destructor never runs -> leak / UB.
//     Also: never call a virtual function's derived version from a ctor/dtor - dispatch
//     is limited to the currently-constructed level.
//
//  Q  Can a destructor be pure virtual?
//  A  Yes (`virtual ~B() = 0;`), but you must still provide a definition out of line,
//     because every derived destructor calls it.
//
//  Q  Abstract class vs Interface in C++?
//  A  An abstract class has >=1 pure virtual and MAY have state and concrete methods.
//     An "interface" is the idiom of an abstract class with only pure virtuals, no data,
//     and a virtual destructor. C++ has no `interface` keyword.
//
//  Q  Can an abstract class have a constructor?
//  A  Yes - it runs when a derived object is constructed, to initialise the base's state.
//
//  Q  Diamond problem and its fix?
//  A  Two paths to a common base duplicate the base sub-object and make members
//     ambiguous. Fix: `virtual` inheritance in the intermediate classes -> one shared
//     sub-object, initialised by the most-derived class.
//
//  Q  What is object slicing?
//  A  Copying a Derived into a Base BY VALUE keeps only the base part (and Base's vptr),
//     so polymorphism is lost. Always use references, pointers, or smart pointers.
//
//  Q  What is a vtable / vptr? What is the cost?
//  A  vtable = one per-class array of virtual function pointers; vptr = one hidden pointer
//     per object into it. Cost = +1 pointer per object, one indirection per virtual call,
//     and no inlining of that call.
//
//  Q  Deep copy vs shallow copy?
//  A  Shallow copies the pointer (both objects share and both will free -> double delete).
//     Deep allocates and copies the pointee. The compiler-generated copy is shallow for
//     raw pointers - hence the Rule of Three/Five (or just use smart pointers: Rule of Zero).
//
//  Q  static vs const member function?
//  A  static: no `this`, shared, callable as Class::f(), only touches static members.
//     const : has `this` as a pointer-to-const, promises not to mutate observable state.
//
//  Q  Why does a copy constructor take a reference?
//  A  Taking it by value would need a copy to make the copy -> infinite recursion.
//     Signature: `Foo(const Foo&)`.
//
//  Q  When is a copy elided / when does move happen?
//  A  Returning a local by value is elided (RVO/NRVO, mandatory in C++17 for temporaries).
//     Otherwise, binding to an rvalue (temporary or std::move'd value) selects the move.
//
//  Q  Composition vs Inheritance - which and why?
//  A  Prefer composition: runtime swappable, no fragile base class, no protected surface
//     leak, easier to test, combines freely. Use inheritance only for a genuine IS-A that
//     honours LSP.
//
//  Q  Difference between Strategy and State?
//  A  Same structure; different intent. Strategy = client chooses an interchangeable
//     algorithm. State = the object's behaviour changes as it transitions, and the states
//     drive the transitions themselves.
//
//  Q  Factory Method vs Abstract Factory?
//  A  Factory Method = ONE product, subclass decides the concrete class (inheritance).
//     Abstract Factory = a FAMILY of related products created consistently (composition).
//
//  Q  Decorator vs Inheritance for adding behaviour?
//  A  Decorator adds behaviour per-object, at runtime, and stacks in any order.
//     Inheritance fixes it at compile time and explodes combinatorially (MilkCaramelCoffee).
//
//  Q  Why is Singleton considered an anti-pattern?
//  A  Global mutable state, hidden dependencies, hard to test/mock, awkward lifetime and
//     initialisation order, and it can hide an SRP violation. Prefer DI with a single
//     instance created at the composition root.
//
//  Q  How do you make a Singleton thread-safe in C++?
//  A  Meyers singleton: a function-local `static` is initialised exactly once, thread-safely,
//     since C++11. Otherwise std::call_once or double-checked locking with std::atomic.
//
//  Q  What is dependency injection?
//  A  Passing collaborators in (constructor / setter / method) instead of constructing them
//     inside. It is the technique that implements the Dependency Inversion Principle.
//
//  Q  RAII in one line?
//  A  A resource's lifetime equals an object's lifetime: acquire in the ctor, release in
//     the dtor, so every exit path (including exceptions) cleans up.
//
//  Q  unique_ptr vs shared_ptr vs weak_ptr?
//  A  unique = sole ownership, move-only, free. shared = shared ownership, atomic refcount.
//     weak = non-owning observer that breaks reference cycles; call .lock() to use it.
//
//  Q  Why prefer make_shared over shared_ptr<T>(new T)?
//  A  One allocation instead of two, and it is exception-safe.
//
//  Q  What breaks LSP most often in practice?
//  A  Throwing "unsupported" from an override, strengthening a precondition, changing an
//     invariant (Square/Rectangle), or returning a narrower type of result than promised.
//
//  Q  Cohesion vs Coupling?
//  A  Cohesion = how related one module's responsibilities are (want HIGH).
//     Coupling = how dependent modules are on each other (want LOW).
//
//  Q  How do you decide a class boundary in an LLD round?
//  A  Nouns from the requirements become classes; each class should have one reason to
//     change and own its own data (Information Expert). Anything that varies becomes an
//     interface behind a strategy/factory.
//
//  Q  Multiple inheritance - is it bad?
//  A  Not inherently. Multiple inheritance of INTERFACES (pure abstract, no state) is
//     clean and common. Multiple inheritance of STATE causes the diamond and ambiguity.
//
//  Q  What cannot be virtual?
//  A  Constructors, static member functions, friend/free functions, and template
//     instantiation cannot combine as virtual member function templates.
//
//  Q  What is the fragile base class problem?
//  A  A base class change silently breaks derived classes, because inheritance exposes
//     implementation detail. Another argument for composition.
// ===================================================================================

int main() {
    part00::demo();
    part01::demo();
    part02::demo();
    part03::demo();
    part04::demo();
    part05::demo();
    part06::demo();
    part07::demo();
    part08::demo();
    part09::demo();
    part10::demo();
    part11::demo();
    part12::demo();
    part15::demo();
    part16::demo();
    part17::demo();
    part18::demo();

    banner("DONE");
    cout << "Sections 13 (principles), 14 (UML) and 19 (Q&A) are comment-only revision notes.\n";
    return 0;
}
