#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <thread>
#include <chrono>
#include <algorithm>
#include <atomic>
#include <cctype>
#include <ctime>

using namespace std;

enum TheaterStatus {
    ACTIVE,
    CLOSED,
    INACTIVE
};

enum BookingStatus {
    PENDING,
    CONFIRMED,
    CANCELLED,
    EXPIRED
};

class Address {

    string city;
    string street;

public:

    Address(string city, string street) {
        this->city = city;
        this->street = street;
    }

    string getCity() {
        return city;
    }

    string getStreet() {
        return street;
    }
};

class User {

    int id;
    string name;
    string email;
    Address* address;

public:

    User(int id, string name, string email) {
        this->id = id;
        this->name = name;
        this->email = email;
        this->address = NULL;
    }

    int getId() {
        return id;
    }

    string getName() {
        return name;
    }

    string getEmail() {
        return email;
    }

    Address* getAddress() {
        return address;
    }

    void setAddress(Address* address) {
        this->address = address;
    }
};

class Movie {

    int id;
    string title;
    int duration;
    string description;

public:

    Movie(int id, string title, int duration, string description) {
        this->id = id;
        this->title = title;
        this->duration = duration;
        this->description = description;
    }

    int getId() {
        return id;
    }

    string getTitle() {
        return title;
    }
};

class Seat {

    int id;
    char row;
    int column;

public:

    Seat(int id, char row, int column) {
        this->id = id;
        this->row = row;
        this->column = column;
    }

    int getId() {
        return id;
    }

    char getRow() {
        return row;
    }

    int getColumn() {
        return column;
    }

    string getSeatNumber() {
        return string(1, row) + to_string(column);
    }
};

class Screen {

    int id;
    string name;
    vector<Seat*> seats;

public:

    Screen(int id, string name) {
        this->id = id;
        this->name = name;
    }

    ~Screen() {
        for (Seat* seat : seats) {
            delete seat;
        }
    }

    int getId() {
        return id;
    }

    string getName() {
        return name;
    }

    void addSeat(Seat* seat) {
        seats.push_back(seat);
    }

    vector<Seat*> getSeats() {
        return seats;
    }

    Seat* getSeat(int seatId) {
        for (Seat* seat : seats) {
            if (seat->getId() == seatId) {
                return seat;
            }
        }

        return NULL;
    }
};

class Theater {

    int id;
    string name;
    Address* address;
    TheaterStatus status;
    vector<Screen*> screens;

public:

    Theater(int id, string name, Address* address) {
        this->id = id;
        this->name = name;
        this->address = address;
        this->status = ACTIVE;
    }

    ~Theater() {

        for (Screen* screen : screens) {
            delete screen;
        }

        delete address;
    }

    int getId() {
        return id;
    }

    string getName() {
        return name;
    }

    Address* getAddress() {
        return address;
    }

    TheaterStatus getStatus() {
        return status;
    }

    void addScreen(Screen* screen) {
        screens.push_back(screen);
    }

    vector<Screen*> getScreens() {
        return screens;
    }
};

class Show {

    int id;
    Screen* screen;
    Movie* movie;
    string startTime;
    unordered_set<int> bookedSeats;

public:

    Show(
        int id,
        Screen* screen,
        Movie* movie,
        string startTime
    ) {
        this->id = id;
        this->screen = screen;
        this->movie = movie;
        this->startTime = startTime;
    }

    int getId() {
        return id;
    }

    Screen* getScreen() {
        return screen;
    }

    Movie* getMovie() {
        return movie;
    }

    string getStartTime() {
        return startTime;
    }

    bool isBooked(int seatId) {
        return bookedSeats.find(seatId) != bookedSeats.end();
    }

    void bookSeat(int seatId) {
        bookedSeats.insert(seatId);
    }

    void releaseSeat(int seatId) {
        bookedSeats.erase(seatId);
    }
};

class SeatLock {

    int showId;
    int seatId;
    time_t expiryTime;

public:

    SeatLock(int showId, int seatId, int duration) {
        this->showId = showId;
        this->seatId = seatId;
        expiryTime = time(NULL) + duration;
    }

    int getShowId() {
        return showId;
    }

    int getSeatId() {
        return seatId;
    }

    bool isExpired() {
        return time(NULL) >= expiryTime;
    }
};

class SeatLockManager {

    unordered_map<string, SeatLock*> locks;

    mutex lockMutex;

    atomic<bool> stopped;

    thread cleanupThread;

    string getKey(int showId, int seatId) {
        return to_string(showId) + "_" + to_string(seatId);
    }

    void cleanup() {

        while (!stopped) {

            this_thread::sleep_for(chrono::seconds(1));

            lockMutex.lock();

            vector<string> expiredKeys;

            for (auto entry : locks) {

                if (entry.second->isExpired()) {
                    expiredKeys.push_back(entry.first);
                }
            }

            for (string key : expiredKeys) {

                delete locks[key];

                locks.erase(key);
            }

            lockMutex.unlock();
        }
    }

public:

    SeatLockManager() {

        stopped = false;

        cleanupThread =
            thread(&SeatLockManager::cleanup, this);
    }

    ~SeatLockManager() {

        stopped = true;

        if (cleanupThread.joinable()) {
            cleanupThread.join();
        }

        lockMutex.lock();

        for (auto entry : locks) {
            delete entry.second;
        }

        locks.clear();

        lockMutex.unlock();
    }

    bool lockSeats(
        Show* show,
        vector<int> seatIds
    ) {

        lockMutex.lock();

        for (int seatId : seatIds) {

            string key =
                getKey(show->getId(), seatId);

            if (show->isBooked(seatId)) {

                lockMutex.unlock();

                return false;
            }

            if (locks.find(key) != locks.end()) {

                if (!locks[key]->isExpired()) {

                    lockMutex.unlock();

                    return false;
                }

                delete locks[key];

                locks.erase(key);
            }
        }

        for (int seatId : seatIds) {

            string key =
                getKey(show->getId(), seatId);

            locks[key] =
                new SeatLock(
                    show->getId(),
                    seatId,
                    120
                );
        }

        lockMutex.unlock();

        return true;
    }

    bool confirmSeats(
        Show* show,
        vector<int> seatIds
    ) {

        lockMutex.lock();

        for (int seatId : seatIds) {

            string key =
                getKey(show->getId(), seatId);

            if (locks.find(key) == locks.end()) {

                lockMutex.unlock();

                return false;
            }

            if (locks[key]->isExpired()) {

                lockMutex.unlock();

                return false;
            }
        }

        for (int seatId : seatIds) {

            string key =
                getKey(show->getId(), seatId);

            show->bookSeat(seatId);

            delete locks[key];

            locks.erase(key);
        }

        lockMutex.unlock();

        return true;
    }

    void unlockSeats(
        Show* show,
        vector<int> seatIds
    ) {

        lockMutex.lock();

        for (int seatId : seatIds) {

            string key =
                getKey(show->getId(), seatId);

            if (locks.find(key) != locks.end()) {

                delete locks[key];

                locks.erase(key);
            }
        }

        lockMutex.unlock();
    }

    vector<int> getLockedSeats(Show* show) {

        lockMutex.lock();

        vector<int> result;

        for (auto entry : locks) {

            if (entry.second->getShowId() ==
                show->getId()) {

                if (!entry.second->isExpired()) {

                    result.push_back(
                        entry.second->getSeatId()
                    );
                }
            }
        }

        lockMutex.unlock();

        return result;
    }
};

class PaymentStrategy {

public:

    virtual bool pay(double amount) = 0;

    virtual ~PaymentStrategy() {}
};

class UPI : public PaymentStrategy {

public:

    bool pay(double amount) {

        cout << "UPI payment: "
             << amount
             << endl;

        return true;
    }
};

class Cash : public PaymentStrategy {

public:

