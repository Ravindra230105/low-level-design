// Parking Lot
// multiple entry or exit gates
// each parking lot has multiple floors
// each floor has multiple parking spots
// each parking spot has a type (LARGE, SMALL, MEDIUM)
// each parking sopt has a different price
// ticket generation at the time of entry.
// park vehicle at the time of entry
// unpark vehicle at the time of exit
// payment at the time of exit through ticket. 
// multiple ways of payments
// ticket pricing : time based or event based
// concurrency handle


// Classes / Entities
   // ParkingLot
   // Gate : EntryGate, ExitGate (Inherited) (abstract class)
   // Floor
   // ParkingSpot : LargeParkingSpot, MediumParkingSpot, SmallParkingSpot (Inherited)
   // Vehicle
   // Ticket
   // PricingStrategy : TimeBasedPricing, EventBasedPricing
   // Payment : CashPayment, UPIPayment, CreditCardPayment


// Relationships
  // ParkingLot -> Gates (1 : M)
  // ParkingLot -> Floors (1 : M)
  // Floor -> ParkingSpots (1: M)
  // ParkingSpot -> Vehicle (1: 1)
  // Vehicle -> Ticket (1: 1)
  // Ticket -> Payments (1: M)
  // ParkingLot -> PricingStrategy (1: 1)


// Flow 
   // Initialize ParkingLot
   // Add floor
   // Add parkingSpot to each Floor
   // Set PricingStrategy (time based or event based)
   // Enter Vehicle
   // Park Vehicle -> reserve spot -> Ticket Generation 
   // Unpark Vehicle -> set payment strategy -> Payment -> release spot


// Design Patterns
   // Singleton pattern for Parking lot single instance
   // Factory pattern for parkingSpot
   // Factory Pattern for payment strategy
   // Strategy Pattern for Payment
   // Strategy Pattern for Pricing


// bottom - up approach for implementation. 

#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<algorithm>
#include<mutex>
#include<ctime>
#include<cmath>
#include<sstream>

using namespace std;


enum class VehicleType { BIKE, CAR, TRUCK };

enum class SpotType { SMALL, MEDIUM, LARGE };

enum class PaymentType { CASH, UPI, CREDIT_CARD };


string spotTypeName(SpotType type) {
    switch (type) {
        case SpotType::SMALL:  return "SMALL";
        case SpotType::MEDIUM: return "MEDIUM";
        case SpotType::LARGE:  return "LARGE";
        default:               return "UNKNOWN";
    }
}

SpotType getRequiredSpotType(VehicleType type) {
    switch (type) {
        case VehicleType::BIKE:  return SpotType::SMALL;
        case VehicleType::CAR:   return SpotType::MEDIUM;
        case VehicleType::TRUCK: return SpotType::LARGE;
        default:                 return SpotType::MEDIUM;
    }
}


class Vehicle {
    private:
        string number;
        VehicleType type;

    public:
        Vehicle(string number, VehicleType type) : number(number), type(type) {}

        string getNumber() { return number; }

        VehicleType getType() { return type; }
};

class ParkingSpot {
    protected:
        int spotId;
        SpotType type;
        double pricePerHour;
        bool occupied;
        Vehicle* vehicle;

    public:
        ParkingSpot(int spotId, SpotType type, double pricePerHour)
            : spotId(spotId), type(type), pricePerHour(pricePerHour),
              occupied(false), vehicle(nullptr) {}

        bool isFree() { return !occupied; }

        void assignVehicle(Vehicle* v) {
            vehicle = v;
            occupied = true;
        }

        void removeVehicle() {
            vehicle = nullptr;
            occupied = false;
        }

        int getSpotId() { return spotId; }

        SpotType getType() { return type; }

        double getPricePerHour() { return pricePerHour; }

        Vehicle* getVehicle() { return vehicle; }

        virtual ~ParkingSpot() {};
};

class SmallParkingSpot : public ParkingSpot {
    public:
        SmallParkingSpot(int spotId) : ParkingSpot(spotId, SpotType::SMALL, 10.0) {}
};

class MediumParkingSpot : public ParkingSpot {
    public:
        MediumParkingSpot(int spotId) : ParkingSpot(spotId, SpotType::MEDIUM, 20.0) {}
};

class LargeParkingSpot : public ParkingSpot {
    public:
        LargeParkingSpot(int spotId) : ParkingSpot(spotId, SpotType::LARGE, 30.0) {}
};

