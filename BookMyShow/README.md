# BookMyShow (Movie Ticket Booking) — Low Level Design

[⬅ Back to index](../README.md)

**Code:** [`BookMyShow.cpp`](./BookMyShow.cpp) · C++17 · Notes: [`bookMyShow.txt`](./bookMyShow.txt)

## 1. Requirements

**Functional**
1. Users can **search movies by title** (case-insensitive substring match).
2. Users can **browse shows** of a movie — in code: shows at **ACTIVE theaters in the user's city**.
3. Theaters have **multiple screens**; all screens share the same seat layout (notes: rows A–Z, columns 0–20).
4. Users can **view available seats** for a show and select specific ones.
5. Users can **book multiple seats** in a single booking.
6. **Concurrent booking of the same seat** must be handled — only one user wins.
7. Users can **cancel a booking by confirmation ID**, which releases the seats.
8. **Payment processing** (UPI / Cash / Credit Card).

**Admin:** add theater, add screen/seats, add movie, add show, add user.

**User flow (from notes)**

> Search movie → list of movies → list of theaters / shows → available seats → select seats → **lock seats** → payment → seats booked

## 2. Core Entities

| Entity | Responsibility |
|---|---|
| `BookMyShowController` | Facade: owns users, movies, theaters, shows, bookings; ID generation; exposes all APIs |
| `User` | id, name, email, optional `Address` |
| `Movie` | id, title, duration (min), description |
| `Theater` | name, `Address`, `TheaterStatus`, list of `Screen`s |
| `Screen` | Owns the physical `Seat`s |
| `Seat` | id, row (`char`), column; `getSeatNumber()` → e.g. `A4` |
| `Show` | Movie × Screen × startTime; tracks **booked seat ids** (`unordered_set<int>`) |
| `SeatLock` | Temporary hold on (show, seat) with an **expiry time** |
| `SeatLockManager` | Locks / confirms / unlocks seats; background thread purges expired locks |
| `Booking` | confirmationId (`BMS<n>`), user, show, seatIds, amount, `BookingStatus` |
| `PaymentStrategy` (+ `UPI`, `Cash`, `CreditCard`) | Pluggable payment method |
| `Payment` | Context that delegates to a `PaymentStrategy` |
| `Search` | Title search over movies |
| `Address` | city, street (used for nearby-theater filtering) |
| `TheaterStatus` / `BookingStatus` | Enums |

## 3. Class Diagram