    bool pay(double amount) {

        cout << "Cash payment: "
             << amount
             << endl;

        return true;
    }
};

class CreditCard : public PaymentStrategy {

public:

    bool pay(double amount) {

        cout << "Credit Card payment: "
             << amount
             << endl;

        return true;
    }
};

class Payment {

    PaymentStrategy* strategy;

public:

    Payment(PaymentStrategy* strategy) {
        this->strategy = strategy;
    }

    bool makePayment(double amount) {
        return strategy->pay(amount);
    }
};

class Booking {

    int id;

    string confirmationId;

    User* user;

    Show* show;

    vector<int> seatIds;

    double amount;

    BookingStatus status;

public:

    Booking(
        int id,
        string confirmationId,
        User* user,
        Show* show,
        vector<int> seatIds,
        double amount
    ) {

        this->id = id;
        this->confirmationId = confirmationId;
        this->user = user;
        this->show = show;
        this->seatIds = seatIds;
        this->amount = amount;
        this->status = PENDING;
    }

    int getId() {
        return id;
    }

    string getConfirmationId() {
        return confirmationId;
    }

    User* getUser() {
        return user;
    }

    Show* getShow() {
        return show;
    }

    vector<int> getSeatIds() {
        return seatIds;
    }

    double getAmount() {
        return amount;
    }

    BookingStatus getStatus() {
        return status;
    }

    void confirm() {
        status = CONFIRMED;
    }

    void cancel() {
        status = CANCELLED;
    }

    void expire() {
        status = EXPIRED;
    }
};

class Search {

public:

    vector<Movie*> searchByTitle(
        vector<Movie*> movies,
        string title
    ) {

        vector<Movie*> result;

        transform(
            title.begin(),
            title.end(),
            title.begin(),
            ::tolower
        );

        for (Movie* movie : movies) {

            string movieTitle =
                movie->getTitle();

            transform(
                movieTitle.begin(),
                movieTitle.end(),
                movieTitle.begin(),
                ::tolower
            );

            if (
                movieTitle.find(title)
                != string::npos
            ) {
                result.push_back(movie);
            }
        }

        return result;
    }
};

class BookMyShowController {

    vector<User*> users;

    vector<Movie*> movies;

    vector<Theater*> theaters;

    vector<Show*> shows;

    unordered_map<
        Movie*,
        unordered_map<
            Theater*,
            vector<Show*>
        >
    > showIndex;

    unordered_map<string, Booking*> bookings;

    SeatLockManager lockManager;

    Search search;

    int userId;
    int movieId;
    int theaterId;
    int screenId;
    int seatId;
    int showId;
    int bookingId;

    mutex bookingMutex;

    Theater* findTheaterByScreen(Screen* screen) {

        for (Theater* theater : theaters) {

            for (Screen* theaterScreen :
                 theater->getScreens()) {

                if (theaterScreen == screen) {
                    return theater;
                }
            }
        }

        return NULL;
    }

    vector<Theater*> getNearbyTheaters(
        Address* address
    ) {

        vector<Theater*> result;

        for (Theater* theater : theaters) {

            if (theater->getStatus() != ACTIVE) {
                continue;
            }

            if (
                theater->getAddress()->getCity()
                == address->getCity()
            ) {
                result.push_back(theater);
            }
        }

        return result;
    }

public:

    BookMyShowController() {

        userId = 1;
        movieId = 1;
        theaterId = 1;
        screenId = 1;
        seatId = 1;
        showId = 1;
        bookingId = 1;
    }

    ~BookMyShowController() {

        for (auto& entry : bookings) {
            delete entry.second;
        }

        for (Show* show : shows) {
            delete show;
        }

        for (Theater* theater : theaters) {
            delete theater;
        }

        for (Movie* movie : movies) {
            delete movie;
        }

        for (User* user : users) {
            delete user;
        }
    }

    User* addUser(
        string name,
        string email
    ) {

        User* user =
            new User(
                userId++,
                name,
                email
            );

        users.push_back(user);

        return user;
    }

