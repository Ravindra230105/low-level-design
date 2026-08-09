// Book my show

// Classes
// Attributes/data-members and function/methods
// Design patterns : we can use to implement this system
// error handling and concurrency and all other cases.



// ---------------------------------------- Classes -------------------------
// User
// Movie
// City
// Theatre
// Screen
// Show
// Seat
// ShowSeat
// SeatLock
// Booking
// Ticket
// Payment
// SearchService
// Notification



// ---------------------------------------- Attributes ---------------------------
// User class
// userId, name, email, phone

// Movie Class
// movieId, title, duration, language, genre, description, rating

// Theatre Class
// theatreId, name, city, adddress, list of screens

// Screen Class
// screenId, name, list of seats(Seat)

// Seat Class
// seatId, row, column, type
// SeatType:  REGULAR, PREMIUM, VIP

// Show Class
// showId, movie, screen, startTime, endTime, list of seats(ShowSeat)

// ShowSeat Class
// showSeatId, seat(Seat), status, price
// SeatStatus: AVAILABLE, LOCKED, BOOKED

// SeatLock Class
// seat(ShowSeat), user, lockTime(for expiry)

// Booking Class
// bookingId, user, show, list of seats(ShowSeat), status, payment
// BookingStatus: CREATED, CONFIRMED, CANCELLED, EXPIRED

// Ticket Class
// ticketId, booking, qrCode(for entry)

// Payment Class
// paymentId, amount, status, mode
// PaymentStatus: PENDING, SUCCESS, FAILED
// PaymentMode: UPI, CARD, NETBANKING

// SearchService Class
// list of movie, list of show

// Notification Class
// notificationId, message, type
// NotificationType: EMAIL, SMS, PUSH




// -------------------------------------- Methods -----------------------

// For BookMyShow: Following will be the user actions
    // Search Movie
    // View Shows
    // Select Seats
    // Lock Seats
    // Create Booking
    // Make Payment
    // Confirm Booking
    // Get Ticket
    // Cancel Booking

// We will divide into: 
    // Services (MAIN LOGIC)
    // Entities (LIGHT METHODS ONLY)


// SearchService
   // vector<Movie*> searchMovies(string movieName);
   // vector<Show*> getShowByMovie(Movie* movie);
   // vector<Theatre*> getTheatresByCity(string city);
   // vector<Show*> getShowsByTheatre(Theatre* theatre);

// SeatLockService
   // bool lockSeats(User* user, vector<ShowSeat*> seats)
   // void unlockSeats(vector<ShowSeat*> seats);
   // bool isSeatAvailable(ShowSeat* seat);
   // void validateLockExpiry();

// BookingService
   // Booking* createBooking(User* user, Show* show, vector<ShowSeat*>seats);
   // bool confirmBooking(Booking* booking)
   // bool cancelBooking(Booking* booking)
   // Booking* getBookingById(string bookingId);

// PaymentService
   // bool processPayment(Payment* payment);
   // PaymentStatus getPaymentStatus(string paymentId);

// TicketService
   // Ticket* generateTicket(Booking* booking);
   // Ticket* getTicketByBooking(Booking* booking);

// NotificationService
   // void sendNotification(User* user, string message);


// ENTITY METHODS (KEEP MINIMAL)

// User
   // string getUserId();

// Movie
   // string getTitle();

// Theatre
   // vector<Screen*> getScreens();

// Screen
   // vector<Seat*> getSeats();

// Show
   // vector<ShowSeat*> getAvailableSeats();

// ShowSeat
   // void setStatus(SeatStatus status);
   // SeatStatus getStatus();

// SeatLock
   // bool isExpired();

// Booking
   // void setStatus(BookingStatus status);
   // BookingStatus getStatus();

// Payment
   // void setStatus(PaymentStatus status);
   // PaymentStatus getStatus();

// Ticket
   // string getQRCode();


