#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <cmath>
#include <limits>
#include <atomic>

using namespace std;

enum VehicleType {
    CAR,
    BIKE,
    AUTO
};

enum RideStatus {
    REQUESTED,
    ACCEPTED,
    IN_PROGRESS,
    COMPLETED,
    CANCELLED
};

class Observer {
public:
    virtual void update(string message) = 0;

    virtual ~Observer() {}
};

class Subject {
    vector<Observer*> observers;

public:
    void attach(Observer* observer) {
        if (observer == NULL) {
            return;
        }

        observers.push_back(observer);
    }

    void detach(Observer* observer) {
        for (int i = 0; i < observers.size(); i++) {

            if (observers[i] == observer) {
                observers.erase(observers.begin() + i);
                return;
            }
        }
    }

    void notifyAll(string message) {
        for (Observer* observer : observers) {
            observer->update(message);
        }
    }

    virtual ~Subject() {}
};

class Location {
    int x;
    int y;

public:
    Location(int x, int y) {
        this->x = x;
        this->y = y;
    }

    int getX() {
        return x;
    }

    int getY() {
        return y;
    }
};

class DistanceStrategy {
public:
    virtual double calculateDistance(
        Location* start,
        Location* end
    ) = 0;

    virtual ~DistanceStrategy() {}
};

class EuclideanDistanceStrategy : public DistanceStrategy {
public:
    double calculateDistance(
        Location* start,
        Location* end
    ) override {
        double dx = start->getX() - end->getX();
        double dy = start->getY() - end->getY();

        return sqrt(dx * dx + dy * dy);
    }
};

class ManhattanDistanceStrategy : public DistanceStrategy {
public:
    double calculateDistance(
        Location* start,
        Location* end
    ) override {
        return abs(start->getX() - end->getX()) +
               abs(start->getY() - end->getY());
    }
};

class DistanceCalculator {
    DistanceStrategy* strategy;

    DistanceCalculator() {
        strategy = new EuclideanDistanceStrategy();
    }

    DistanceCalculator(DistanceCalculator& other) = delete;

    void operator=(DistanceCalculator& other) = delete;

public:
    ~DistanceCalculator() {
        delete strategy;
    }

    static DistanceCalculator* getInstance() {
        static DistanceCalculator calculator;

        return &calculator;
    }

    void setStrategy(DistanceStrategy* strategy) {
        delete this->strategy;
        this->strategy = strategy;
    }

    double getDistance(
        Location* start,
        Location* end
    ) {
        return strategy->calculateDistance(start, end);
    }
};

class Vehicle {
    string vehicleNumber;
    VehicleType type;

public:
    Vehicle(
        string vehicleNumber,
        VehicleType type
    ) {
        this->vehicleNumber = vehicleNumber;
        this->type = type;
    }

    string getVehicleNumber() {
        return vehicleNumber;
    }

    VehicleType getType() {
        return type;
    }
};

class Rating {
    int value;
    string comment;

public:
    Rating(
        int value,
        string comment
    ) {
        this->value = value;
        this->comment = comment;
    }

    int getValue() {
        return value;
    }

    string getComment() {
        return comment;
    }
};

class User {
protected:
    string userId;
    string userName;

public:
    User(
        string userId,
        string userName
    ) {
        this->userId = userId;
        this->userName = userName;
    }

    virtual ~User() {}

    string getUserId() {
        return userId;
    }

    string getUserName() {
        return userName;
    }
};

class Driver : public User, public Observer {
    Location* location;
    Vehicle* vehicle;
    bool available;
    vector<Rating*> ratings;

public:
    Driver(
        string userId,
        string userName,
        Location* location,
        Vehicle* vehicle
    ) : User(userId, userName) {

        this->location = location;
        this->vehicle = vehicle;
        this->available = true;
    }

    ~Driver() {
        delete location;
        delete vehicle;

        for (Rating* rating : ratings) {
            delete rating;
        }
    }

    Location* getLocation() {
        return location;
    }

    Vehicle* getVehicle() {
        return vehicle;
    }

    bool isAvailable() {
        return available;
    }

    void setAvailable(bool available) {
        this->available = available;
    }

    void updateLocation(Location* location) {
        delete this->location;
        this->location = location;
    }

    void addRating(Rating* rating) {
        ratings.push_back(rating);
    }

    double getAverageRating() {
        if (ratings.empty()) {
            return 0;
        }

        double total = 0;

        for (Rating* rating : ratings) {
            total = total + rating->getValue();
        }

        return total / ratings.size();
    }

    void update(string message) override {
        cout << "Notification to " << userName << ": " << message << endl;
    }
};

class Rider : public User, public Observer {
    bool riderPass;

public:
    Rider(
        string userId,
        string userName,
        bool riderPass
    ) : User(userId, userName) {

        this->riderPass = riderPass;
    }