    Movie* addMovie(
        string title,
        int duration,
        string description
    ) {

        Movie* movie =
            new Movie(
                movieId++,
                title,
                duration,
                description
            );

        movies.push_back(movie);

        return movie;
    }

    Theater* addTheater(
        string name,
        Address* address
    ) {

        Theater* theater =
            new Theater(
                theaterId++,
                name,
                address
            );

        theaters.push_back(theater);

        return theater;
    }

    Screen* addScreen(
        Theater* theater,
        string name
    ) {

        Screen* screen =
            new Screen(
                screenId++,
                name
            );

        theater->addScreen(screen);

        return screen;
    }

    Seat* addSeat(
        Screen* screen,
        char row,
        int column
    ) {

        Seat* seat =
            new Seat(
                seatId++,
                row,
                column
            );

        screen->addSeat(seat);

        return seat;
    }

    Show* addShow(
        Screen* screen,
        Movie* movie,
        string startTime
    ) {

        Show* show =
            new Show(
                showId++,
                screen,
                movie,
                startTime
            );

        shows.push_back(show);

        Theater* theater =
            findTheaterByScreen(screen);

        if (theater != NULL) {
            showIndex[movie][theater].push_back(show);
        }

        return show;
    }

    vector<Movie*> searchMovie(
        string title
    ) {

        return search.searchByTitle(
            movies,
            title
        );
    }

    vector<Show*> getShows(
        Movie* movie,
        Address* userAddress
    ) {

        vector<Show*> result;

        vector<Theater*> nearbyTheaters =
            getNearbyTheaters(userAddress);

        if (showIndex.find(movie) ==
            showIndex.end()) {

            return result;
        }

        for (Theater* theater : nearbyTheaters) {

            if (
                showIndex[movie].find(theater)
                == showIndex[movie].end()
            ) {
                continue;
            }

            vector<Show*>& theaterShows =
                showIndex[movie][theater];

            for (Show* show : theaterShows) {
                result.push_back(show);
            }
        }

        return result;
    }

    vector<int> getAvailableSeats(
        Show* show
    ) {

        bookingMutex.lock();

        vector<int> result;

        vector<int> lockedSeats =
            lockManager.getLockedSeats(show);

        for (
            Seat* seat :
            show->getScreen()->getSeats()
        ) {

            int id = seat->getId();

            if (show->isBooked(id)) {
                continue;
            }

            bool locked = false;

            for (int lockedSeat : lockedSeats) {

                if (lockedSeat == id) {

                    locked = true;

                    break;
                }
            }

            if (!locked) {
                result.push_back(id);
            }
        }

        bookingMutex.unlock();

        return result;
    }

    Booking* createBooking(
        User* user,
        Show* show,
        vector<int> seatIds,
        double amount
    ) {

        bookingMutex.lock();

        bool success =
            lockManager.lockSeats(
                show,
                seatIds
            );

        if (!success) {

            bookingMutex.unlock();

            return NULL;
        }

        string confirmationId =
            "BMS" + to_string(bookingId);

        Booking* booking =
            new Booking(
                bookingId++,
                confirmationId,
                user,
                show,
                seatIds,
                amount
            );

        bookings[confirmationId] =
            booking;

        bookingMutex.unlock();

        return booking;
    }

    bool makePayment(
        string confirmationId,
        PaymentStrategy* strategy
    ) {

        bookingMutex.lock();

        if (
            bookings.find(confirmationId)
            == bookings.end()
        ) {

            bookingMutex.unlock();

            return false;
        }

        Booking* booking =
            bookings[confirmationId];

        if (
            booking->getStatus()
            != PENDING
        ) {

            bookingMutex.unlock();

            return false;
        }

        Payment payment(strategy);

        bool paid =
            payment.makePayment(
                booking->getAmount()
            );

        if (!paid) {

            booking->expire();

            lockManager.unlockSeats(
                booking->getShow(),
                booking->getSeatIds()
            );

            bookingMutex.unlock();

            return false;
        }

        bool confirmed =
            lockManager.confirmSeats(
                booking->getShow(),
                booking->getSeatIds()
            );

        if (!confirmed) {

            booking->expire();

            bookingMutex.unlock();

            return false;
        }

        booking->confirm();

        bookingMutex.unlock();

        return true;
    }