// ----------------------------------- SOLID PRINCIPLES AND DESIGN PATTERNS ----------------------------------
// - SRP: Entities (data) and Services (logic) separated
// - OCP: Payment system extensible via strategies (no modification needed for new methods)
// - LSP: All payment strategies interchangeable via common interface
// - ISP: Minimal interfaces (e.g., PaymentStrategy only has pay())
// - DIP: Services depend on abstractions, not concrete implementations


// - Strategy: Used for multiple payment methods (UPI, Card, etc.)
// - Factory: Used to create payment strategy based on input
// - Facade: BookingService simplifies complex booking workflow
// - Observer: For notifications (SMS/Email on booking events)
// - Singleton: For shared services like SeatLockService



//  --------------------------------- CODE IMPLEMENTATION -------------------------------------


#include<iostream>
#include<string>
#include<vector>
#include<algorithm>

using namespace std;


enum SeatType { REGULAR, PREMIUM, VIP };
enum SeatStatus { AVAILABLE, LOCKED, BOOKED };
enum BookingStatus { CREATED, CONFIRMED, CANCELLED, EXPIRED };
enum PaymentStatus { PENDING, SUCCESS, FAILED };
enum PaymentMode { UPI, CARD, NETBANKING };
enum NotificationType { EMAIL, SMS, PUSH };


class User {
   string userId, name, email, phone;
public:
   User(string id, string n, string e, string p)
      : userId(id), name(n), email(e), phone(p) {}

   string getUserId() { 
      return userId; 
   }
};

class Movie {
   string movieId, title, language, genre, description;
   int duration;
   double rating;
public:
   Movie(string id, string t, int d, string lang, string g, string desc, double r)
        : movieId(id), title(t), duration(d), language(lang),
          genre(g), description(desc), rating(r) {}

   string getTitle() { 
      return title; 
   }
};

class Seat {
public:
   string seatId;
   int row, col;
   SeatType type;

   Seat(string id, int r, int c, SeatType t)
      : seatId(id), row(r), col(c), type(t) {}
};

class Screen {
   string screenId, name;
   vector<Seat*> seats;

public:
   Screen(string id, string name, vector<Seat*> seats)
      : screenId(id), name(name), seats(seats) {}

   vector<Seat*> getSeats() { 
      return seats; 
   }
};

class Theatre {
public:
   string theatreId, name, city, address;
   vector<Screen*> screens;

   Theatre(string id, string n, string c, string addr, vector<Screen*> s)
      : theatreId(id), name(n), city(c), address(addr), screens(s) {}

   vector<Screen*> getScreens() { 
      return screens; 
   }
};

class ShowSeat {
   string showSeatId;
   Seat* seat;
   SeatStatus status;
   double price;

public:
   ShowSeat(string id, Seat* s, double p)
      : showSeatId(id), seat(s), price(p), status(AVAILABLE) {}

   void setStatus(SeatStatus s) { 
      status = s; 
   }

   SeatStatus getStatus() { 
      return status; 
   }

   double getPrice() { 
      return price; 
   }
};

class Show {
public:
   string showId;
   Movie* movie;
   Screen* screen;
   time_t startTime, endTime;
   vector<ShowSeat*> seats;

   Show(string id, Movie* m, Screen* s, time_t st, time_t et, vector<ShowSeat*> seats)
      : showId(id), movie(m), screen(s), startTime(st), endTime(et), seats(seats) {}

   vector<ShowSeat*> getAvailableSeats() {
      vector<ShowSeat*> res;
      for (auto s : seats) {
         if (s->getStatus() == AVAILABLE) {
            res.push_back(s);
         }
      }
      return res;
   }
};

class SeatLock {
public:
   ShowSeat* seat;
   User* user;
   time_t lockTime;

   SeatLock(ShowSeat* s, User* u)
      : seat(s), user(u), lockTime(time(0)) {}

   bool isExpired() {
      return (time(0) - lockTime) > 300;
   }
};