class ParkingSpotFactory {
    public:
        static ParkingSpot* createSpot(int spotId, SpotType type) {
            switch (type) {
                case SpotType::SMALL:
                    return new SmallParkingSpot(spotId);

                case SpotType::MEDIUM:
                    return new MediumParkingSpot(spotId);

                case SpotType::LARGE:
                    return new LargeParkingSpot(spotId);

                default:
                    return nullptr;
            }
        }
};


class Ticket {
    private:
        string ticketId;
        Vehicle* vehicle;
        ParkingSpot* spot;
        int floorId;
        time_t entryTime;
        time_t exitTime;
        bool paid;

    public:
        Ticket(string ticketId, Vehicle* vehicle, ParkingSpot* spot, int floorId)
            : ticketId(ticketId), vehicle(vehicle), spot(spot), floorId(floorId),
              entryTime(time(nullptr)), exitTime(0), paid(false) {}

        void markExit() { exitTime = time(nullptr); }

        void markPaid() { paid = true; }

        bool isPaid() { return paid; }

        double getDurationInHours() {
            time_t end = (exitTime == 0) ? time(nullptr) : exitTime;
            double hours = difftime(end, entryTime) / 3600.0;
            return (hours < 1.0) ? 1.0 : ceil(hours);
        }

        string getTicketId() { return ticketId; }

        Vehicle* getVehicle() { return vehicle; }

        ParkingSpot* getSpot() { return spot; }

        int getFloorId() { return floorId; }

        void setEntryTime(time_t t) { entryTime = t; }

        void print() {
            cout << "  Ticket   : " << ticketId << endl;
            cout << "  Vehicle  : " << vehicle->getNumber() << endl;
            cout << "  Floor    : " << floorId << endl;
            cout << "  Spot     : " << spot->getSpotId()
                 << " (" << spotTypeName(spot->getType()) << ")" << endl;
        }
};


class PricingStrategy {
    public:
        virtual double calculatePrice(Ticket* ticket) = 0;
        virtual string getName() = 0;
        virtual ~PricingStrategy() {};
};

class TimeBasedPricing : public PricingStrategy {
    public:
        double calculatePrice(Ticket* ticket) override {
            double hours = ticket->getDurationInHours();
            double rate  = ticket->getSpot()->getPricePerHour();
            return hours * rate;
        }

        string getName() override { return "Time Based Pricing"; }
};

class EventBasedPricing : public PricingStrategy {
    private:
        string eventName;
        double flatFee;

    public:
        EventBasedPricing(string eventName, double flatFee)
            : eventName(eventName), flatFee(flatFee) {}

        double calculatePrice(Ticket* ticket) override {
            return flatFee;
        }

        string getName() override { return "Event Based Pricing (" + eventName + ")"; }
};


class PaymentStrategy {
    public: 
        virtual bool pay(double amount) = 0;
        virtual ~PaymentStrategy() {};
};

class CashPayment : public PaymentStrategy {
    public:
        bool pay(double amount) override {
            cout << "Processing Cash Payment of Rs. "<< amount << endl;
            cout << "Cash Payment Successful." << endl;

            return true;
        }
};

class UPIPayment : public PaymentStrategy {
    public: 
        bool pay(double amount) override {
            cout << "Processing UPI Payment of Rs. "<< amount << endl;
            cout << "UPI Payment Successful." << endl;

            return true;
        }
};

class CreditCardPayment : public PaymentStrategy {
    public: 
        bool pay(double amount) override {
            cout << "Processing Credit Card Payment of Rs. "<< amount << endl;
            cout << "Credit Card Payment Successful." << endl;

            return true;
        }
};

class PaymentFactory {
    public: 
        static PaymentStrategy* getPaymentStrategy(PaymentType type) {
            switch (type) {
                case PaymentType::CASH :
                    return new CashPayment();
                
                case PaymentType::UPI :
                    return new UPIPayment();

                case PaymentType::CREDIT_CARD : 
                    return new CreditCardPayment();
                
                default:
                    return nullptr;
            }
        }
};

class PaymentProcessor {
    public:
        bool processPayment(PaymentStrategy* strategy, double amount) {
            if (strategy == nullptr) {
                cout << "Please choose valid payment method" << endl;
                return false;
            }

            return strategy->pay(amount);
        }
};


class Floor {
    private:
        int floorId;
        int nextSpotId;
        vector<ParkingSpot*> spots;
        mutex floorMutex;

    public:
        Floor(int floorId) : floorId(floorId), nextSpotId(1) {}

        void addSpot(SpotType type) {
            lock_guard<mutex> lock(floorMutex);
            int spotId = floorId * 100 + nextSpotId;
            nextSpotId++;
            spots.push_back(ParkingSpotFactory::createSpot(spotId, type));
        }