    bool cancelBooking(
        string confirmationId
    ) {

        bookingMutex.lock();

        if (
            bookings.find(confirmationId)
            == bookings.end()
        ) {

            bookingMutex.unlock();

            return false;
        }

        Booking* booking =
            bookings[confirmationId];

        if (
            booking->getStatus()
            != CONFIRMED
        ) {

            bookingMutex.unlock();

            return false;
        }

        for (
            int seatId :
            booking->getSeatIds()
        ) {

            booking->getShow()
                ->releaseSeat(seatId);
        }

        booking->cancel();

        bookingMutex.unlock();

        return true;
    }
};

void tryBooking(
    BookMyShowController* controller,
    User* user,
    Show* show,
    vector<int> seats
) {

    Booking* booking =
        controller->createBooking(
            user,
            show,
            seats,
            seats.size() * 200
        );

    if (booking == NULL) {

        cout << user->getName()
             << " booking failed"
             << endl;

        return;
    }

    cout << user->getName()
         << " seats locked. Confirmation ID: "
         << booking->getConfirmationId()
         << endl;

    UPI upi;

    bool success =
        controller->makePayment(
            booking->getConfirmationId(),
            &upi
        );

    if (success) {

        cout << user->getName()
             << " booking confirmed"
             << endl;
    }
    else {

        cout << user->getName()
             << " payment failed"
             << endl;
    }
}

int main() {

    BookMyShowController controller;

    User* user1 =
        controller.addUser(
            "Rahul",
            "rahul@gmail.com"
        );

    User* user2 =
        controller.addUser(
            "Aman",
            "aman@gmail.com"
        );

    Address* userAddress =
        new Address(
            "Pune",
            "FC Road"
        );

    user1->setAddress(userAddress);

    Movie* movie =
        controller.addMovie(
            "Avengers",
            180,
            "Superhero movie"
        );

    Address* theaterAddress =
        new Address(
            "Pune",
            "FC Road"
        );

    Theater* theater =
        controller.addTheater(
            "PVR",
            theaterAddress
        );

    Screen* screen =
        controller.addScreen(
            theater,
            "Screen 1"
        );

    vector<Seat*> seats;

    for (int i = 0; i < 10; i++) {

        seats.push_back(
            controller.addSeat(
                screen,
                'A',
                i
            )
        );
    }

    Show* show =
        controller.addShow(
            screen,
            movie,
            "7:00 PM"
        );

    vector<Movie*> movies =
        controller.searchMovie("avenger");

    cout << "Movies found: "
         << movies.size()
         << endl;

    vector<Show*> availableShows =
        controller.getShows(
            movie,
            user1->getAddress()
        );

    cout << "Shows near user: "
         << availableShows.size()
         << endl;

    for (Show* currentShow :
         availableShows) {

        cout << "Show: "
             << currentShow->getStartTime()
             << endl;
    }

    vector<int> available =
        controller.getAvailableSeats(show);

    cout << "Available seats: ";

    for (int seatId : available) {
        cout << seatId << " ";
    }

    cout << endl;

    vector<int> user1Seats = {
        seats[4]->getId(),
        seats[5]->getId(),
        seats[6]->getId()
    };

    vector<int> user2Seats = {
        seats[6]->getId(),
        seats[7]->getId(),
        seats[8]->getId()
    };

    thread t1(
        tryBooking,
        &controller,
        user1,
        show,
        user1Seats
    );

    thread t2(
        tryBooking,
        &controller,
        user2,
        show,
        user2Seats
    );

    t1.join();

    t2.join();

    return 0;
}