    bool hasRiderPass() {
        return riderPass;
    }

    void rateDriver(
        Driver* driver,
        int value,
        string comment = ""
    ) {
        if (value < 1 || value > 5) {
            cout << "Invalid rating" << endl;
            return;
        }

        driver->addRating(new Rating(value, comment));

        cout << userName << " rated " << driver->getUserName()
             << " with " << value << endl;
    }

    void update(string message) override {
        cout << "Notification to " << userName << ": " << message << endl;
    }
};

class DriverFindStrategy {
public:
    virtual Driver* findDriver(
        Location* pickup,
        VehicleType vehicleType,
        vector<Driver*> drivers
    ) = 0;

    virtual ~DriverFindStrategy() {}
};

class NearestDriverStrategy : public DriverFindStrategy {
public:
    Driver* findDriver(
        Location* pickup,
        VehicleType vehicleType,
        vector<Driver*> drivers
    ) override {
        Driver* result = NULL;
        double minDistance = numeric_limits<double>::max();

        for (Driver* driver : drivers) {

            if (!driver->isAvailable()) {
                continue;
            }

            if (driver->getVehicle()->getType() != vehicleType) {
                continue;
            }

            double distance =
                DistanceCalculator::getInstance()->getDistance(
                    driver->getLocation(),
                    pickup
                );

            if (distance < minDistance) {
                minDistance = distance;
                result = driver;
            }
        }

        return result;
    }
};

class FareStrategy {
public:
    virtual double calculateFare(double distance) = 0;

    virtual ~FareStrategy() {}
};

class BaseFareStrategy : public FareStrategy {
public:
    double calculateFare(double distance) override {
        return 50 + distance * 10;
    }
};

class PremiumFareStrategy : public FareStrategy {
public:
    double calculateFare(double distance) override {
        return 100 + distance * 20;
    }
};

class FareDecorator : public FareStrategy {
protected:
    FareStrategy* fareStrategy;

public:
    FareDecorator(FareStrategy* fareStrategy) {
        this->fareStrategy = fareStrategy;
    }

    ~FareDecorator() {
        delete fareStrategy;
    }
};

class SurgeFare : public FareDecorator {
    double multiplier;

public:
    SurgeFare(
        FareStrategy* fareStrategy,
        double multiplier
    ) : FareDecorator(fareStrategy) {

        this->multiplier = multiplier;
    }

    double calculateFare(double distance) override {
        return fareStrategy->calculateFare(distance) * multiplier;
    }
};

class RiderPassDiscount : public FareDecorator {
    double percent;

public:
    RiderPassDiscount(
        FareStrategy* fareStrategy,
        double percent
    ) : FareDecorator(fareStrategy) {

        this->percent = percent;
    }

    double calculateFare(double distance) override {
        double fare = fareStrategy->calculateFare(distance);

        return fare - (fare * percent / 100);
    }
};

class CouponDiscount : public FareDecorator {
    double amount;

public:
    CouponDiscount(
        FareStrategy* fareStrategy,
        double amount
    ) : FareDecorator(fareStrategy) {

        this->amount = amount;
    }

    double calculateFare(double distance) override {
        double fare = fareStrategy->calculateFare(distance);

        if (fare < amount) {
            return 0;
        }

        return fare - amount;
    }
};

class Ride : public Subject {
    string rideId;
    Rider* rider;
    Driver* driver;
    Location* pickup;
    Location* drop;
    RideStatus status;
    FareStrategy* fareStrategy;
    double fare;

    Ride(
        Rider* rider,
        Driver* driver,
        Location* pickup,
        Location* drop,
        FareStrategy* fareStrategy
    ) {
        static atomic<int> counter(1);

        this->rideId = "R" + to_string(counter++);
        this->rider = rider;
        this->driver = driver;
        this->pickup = pickup;
        this->drop = drop;
        this->status = REQUESTED;
        this->fareStrategy = fareStrategy;
        this->fare = 0;

        attach(rider);
        attach(driver);
    }

    bool canMoveTo(RideStatus next) {
        if (status == REQUESTED) {
            return next == ACCEPTED || next == CANCELLED;
        }

        if (status == ACCEPTED) {
            return next == IN_PROGRESS || next == CANCELLED;
        }

        if (status == IN_PROGRESS) {
            return next == COMPLETED;
        }

        return false;
    }

    bool moveTo(
        RideStatus next,
        string message
    ) {
        if (!canMoveTo(next)) {
            return false;
        }

        status = next;

        notifyAll(message);

        return true;
    }

public:

    class Builder {
        Rider* rider;
        Driver* driver;
        Location* pickup;
        Location* drop;
        FareStrategy* fareStrategy;