class Payment {
public:
   string paymentId;
   double amount;
   PaymentStatus status;
   PaymentMode mode;

   Payment(string id, double amt, PaymentMode m)
      : paymentId(id), amount(amt), status(PENDING), mode(m) {}

   void setStatus(PaymentStatus s) { 
      status = s; 
   }

   PaymentStatus getStatus() { 
      return status; 
   }
};

class Booking {
public:
   string bookingId;
   User* user;
   Show* show;
   vector<ShowSeat*> seats;
   BookingStatus status;
   Payment* payment;

   Booking(string id, User* u, Show* s, vector<ShowSeat*> st)
      : bookingId(id), user(u), show(s), seats(st), status(CREATED), payment(nullptr) {}

   void setStatus(BookingStatus s) { 
      status = s; 
   }

   BookingStatus getStatus() { 
      return status; 
   }
};

class Ticket {
   string ticketId;
   Booking* booking;
   string qrCode;

public:
   Ticket(string id, Booking* b)
      : ticketId(id), booking(b), qrCode("QR_" + id) {}

   string getQRCode() { 
      return qrCode; 
   }
};

class Notification {
public:
   string notificationId, message;
   NotificationType type;

   Notification(string id, string msg, NotificationType t)
      : notificationId(id), message(msg), type(t) {}
};

class PaymentStrategy {
public:
   virtual bool pay(double amount) = 0;
};

class UpiPayment : public PaymentStrategy {
public:
   bool pay(double amount) override {
      cout << "UPI Payment: " << amount << endl;
      return true;
   }
};

class CardPayment : public PaymentStrategy {
public:
   bool pay(double amount) override {
      cout << "Card Payment: " << amount << endl;
      return true;
   }
};

class PaymentFactory {
public:
   static PaymentStrategy* getStrategy(PaymentMode mode) {
      if (mode == UPI) return new UpiPayment();
      if (mode == CARD) return new CardPayment();
      return nullptr;
   }
};

class Observer {
public:
   virtual void update(User* user, string message) = 0;
};
class EmailService : public Observer {
public:
    void update(User* user, string message) override {
        cout << "[EMAIL] to " << user->getUserId() << ": " << message << endl;
    }
};

class SMSService : public Observer {
public:
    void update(User* user, string message) override {
        cout << "[SMS] to " << user->getUserId() << ": " << message << endl;
    }
};

class NotificationManager {
    vector<Observer*> observers;

public:
    void addObserver(Observer* obs) {
        observers.push_back(obs);
    }

    void notifyAll(User* user, string message) {
        for (auto obs : observers) {
            obs->update(user, message);
        }
    }
};
class SearchService {
public:
   vector<Movie*> movies;
   vector<Show*> shows;
   vector<Theatre*> theatres;

   vector<Movie*> searchMovies(string name) {
      vector<Movie*> res;
      for (auto m : movies)
         if (m->getTitle() == name)
            res.push_back(m);
      return res;
   }

   vector<Show*> getShowByMovie(Movie* movie) {
      vector<Show*> res;
      for (auto s : shows)
         if (s->movie == movie)
            res.push_back(s);
      return res;
   }

   vector<Theatre*> getTheatresByCity(string city) {
      vector<Theatre*> res;
      for (auto t : theatres)
         if (t->city == city)
            res.push_back(t);
      return res;
   }

   vector<Show*> getShowsByTheatre(Theatre* theatre) {
      vector<Show*> res;
      for (auto s : shows)
         if (s->screen && theatre)
            res.push_back(s);
      return res;
   }
};

class SeatLockService {
   unordered_map<ShowSeat*, SeatLock*> locks;

public:
   bool isSeatAvailable(ShowSeat* seat) {
      return seat->getStatus() == AVAILABLE;
   }

   bool lockSeats(User* user, vector<ShowSeat*> seats) {
      for (auto s : seats)
         if (!isSeatAvailable(s)) return false;

      for (auto s : seats) {
         s->setStatus(LOCKED);
         locks[s] = new SeatLock(s, user);
      }
      return true;
   }