```mermaid
classDiagram
    class BookMyShowController {
        -vector~User*~ users
        -vector~Movie*~ movies
        -vector~Theater*~ theaters
        -vector~Show*~ shows
        -unordered_map showIndex
        -unordered_map bookings
        -SeatLockManager lockManager
        -Search search
        -mutex bookingMutex
        +addUser(name, email) User*
        +addMovie(title, duration, desc) Movie*
        +addTheater(name, address) Theater*
        +addScreen(theater, name) Screen*
        +addSeat(screen, row, col) Seat*
        +addShow(screen, movie, startTime) Show*
        +searchMovie(title) vector~Movie*~
        +getShows(movie, userAddress) vector~Show*~
        +getAvailableSeats(show) vector~int~
        +createBooking(user, show, seatIds, amount) Booking*
        +makePayment(confirmationId, strategy) bool
        +cancelBooking(confirmationId) bool
    }
    class User {
        -int id
        -string name
        -string email
        -Address* address
    }
    class Address {
        -string city
        -string street
    }
    class Movie {
        -int id
        -string title
        -int duration
        -string description
    }
    class Theater {
        -int id
        -string name
        -Address* address
        -TheaterStatus status
        -vector~Screen*~ screens
        +addScreen(screen)
    }
    class Screen {
        -int id
        -string name
        -vector~Seat*~ seats
        +addSeat(seat)
        +getSeat(seatId) Seat*
    }
    class Seat {
        -int id
        -char row
        -int column
        +getSeatNumber() string
    }
    class Show {
        -int id
        -Screen* screen
        -Movie* movie
        -string startTime
        -unordered_set~int~ bookedSeats
        +isBooked(seatId) bool
        +bookSeat(seatId)
        +releaseSeat(seatId)
    }
    class SeatLock {
        -int showId
        -int seatId
        -time_t expiryTime
        +isExpired() bool
    }
    class SeatLockManager {
        -unordered_map locks
        -mutex lockMutex
        -atomic~bool~ stopped
        -thread cleanupThread
        +lockSeats(show, seatIds) bool
        +confirmSeats(show, seatIds) bool
        +unlockSeats(show, seatIds)
        +getLockedSeats(show) vector~int~
    }
    class Booking {
        -string confirmationId
        -User* user
        -Show* show
        -vector~int~ seatIds
        -double amount
        -BookingStatus status
        +confirm()
        +cancel()
        +expire()
    }
    class Payment {
        -PaymentStrategy* strategy
        +makePayment(amount) bool
    }
    class PaymentStrategy {
        <<interface>>
        +pay(amount) bool
    }
    class UPI
    class Cash
    class CreditCard
    class Search {
        +searchByTitle(movies, title) vector~Movie*~
    }
    class TheaterStatus {
        <<enumeration>>
        ACTIVE
        CLOSED
        INACTIVE
    }
    class BookingStatus {
        <<enumeration>>
        PENDING
        CONFIRMED
        CANCELLED
        EXPIRED
    }

    BookMyShowController o-- User
    BookMyShowController o-- Movie
    BookMyShowController o-- Theater
    BookMyShowController o-- Show
    BookMyShowController o-- Booking
    BookMyShowController *-- SeatLockManager
    BookMyShowController *-- Search
    Theater *-- Screen
    Theater --> Address
    Theater --> TheaterStatus
    User --> Address
    Screen *-- Seat
    Show --> Screen
    Show --> Movie
    SeatLockManager *-- SeatLock
    Booking --> User
    Booking --> Show
    Booking --> BookingStatus
    Payment --> PaymentStrategy
    PaymentStrategy <|-- UPI
    PaymentStrategy <|-- Cash
    PaymentStrategy <|-- CreditCard
```

## 4. Design Patterns Used

| Pattern | Where | Why |
|---|---|---|
| **Strategy** | `PaymentStrategy` → `UPI` / `Cash` / `CreditCard`, used by `Payment` | Add new payment methods without touching booking logic (OCP) |
| **Facade / Controller** | `BookMyShowController` | Single entry point hiding seat locking, indexing, payment, ID generation |
| **Temporary lock with TTL** (pessimistic hold) | `SeatLock` + `SeatLockManager` | Holds seats while the user pays; auto-releases if abandoned |
| **Background cleaner thread** | `SeatLockManager::cleanup()` | Every 1 s purges expired locks so seats return to the pool |
| **Index (in-memory lookup)** | `showIndex : Movie → Theater → vector<Show*>` | Fast "shows of movie X at theater Y" lookup |

## 5. Key Flows

### Booking flow (lock → pay → confirm)

```mermaid
sequenceDiagram
    actor U as User
    participant C as BookMyShowController
    participant L as SeatLockManager
    participant P as Payment
    participant S as Show

    U->>C: searchMovie("avenger")
    C-->>U: matching movies
    U->>C: getShows(movie, userAddress)
    C-->>U: shows at ACTIVE theaters in same city
    U->>C: getAvailableSeats(show)
    C->>L: getLockedSeats(show)
    C-->>U: seats not booked and not locked
    U->>C: createBooking(user, show, seatIds, amount)
    C->>L: lockSeats(show, seatIds) with TTL 120s
    alt any seat booked or locked
        L-->>C: false
        C-->>U: NULL (booking failed)
    else all free
        L-->>C: true
        C-->>U: Booking PENDING with confirmationId BMSn
    end
    U->>C: makePayment(confirmationId, UPI)
    C->>P: makePayment(amount)
    alt payment failed
        C->>L: unlockSeats
        C-->>U: false (booking EXPIRED)
    else payment ok
        C->>L: confirmSeats(show, seatIds)
        L->>S: bookSeat(seatId) for each
        C-->>U: true (booking CONFIRMED)
    end
```

