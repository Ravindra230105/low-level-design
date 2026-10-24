#include <iostream>
#include <vector>
#include <climits>
#include <cmath>

using namespace std;

enum class RequestType {
    PICKUP,
    PICKDOWN,
    DESTINATION
};

enum class Direction {
    UP,
    DOWN,
    IDLE
};

class Request {
private:
    int floor;
    RequestType type;

public:
    Request(int floor, RequestType type)
        : floor(floor), type(type) {
    }

    int getFloor() {
        return floor;
    }

    RequestType getRequestType() {
        return type;
    }
};

class Elevator {
private:
    int currentFloor;
    Direction direction;
    vector<Request> requests;

public:
    Elevator()
        : currentFloor(0),
          direction(Direction::IDLE) {
    }

    Elevator(int currentFloor)
        : currentFloor(currentFloor),
          direction(Direction::IDLE) {
    }

    bool addRequest(Request& request) {
        requests.push_back(request);

        if (direction == Direction::IDLE) {
            if (request.getFloor() > currentFloor) {
                direction = Direction::UP;
            }
            else if (request.getFloor() < currentFloor) {
                direction = Direction::DOWN;
            }
            else {
                direction = Direction::IDLE;
            }
        }

        return true;
    }

    int getCurrentFloor() {
        return currentFloor;
    }

    Direction getDirection() {
        return direction;
    }

    bool hasRequestsAtOrBeyond(
        int floor,
        Direction direction
    ) {
        for ( auto& request : requests) {
            int requestFloor = request.getFloor();

            if (direction == Direction::UP &&
                requestFloor >= floor) {
                return true;
            }

            if (direction == Direction::DOWN &&
                requestFloor <= floor) {
                return true;
            }
        }

        return false;
    }

    void step() {
        if (requests.empty()) {
            direction = Direction::IDLE;
            return;
        }

        Request& request = requests.front();
        int targetFloor = request.getFloor();

        if (currentFloor < targetFloor) {
            currentFloor++;
            direction = Direction::UP;
        }
        else if (currentFloor > targetFloor) {
            currentFloor--;
            direction = Direction::DOWN;
        }
        else {
            requests.erase(requests.begin());

            if (requests.empty()) {
                direction = Direction::IDLE;
            }
        }
    }
};

class ElevatorController {
private:
    vector<Elevator> elevators;
    int elevatorCount;
    int minFloor;
    int maxFloor;

    Elevator* findCommitedToFloor(Request& request) {
        int floor = request.getFloor();

        Direction direction =
            (request.getRequestType() == RequestType::PICKUP)
                ? Direction::UP
                : Direction::DOWN;

        Elevator* nearest = nullptr;
        int minDistance = INT_MAX;

        for (auto& e : elevators) {
            if (e.getDirection() != direction) {
                continue;
            }

            if ((direction == Direction::UP &&
                 e.getCurrentFloor() > floor) ||
                (direction == Direction::DOWN &&
                 e.getCurrentFloor() < floor)) {
                continue;
            }

            if (!e.hasRequestsAtOrBeyond(floor, direction)) {
                continue;
            }

            int distance =
                abs(e.getCurrentFloor() - floor);

            if (distance < minDistance) {
                minDistance = distance;
                nearest = &e;
            }
        }

        return nearest;
    }

    Elevator* findNearestIdle(int floor) {
        Elevator* nearest = nullptr;
        int minDistance = INT_MAX;

        for (auto& e : elevators) {
            if (e.getDirection() != Direction::IDLE) {
                continue;
            }

            int distance =
                abs(e.getCurrentFloor() - floor);

            if (distance < minDistance) {
                minDistance = distance;
                nearest = &e;
            }
        }

        return nearest;
    }

    Elevator* findNearest(int floor) {
        if (elevators.empty()) {
            return nullptr;
        }

        Elevator* nearest = &elevators[0];

        int minDistance =
            abs(nearest->getCurrentFloor() - floor);

        for (auto& e : elevators) {
            int distance =
                abs(e.getCurrentFloor() - floor);

            if (distance < minDistance) {
                minDistance = distance;
                nearest = &e;
            }
        }

        return nearest;
    }

    Elevator* selectBestElevator(Request& request) {
        Elevator* best = findCommitedToFloor(request);

        if (best != nullptr) {
            return best;
        }

        best = findNearestIdle(request.getFloor());

        if (best != nullptr) {
            return best;
        }

        return findNearest(request.getFloor());
    }

public:
    ElevatorController(int elevatorCount, int minFloor, int maxFloor)
        : elevatorCount(elevatorCount),
          minFloor(minFloor),
          maxFloor(maxFloor) {

        for (int cnt = 0; cnt < elevatorCount; cnt++) {
            elevators.push_back(Elevator());
        }
    }

    bool requestElevator(int floor, RequestType type) {
        if (floor < minFloor || floor > maxFloor) {
            return false;
        }

        if (type == RequestType::DESTINATION) {
            return false;
        }

        Request request(floor, type);

        Elevator* best = selectBestElevator(request);

        if (best == nullptr) {
            return false;
        }

        return best->addRequest(request);
    }

    void step() {
        for (auto& elevator : elevators) {
            elevator.step();
        }
    }
};

int main() {
    ElevatorController controller(3, 0, 20);

    controller.requestElevator(5, RequestType::PICKUP);

    controller.requestElevator(10, RequestType::PICKUP);

    controller.requestElevator(2, RequestType::PICKDOWN);

    for (int i = 0; i < 15; i++) {
        controller.step();
    }

    cout << "Elevator simulation completed." << endl;

    return 0;
}