    public:
        Builder() {
            rider = NULL;
            driver = NULL;
            pickup = NULL;
            drop = NULL;
            fareStrategy = new BaseFareStrategy();
        }

        ~Builder() {
            delete fareStrategy;
        }

        Builder& withRider(Rider* rider) {
            this->rider = rider;
            return *this;
        }

        Builder& withDriver(Driver* driver) {
            this->driver = driver;
            return *this;
        }

        Builder& withPickup(Location* pickup) {
            this->pickup = pickup;
            return *this;
        }

        Builder& withDrop(Location* drop) {
            this->drop = drop;
            return *this;
        }

        Builder& withFareStrategy(FareStrategy* fareStrategy) {
            delete this->fareStrategy;
            this->fareStrategy = fareStrategy;
            return *this;
        }

        Ride* build() {
            FareStrategy* strategy = fareStrategy;

            fareStrategy = NULL;

            return new Ride(rider, driver, pickup, drop, strategy);
        }
    };

    ~Ride() {
        delete pickup;
        delete drop;
        delete fareStrategy;
    }

    string getRideId() {
        return rideId;
    }

    Rider* getRider() {
        return rider;
    }

    Driver* getDriver() {
        return driver;
    }

    Location* getPickupLocation() {
        return pickup;
    }

    Location* getDropLocation() {
        return drop;
    }

    RideStatus getStatus() {
        return status;
    }

    double getFare() {
        return fare;
    }

    void setDriver(Driver* driver) {
        this->driver = driver;
    }

    double calculateFare() {
        double distance =
            DistanceCalculator::getInstance()->getDistance(pickup, drop);

        return fareStrategy->calculateFare(distance);
    }

    bool accept() {
        return moveTo(
            ACCEPTED,
            "Ride " + rideId + " accepted by " + driver->getUserName()
        );
    }

    bool start() {
        return moveTo(IN_PROGRESS, "Ride " + rideId + " started");
    }

    bool complete() {
        if (!canMoveTo(COMPLETED)) {
            return false;
        }

        fare = calculateFare();

        return moveTo(COMPLETED, "Ride " + rideId + " completed");
    }

    bool cancel(string message) {
        return moveTo(CANCELLED, message);
    }
};

class UserFactory {
public:
    static Rider* createRider(
        string userId,
        string name,
        bool riderPass
    ) {
        return new Rider(userId, name, riderPass);
    }

    static Driver* createDriver(
        string userId,
        string name,
        Location* location,
        Vehicle* vehicle
    ) {
        return new Driver(userId, name, location, vehicle);
    }
};

class RideManager {
    vector<Driver*> drivers;
    unordered_map<string, Ride*> rides;
    DriverFindStrategy* driverFindStrategy;
    mutex rideMutex;

    RideManager() {
        driverFindStrategy = new NearestDriverStrategy();
    }

    RideManager(RideManager& other) = delete;

    void operator=(RideManager& other) = delete;

    Ride* getRideById(string rideId) {
        if (rides.find(rideId) == rides.end()) {
            return NULL;
        }

        return rides[rideId];
    }

public:
    ~RideManager() {
        for (auto entry : rides) {
            delete entry.second;
        }

        rides.clear();

        delete driverFindStrategy;
    }

    static RideManager* getInstance() {
        static RideManager manager;

        return &manager;
    }

    void setDriverFindStrategy(DriverFindStrategy* strategy) {
        lock_guard<mutex> lock(rideMutex);

        delete driverFindStrategy;
        driverFindStrategy = strategy;
    }

    void addDriver(Driver* driver) {
        lock_guard<mutex> lock(rideMutex);

        drivers.push_back(driver);
    }

    Ride* bookRide(
        Rider* rider,
        Location* pickup,
        Location* drop,
        VehicleType vehicleType,
        FareStrategy* fareStrategy
    ) {
        lock_guard<mutex> lock(rideMutex);

        Driver* driver =
            driverFindStrategy->findDriver(pickup, vehicleType, drivers);

        if (driver == NULL) {
            delete pickup;
            delete drop;
            delete fareStrategy;

            return NULL;
        }

        if (rider->hasRiderPass()) {
            fareStrategy = new RiderPassDiscount(fareStrategy, 10);
        }

        driver->setAvailable(false);

        Ride::Builder builder;

        Ride* ride =
            builder
                .withRider(rider)
                .withDriver(driver)
                .withPickup(pickup)
                .withDrop(drop)
                .withFareStrategy(fareStrategy)
                .build();

        rides[ride->getRideId()] = ride;

        ride->notifyAll("Ride " + ride->getRideId() + " requested");

        return ride;
    }