   void unlockSeats(vector<ShowSeat*> seats) {
      for (auto s : seats) {
         s->setStatus(AVAILABLE);
         locks.erase(s);
      }
   }

   void validateLockExpiry() {
      for (auto it = locks.begin(); it != locks.end();) {
         if (it->second->isExpired()) {
            it->first->setStatus(AVAILABLE);
            it = locks.erase(it);
         } else {
            ++it;
         }
      }
   }
};

class PaymentService {
public:
   bool processPayment(Payment* payment) {
        PaymentStrategy* strategy = PaymentFactory::getStrategy(payment->mode);
        if (strategy && strategy->pay(payment->amount)) {
            payment->setStatus(SUCCESS);
            return true;
        }
        payment->setStatus(FAILED);
        return false;
   }

   PaymentStatus getPaymentStatus(Payment* p) {
        return p->getStatus();
   }
};

class TicketService {
    unordered_map<Booking*, Ticket*> tickets;

public:
    Ticket* generateTicket(Booking* booking) {
        Ticket* t = new Ticket("T_" + booking->bookingId, booking);
        tickets[booking] = t;
        return t;
    }

    Ticket* getTicketByBooking(Booking* booking) {
        return tickets[booking];
    }
};

class NotificationService {
public:
    void sendNotification(User* user, string message) {
        cout << "Notify " << user->getUserId() << ": " << message << endl;
    }
};

class BookingService {
    SeatLockService* sls;
    PaymentService* ps;
    TicketService* ts;
    NotificationManager* nm;

public:
    BookingService(SeatLockService* s, PaymentService* p,
                   TicketService* t, NotificationManager* n)
        : sls(s), ps(p), ts(t), nm(n) {}

    Booking* createBooking(User* user, Show* show, vector<ShowSeat*> seats) {
        if (!sls->lockSeats(user, seats)) return nullptr;
        return new Booking("B1", user, show, seats);
    }

    bool confirmBooking(Booking* booking) {
        double total = 0;
        for (auto s : booking->seats) total += s->getPrice();

        Payment* payment = new Payment("P1", total, UPI);
        booking->payment = payment;

        if (ps->processPayment(payment)) {
            booking->setStatus(CONFIRMED);
            for (auto s : booking->seats) s->setStatus(BOOKED);

            ts->generateTicket(booking);
            nm->notifyAll(booking->user, "Booking Confirmed");
            return true;
        }

        sls->unlockSeats(booking->seats);
        booking->setStatus(CANCELLED);
        return false;
    }

    bool cancelBooking(Booking* booking) {
        sls->unlockSeats(booking->seats);
        booking->setStatus(CANCELLED);
        nm->notifyAll(booking->user, "Booking Cancelled");
        return true;
    }
};

int main() {

    User* user = new User("U1", "Ravi", "r@x.com", "999");

    Movie* movie = new Movie("M1", "Inception", 120, "EN", "Sci-Fi", "Dream", 8.8);

    Seat* s1 = new Seat("A1", 1, 1, REGULAR);
    Seat* s2 = new Seat("A2", 1, 2, REGULAR);

    vector<Seat*> baseSeats = {s1, s2};

    Screen* screen = new Screen("SC1", "Screen1", baseSeats);

    vector<ShowSeat*> showSeats = {
        new ShowSeat("SS1", s1, 200),
        new ShowSeat("SS2", s2, 200)
    };

    Show* show = new Show("SH1", movie, screen, time(0), time(0)+7200, showSeats);

    SeatLockService sls;
    PaymentService ps;
    TicketService ts;
    NotificationManager nm;

    nm.addObserver(new EmailService());
    nm.addObserver(new SMSService());

    BookingService bs(&sls, &ps, &ts, &nm);

    Booking* booking = bs.createBooking(user, show, showSeats);

    if (booking) bs.confirmBooking(booking);

    return 0;
}