        ParkingSpot* reserveSpot(SpotType type, Vehicle* vehicle) {
            lock_guard<mutex> lock(floorMutex);
            for (ParkingSpot* spot : spots) {
                if (spot->getType() == type && spot->isFree()) {
                    spot->assignVehicle(vehicle);
                    return spot;
                }
            }
            return nullptr;
        }

        void releaseSpot(ParkingSpot* spot) {
            lock_guard<mutex> lock(floorMutex);
            if (spot != nullptr && !spot->isFree()) {
                spot->removeVehicle();
            }
        }

        int getFreeCount() {
            lock_guard<mutex> lock(floorMutex);
            int count = 0;
            for (ParkingSpot* spot : spots) {
                if (spot->isFree()) count++;
            }
            return count;
        }

        int getTotalCount() {
            lock_guard<mutex> lock(floorMutex);
            return (int) spots.size();
        }

        int getFloorId() { return floorId; }

        ~Floor() {
            for (ParkingSpot* spot : spots) delete spot;
        }
};


class Gate {
    protected:
        int gateId;
        string gateName;

    public:
        Gate(int gateId, string gateName) : gateId(gateId), gateName(gateName) {}

        virtual string getGateType() = 0;

        int getGateId() { return gateId; }

        string getGateName() { return gateName; }

        virtual ~Gate() {};
};

class EntryGate : public Gate {
    public:
        EntryGate(int gateId, string gateName) : Gate(gateId, gateName) {}

        string getGateType() override { return "ENTRY"; }
};

class ExitGate : public Gate {
    public:
        ExitGate(int gateId, string gateName) : Gate(gateId, gateName) {}

        string getGateType() override { return "EXIT"; }
};


class ParkingLot {
    private:
        vector<Floor*> floors;
        vector<Gate*> gates;
        unordered_map<string, Ticket*> activeTickets;
        vector<Ticket*> allTickets;
        PricingStrategy* pricingStrategy;
        int ticketCounter;
        mutex lotMutex;

        ParkingLot() : pricingStrategy(nullptr), ticketCounter(0) {}

        ParkingLot(const ParkingLot&) = delete;
        ParkingLot& operator=(const ParkingLot&) = delete;

        string generateTicketId() {
            ticketCounter++;
            stringstream ss;
            ss << "TKT-" << ticketCounter;
            return ss.str();
        }

        Floor* getFloorById(int floorId) {
            for (Floor* floor : floors) {
                if (floor->getFloorId() == floorId) return floor;
            }
            return nullptr;
        }

        Ticket* parkVehicle(Vehicle* vehicle) {
            SpotType required = getRequiredSpotType(vehicle->getType());

            for (Floor* floor : floors) {
                ParkingSpot* spot = floor->reserveSpot(required, vehicle);
                if (spot == nullptr) continue;

                lock_guard<mutex> lock(lotMutex);
                Ticket* ticket = new Ticket(generateTicketId(), vehicle, spot,
                                            floor->getFloorId());
                activeTickets[ticket->getTicketId()] = ticket;
                allTickets.push_back(ticket);
                return ticket;
            }

            return nullptr;
        }

        bool unparkVehicle(Ticket* ticket, PaymentType paymentType) {
            if (ticket == nullptr) return false;

            {
                lock_guard<mutex> lock(lotMutex);
                if (activeTickets.find(ticket->getTicketId()) == activeTickets.end()) {
                    cout << "Invalid or already used ticket." << endl;
                    return false;
                }
            }

            if (pricingStrategy == nullptr) {
                cout << "No pricing strategy configured." << endl;
                return false;
            }

            ticket->markExit();
            double amount = pricingStrategy->calculatePrice(ticket);

            cout << "  Duration : " << ticket->getDurationInHours() << " hour(s)" << endl;
            cout << "  Amount   : Rs. " << amount << endl;

            PaymentStrategy* strategy = PaymentFactory::getPaymentStrategy(paymentType);
            PaymentProcessor processor;
            bool success = processor.processPayment(strategy, amount);
            delete strategy;

            if (!success) {
                cout << "Payment failed. Barrier stays down." << endl;
                return false;
            }

            ticket->markPaid();

            Floor* floor = getFloorById(ticket->getFloorId());
            if (floor != nullptr) floor->releaseSpot(ticket->getSpot());

            lock_guard<mutex> lock(lotMutex);
            activeTickets.erase(ticket->getTicketId());
            return true;
        }

    public:
        static ParkingLot* getInstance() {
            static ParkingLot instance;
            return &instance;
        }

        void addFloor(Floor* floor) { floors.push_back(floor); }

        void addGate(Gate* gate) { gates.push_back(gate); }