    bool acceptRide(
        string rideId,
        Driver* driver
    ) {
        lock_guard<mutex> lock(rideMutex);

        Ride* ride = getRideById(rideId);

        if (ride == NULL || ride->getDriver() != driver) {
            return false;
        }

        return ride->accept();
    }

    bool startRide(
        string rideId,
        Driver* driver
    ) {
        lock_guard<mutex> lock(rideMutex);

        Ride* ride = getRideById(rideId);

        if (ride == NULL || ride->getDriver() != driver) {
            return false;
        }

        return ride->start();
    }

    bool completeRide(
        string rideId,
        Driver* driver
    ) {
        lock_guard<mutex> lock(rideMutex);

        Ride* ride = getRideById(rideId);

        if (ride == NULL || ride->getDriver() != driver) {
            return false;
        }

        bool completed = ride->complete();

        if (completed) {
            driver->setAvailable(true);

            cout << ride->getRideId() << " completed. Fare: "
                 << ride->getFare() << endl;
        }

        return completed;
    }

    bool cancelRide(
        string rideId,
        Rider* rider
    ) {
        lock_guard<mutex> lock(rideMutex);

        Ride* ride = getRideById(rideId);

        if (ride == NULL || ride->getRider() != rider) {
            return false;
        }

        bool cancelled =
            ride->cancel("Ride " + rideId + " cancelled by rider");

        if (cancelled && ride->getDriver() != NULL) {
            ride->getDriver()->setAvailable(true);
        }

        return cancelled;
    }

    bool cancelRide(
        string rideId,
        Driver* driver
    ) {
        lock_guard<mutex> lock(rideMutex);

        Ride* ride = getRideById(rideId);

        if (ride == NULL || ride->getDriver() != driver) {
            return false;
        }

        bool cancelled =
            ride->cancel("Ride " + rideId + " cancelled by driver");

        if (cancelled) {
            driver->setAvailable(true);
        }

        return cancelled;
    }

    Ride* getRide(string rideId) {
        lock_guard<mutex> lock(rideMutex);

        return getRideById(rideId);
    }
};

void runConcurrentBooking(
    RideManager* rideManager,
    Rider* rider,
    string name,
    int pickupX,
    int pickupY
) {
    Ride* ride =
        rideManager->bookRide(
            rider,
            new Location(pickupX, pickupY),
            new Location(20, 20),
            CAR,
            new BaseFareStrategy()
        );

    if (ride == NULL) {
        cout << name << " could not get a driver" << endl;
        return;
    }

    cout << name << " got " << ride->getDriver()->getUserName()
         << " for " << ride->getRideId() << endl;
}

int main() {

    RideManager* rideManager = RideManager::getInstance();

    Rider* rider1 = UserFactory::createRider("r1", "Rahul", false);

    Rider* rider2 = UserFactory::createRider("r2", "Aman", true);

    Driver* driver1 =
        UserFactory::createDriver(
            "d1",
            "Driver1",
            new Location(2, 2),
            new Vehicle("MH01AB1111", CAR)
        );

    Driver* driver2 =
        UserFactory::createDriver(
            "d2",
            "Driver2",
            new Location(15, 15),
            new Vehicle("MH01AB2222", CAR)
        );

    rideManager->addDriver(driver1);

    rideManager->addDriver(driver2);

    Ride* ride =
        rideManager->bookRide(
            rider1,
            new Location(3, 3),
            new Location(10, 10),
            CAR,
            new BaseFareStrategy()
        );

    if (ride == NULL) {

        cout << "No driver available" << endl;

        delete rider1;
        delete rider2;
        delete driver1;
        delete driver2;

        return 0;
    }

    cout << "Ride booked: " << ride->getRideId() << endl;

    Driver* assignedDriver = ride->getDriver();

    rideManager->acceptRide(ride->getRideId(), assignedDriver);

    rideManager->startRide(ride->getRideId(), assignedDriver);

    rideManager->completeRide(ride->getRideId(), assignedDriver);

    rider1->rateDriver(assignedDriver, 5, "Smooth ride");

    thread t1(
        runConcurrentBooking,
        rideManager,
        rider1,
        "Concurrent Rider 1",
        0,
        0
    );

    thread t2(
        runConcurrentBooking,
        rideManager,
        rider2,
        "Concurrent Rider 2",
        1,
        1
    );

    t1.join();
    t2.join();

    Ride* extraRide =
        rideManager->bookRide(
            rider1,
            new Location(5, 5),
            new Location(25, 25),
            CAR,
            new PremiumFareStrategy()
        );

    if (extraRide == NULL) {
        cout << "No driver available for extra ride" << endl;
    } else {
        cout << "Extra ride assigned to "
             << extraRide->getDriver()->getUserName() << endl;
    }

    delete rider1;
    delete rider2;
    delete driver1;
    delete driver2;

    return 0;
}