- **All-or-nothing locking:** `lockSeats` first checks *every* seat (booked? locked and unexpired?), and only then locks all of them.
- **Seat availability** = screen seats − `show.bookedSeats` − unexpired locks.
- **Lock expired before payment?** `confirmSeats` fails → booking marked `EXPIRED`.
- **Cancel:** `cancelBooking(confirmationId)` → only allowed if `CONFIRMED` → `show.releaseSeat()` for each seat → `CANCELLED`.
- **Lock key:** `"<showId>_<seatId>"` — locks are **per show**, so the same physical seat can be held in different shows.

### Booking state machine

```mermaid
stateDiagram-v2
    [*] --> PENDING: createBooking (seats locked)
    PENDING --> CONFIRMED: payment ok and locks still valid
    PENDING --> EXPIRED: payment failed or lock expired
    CONFIRMED --> CANCELLED: cancelBooking
    CANCELLED --> [*]
    EXPIRED --> [*]
```

## 6. Concurrency & Edge Cases

| Concern | How it's handled |
|---|---|
| **Two users booking the same seat** | `createBooking` runs under `bookingMutex`; `lockSeats` runs under `lockMutex` → check-then-lock is atomic. Second user gets `NULL` |
| **Abandoned checkout** | `SeatLock` has a **120 s expiry**; cleanup thread deletes expired locks every second; `lockSeats` also overwrites an expired lock |
| **Payment after lock expiry** | `confirmSeats` validates each lock exists and is unexpired → else booking `EXPIRED` |
| **Payment failure** | Seats unlocked, booking `EXPIRED` |
| **Double pay / pay on cancelled booking** | `makePayment` only proceeds if status is `PENDING` |
| **Cancel invalid / unconfirmed booking** | Unknown ID or non-`CONFIRMED` status → returns `false` |
| **Inactive / closed theaters** | Filtered out in `getNearbyTheaters` |
| **Graceful shutdown** | `~SeatLockManager` sets `atomic<bool> stopped`, joins the cleanup thread, frees locks |

Demo (`main`): Rahul wants seats 5,6,7 and Aman wants 7,8,9 on two threads → seat 7 overlaps → exactly one booking succeeds.

**Trade-offs worth mentioning**
- `bookingMutex` is a **single global lock** — simple and correct, but serializes all bookings across all shows (and the payment call runs while it's held). A per-show lock would scale better.
- Price is passed in by the caller (`seats × 200` in the demo); there is no pricing model per seat type.

## 7. How to Run

```bash
g++ -std=c++17 BookMyShow.cpp -o bookmyshow && ./bookmyshow
```

Sample output (thread order may vary):

```
Movies found: 1
Shows near user: 1
Show: 7:00 PM
Available seats: 1 2 3 4 5 6 7 8 9 10
Rahul seats locked. Confirmation ID: BMS1
Aman booking failed
UPI payment: 600
Rahul booking confirmed
```

---
### 🧠 Quick Revision
- **Show** (movie × screen × time) owns booking state — seats are physical, availability is **per show**.
- Booking is **two-phase**: `lockSeats` (TTL hold, status `PENDING`) → payment → `confirmSeats` (`CONFIRMED`).
- **All-or-nothing** seat locking under a mutex prevents double booking of overlapping seat sets.
- **TTL locks + background cleanup thread** release seats from abandoned checkouts.
- **Strategy pattern** for payments (UPI / Cash / CreditCard) — add a method without touching the controller.
- **`showIndex` (Movie → Theater → Shows)** + city filter on ACTIVE theaters powers show discovery.
- Cancellation by **confirmation ID** releases seats back to the show.
- Scale-up talking point: replace the global `bookingMutex` with per-show locks / DB row locks or Redis `SETNX` with TTL.