        void setPricingStrategy(PricingStrategy* strategy) {
            pricingStrategy = strategy;
            cout << ">> Pricing set to : " << strategy->getName() << endl;
        }

        Ticket* processEntry(EntryGate* gate, Vehicle* vehicle) {
            cout << "\n[" << gate->getGateName() << " / " << gate->getGateType()
                 << "] Vehicle " << vehicle->getNumber() << " arrived." << endl;

            Ticket* ticket = parkVehicle(vehicle);

            if (ticket == nullptr) {
                cout << "  No spot available. Barrier stays down." << endl;
                return nullptr;
            }

            cout << "  Ticket generated." << endl;
            ticket->print();
            return ticket;
        }

        bool processExit(ExitGate* gate, Ticket* ticket, PaymentType paymentType) {
            if (ticket == nullptr) {
                cout << "\n[" << gate->getGateName() << " / " << gate->getGateType()
                     << "] No ticket presented." << endl;
                return false;
            }

            cout << "\n[" << gate->getGateName() << " / " << gate->getGateType()
                 << "] Ticket " << ticket->getTicketId() << " presented." << endl;

            bool done = unparkVehicle(ticket, paymentType);

            if (done) {
                cout << "  Spot released. Barrier open. Drive safe!" << endl;
            }
            return done;
        }

        void displayStatus() {
            cout << "\n---------- PARKING LOT STATUS ----------" << endl;
            int totalFree = 0;
            int total = 0;
            for (Floor* floor : floors) {
                int free = floor->getFreeCount();
                int all  = floor->getTotalCount();
                totalFree += free;
                total     += all;
                cout << "  Floor " << floor->getFloorId()
                     << " : vacant " << free << " / " << all
                     << " | filled " << (all - free)
                     << (free == 0 ? "   [FULL]" : "") << endl;
            }
            cout << "  TOTAL   : vacant " << totalFree << " / " << total << endl;
            cout << "----------------------------------------\n" << endl;
        }

        ~ParkingLot() {
            for (Floor* floor : floors) delete floor;
            for (Ticket* ticket : allTickets) delete ticket;
        }
};


int main() {
    ParkingLot* lot = ParkingLot::getInstance();

    Floor* floor1 = new Floor(1);
    floor1->addSpot(SpotType::SMALL);
    floor1->addSpot(SpotType::MEDIUM);
    floor1->addSpot(SpotType::MEDIUM);
    floor1->addSpot(SpotType::LARGE);

    Floor* floor2 = new Floor(2);
    floor2->addSpot(SpotType::SMALL);
    floor2->addSpot(SpotType::MEDIUM);

    lot->addFloor(floor1);
    lot->addFloor(floor2);

    EntryGate entryA(1, "Gate A");
    EntryGate entryB(2, "Gate B");
    ExitGate  exitC(3, "Gate C");

    lot->addGate(&entryA);
    lot->addGate(&entryB);
    lot->addGate(&exitC);

    TimeBasedPricing timePricing;
    lot->setPricingStrategy(&timePricing);

    lot->displayStatus();

    Vehicle car1("MH-12-AB-1234", VehicleType::CAR);
    Vehicle bike1("MH-14-XY-5678", VehicleType::BIKE);
    Vehicle truck1("MH-01-TR-9999", VehicleType::TRUCK);

    Ticket* t1 = lot->processEntry(&entryA, &car1);
    Ticket* t2 = lot->processEntry(&entryB, &bike1);
    Ticket* t3 = lot->processEntry(&entryA, &truck1);

    lot->displayStatus();

    if (t1 != nullptr) t1->setEntryTime(time(nullptr) - 3 * 3600);
    lot->processExit(&exitC, t1, PaymentType::UPI);

    lot->displayStatus();

    EventBasedPricing eventPricing("IPL Match Day", 500.0);
    lot->setPricingStrategy(&eventPricing);

    lot->processExit(&exitC, t2, PaymentType::CASH);
    lot->processExit(&exitC, t3, PaymentType::CREDIT_CARD);

    lot->displayStatus();

    lot->processExit(&exitC, t1, PaymentType::CASH);

    Vehicle car2("MH-12-CD-1111", VehicleType::CAR);
    Vehicle car3("MH-12-EF-2222", VehicleType::CAR);
    Vehicle car4("MH-12-GH-3333", VehicleType::CAR);
    Vehicle car5("MH-12-IJ-4444", VehicleType::CAR);
    lot->processEntry(&entryA, &car2);
    lot->processEntry(&entryA, &car3);
    lot->processEntry(&entryB, &car4);
    lot->processEntry(&entryB, &car5);

    lot->displayStatus();

    return 0;
}
