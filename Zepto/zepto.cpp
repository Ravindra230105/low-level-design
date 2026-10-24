#include<bits/stdc++.h>
#include <algorithm>
using namespace std;

enum class UserRole { CONSUMER, ADMIN, DELIVERY_PARTNER };

enum class NodeStatus { ACTIVE, INACTIVE, CLOSED };

enum class InventoryCategory { FRUIT, ELECTRONICS };

enum class UOM { KG, NOS, GRAM };

enum class OrderStatus { CREATED, PAYMENT_PENDING, CONFIRMED, CANCELLED, COMPLETED };

enum class PaymentStatus { PENDING, FAILED, SUCCESS };

enum class PaymentMode { UPI, CASH, CARD };

class GlobalIdGenerator {
private:
    int userId = 1;
    int nodeId = 1;
    int inventoryId = 1;
    int nodeInventoryId = 1;
    int itemId = 1;
    int cartId = 1;
    int orderId = 1;
    int paymentId = 1;

    GlobalIdGenerator() {}

public:
    GlobalIdGenerator(const GlobalIdGenerator&) = delete;
    GlobalIdGenerator& operator=(const GlobalIdGenerator&) = delete;

    static GlobalIdGenerator& getInstance() {
        static GlobalIdGenerator instance;
        return instance;
    }

    int getUserId() {
        return userId++;
    }

    int getNodeId() {
        return nodeId++;
    }

    int getInventoryId() {
        return inventoryId++;
    }

    int getNodeInventoryId() {
        return nodeInventoryId++;
    }

    int getItemId() {
        return itemId++;
    }

    int getCartId() {
        return cartId++;
    }

    int getOrderId() {
        return orderId++;
    }

    int getPaymentId() {
        return paymentId++;
    }
};

class User {
    private:
        int userId;
        string name;
        string email;
        UserRole role;

    public:
        User(string name, string email, UserRole role) {
            userId = GlobalIdGenerator::getInstance().getUserId();
            this->name = name;
            this->email = email;
            this->role = role;
        }

        int getUserId() {
            return userId;
        }

        string getUserEmail() {
            return email;
        }
};

class Address {
    private:
        string city;
        string area;
        string pincode;
        string state;

    public:
        Address() {}
        Address(string city, string area, string pincode, string state) {
            this->city = city;
            this->area = area;
            this->pincode = pincode;
            this->state = state;
        }
};

class Node {
    private: 
        int nodeId;
        string name;
        Address* address;
        NodeStatus nodeStatus;

    public:
        Node(string name, Address* address) {
            nodeId = GlobalIdGenerator::getInstance().getNodeId();
            this->name = name;
            this->address = address;
            nodeStatus = NodeStatus::ACTIVE;
        }

        int getNodeId() {
            return nodeId;
        }

        NodeStatus getNodeStatus() {
            return nodeStatus;
        }
};

class Inventory {
    private: 
        int inventoryId;
        string name;
        string description;
        InventoryCategory category;
        double price;
        UOM uom;

    public: 
        Inventory(string name, string description, InventoryCategory category, double price, UOM uom) {
            inventoryId = GlobalIdGenerator::getInstance().getInventoryId();
            this->name = name;
            this->description = description;
            this->category = category;
            this->price = price;
            this->uom = uom;
        } 

        void setPrice(double price) {
            this->price = price;
        }

        double getPrice() {
            return price;
        }

        int getInventoryId() {
            return inventoryId;
        }

        string getName() {
            return name;
        }
};

class NodeInventory {
    private:
        int nodeInventoryId;
        int nodeId;
        int inventoryId;
        double quantity;
        mutex mtx;

    public:
        NodeInventory(int nodeId, int inventoryId, double quantity) {
            nodeInventoryId = GlobalIdGenerator::getInstance().getNodeInventoryId();
            this->nodeId = nodeId;
            this->inventoryId = inventoryId;
            this->quantity = quantity;
        }

        int getNodeId() {
            return nodeId;
        }

        int getInventoryId() {
            return inventoryId;
        }

        double getAvailableQuantity() {
            mtx.lock();
            double result = quantity;
            mtx.unlock();
            return result;
        }

        int getNodeInventoryId() {
            return nodeInventoryId;
        }

        bool reserveQuantity(double requestedQuantity) {
            mtx.lock();

            if (requestedQuantity <= 0) {
                mtx.unlock();
                return false;
            }

            if (quantity < requestedQuantity) {
                return false;
            }

            quantity -= requestedQuantity;
            mtx.unlock();
            return true;
        }

        void releaseQuantity(double quantityToRelease) {
            mtx.lock();
            quantity += quantityToRelease;
            mtx.unlock();
        }
};

class Item {
    private:
        int itemId;
        int nodeInventoryId;
        double quantity;
        double price;

    public:
        Item(int nodeInventoryId, double quantity, double price) {
            itemId = GlobalIdGenerator::getInstance().getItemId();
            this->nodeInventoryId = nodeInventoryId;
            this->quantity = quantity;
            this->price = price;
        }

        double getQuantity() {
            return quantity;
        }

        double getPrice() {
            return price;
        }

        double getTotalPrice() {
            double totalPrice = quantity * price;
            return totalPrice;
        }

        int getNodeInventoryId() {
            return nodeInventoryId;
        }
};

class Cart {
    private:
        int cartId;
        vector<Item*>items;
        int userId;

    public:
        Cart(int userId) {
            cartId = GlobalIdGenerator::getInstance().getCartId();
            this->userId = userId;
        }

        int getUserId() {
            return userId;
        }

        int getCartId() {
            return cartId;
        }

        void addItem(Item* item) {
            items.push_back(item);
        }

        void removeItem(Item *item) {
            for (auto it = items.begin(); it != items.end(); it++) {
                if (*it == item) {
                    items.erase(it);
                    return;
                }
            }
        }

        vector<Item*> getCartItems() {
            return items;
        }

        double getCartTotal() {
            double cartTotal = 0;
            for(auto item: items) {
                cartTotal += item->getTotalPrice();
            }

            return cartTotal;
        }

        void clearCart() {
           items.clear();
        }
};

class PaymentStrategy {
    public:
        virtual bool pay(double amount) = 0;
        virtual ~PaymentStrategy() {}
};

class UPIPayment : public PaymentStrategy {
    public:
        bool pay(double amount) override {
            cout << amount << " Rs. Paid Successfully via UPI" << endl;
            return true;
        }
};

class CashPayment : public PaymentStrategy {
    public:
        bool pay(double amount) override {
            cout << amount << " Rs. Paid Successfully via Cash" << endl;
            return true;
        }
};

class CardPayment : public PaymentStrategy {
    public:
        bool pay(double amount) override {
            cout << amount << " Rs. Paid Successfully via Card" << endl;
            return true;
        }
};

class Discount {
    protected:
        double minOrderAmount;
        double maxDiscount;

    public:
        virtual bool isCartApplicable(Cart* cart) = 0;
        virtual double calculateDiscount(Cart* cart) = 0;
        virtual ~Discount() = default;
};

class FlatDiscount : public Discount {
    private:
        double discountAmount;

    public:
        FlatDiscount(double minOrderAmount, double maxDiscount, double discountAmount) {
            this->minOrderAmount = minOrderAmount;
            this->maxDiscount = maxDiscount;
            this->discountAmount = discountAmount;
        }

        bool isCartApplicable(Cart* cart) override {
            double cartTotal = 0;
            for(auto item: cart->getCartItems()) {
                cartTotal += item->getTotalPrice();
            }

            if (cartTotal >= minOrderAmount) {
                return true;
            }

            return false;
        }

        double calculateDiscount(Cart* cart) override {
            return discountAmount;
        }
};

class PercentageDiscount : public Discount {
    private:
        double discountPercentage;

    public:
        PercentageDiscount(double minOrderAmount, double maxDiscount, double discountPercentage) {
            this->minOrderAmount = minOrderAmount;
            this->maxDiscount = maxDiscount;
            this->discountPercentage = discountPercentage;
        }

        bool isCartApplicable(Cart* cart) override {
            double cartTotal = cart->getCartTotal();

            if (cartTotal >= minOrderAmount) {
                return true;
            }

            return false;
        }

        double calculateDiscount(Cart* cart) override {
            double cartTotal = cart->getCartTotal();
            double discount = cartTotal;

            if (cartTotal >= minOrderAmount) {
                discount = cartTotal * discountPercentage / 100;
            }

            double elegibleDiscount = min(discount, maxDiscount);
            return elegibleDiscount;
        }
};

class Order {
    private:
        int orderId;
        int userId;
        int cartId;
        double totalPrice;
        double discount;
        OrderStatus orderStatus;

    public:
        Order(int userId, int cartId, double totalPrice, double discount) {
            orderId = GlobalIdGenerator::getInstance().getOrderId();
            this->userId = userId;
            this->cartId = cartId;
            this->totalPrice = totalPrice;
            this->discount = discount;
            orderStatus = OrderStatus::CREATED;
        }

        int getOrderId() {
            return orderId;
        }

        int getUserId() {
            return userId;
        }

        double getOrderTotalAmount() {
            return totalPrice;
        }

        double getOrderDiscount() {
            return discount;
        }

        double getOrderDiscountedAmount() {
            return totalPrice - discount;
        }

        OrderStatus getOrderStatus() {
            return orderStatus;
        }

        void updateOrderStatus(OrderStatus orderStatus) {
            this->orderStatus = orderStatus;
        }
};

class PaymentStrategyFactory {
    public:
        static PaymentStrategy* createPaymentStrategy(PaymentMode mode) {
            switch(mode) {
                case PaymentMode::UPI:
                    return new UPIPayment();
                case PaymentMode::CASH:
                    return new CashPayment();
                case PaymentMode::CARD:
                    return new CardPayment();

            }
        }
};

class Payment {
    private:
        int paymentId;
        int orderId;
        PaymentStatus paymentStatus;
        PaymentMode mode;
        double totalAmount;

    public:
        Payment(int orderId, PaymentMode mode, double totalAmount) {
            paymentId = GlobalIdGenerator::getInstance().getPaymentId();
            this->orderId = orderId;
            this->totalAmount = totalAmount;
            this->mode = mode;
            this->paymentStatus = PaymentStatus::PENDING;
        }

        bool makePayment() {
            PaymentStrategy* paymentStrategy = PaymentStrategyFactory::createPaymentStrategy(mode);
            bool madePaymentStatus = paymentStrategy->pay(totalAmount);

            if (madePaymentStatus) {
                paymentStatus = PaymentStatus::SUCCESS;
            } else {
                paymentStatus = PaymentStatus::FAILED;
            }

            return madePaymentStatus;
        }
};

class SearchService {
    private:
        vector<NodeInventory*>nodeInventories;
        vector<Inventory*> inventories;

    public:
        vector<NodeInventory*> searchItem(string query, Node* node) {
            vector<NodeInventory*> result;
            for(auto nodeInventory: nodeInventories) {
                if (nodeInventory->getNodeId() == node->getNodeId()) {
                    for (auto inventory : inventories){
                        if (inventory->getInventoryId() == nodeInventory->getInventoryId()) {
                            if (inventory->getName().find(query) != string::npos) {
                                result.push_back(nodeInventory);
                            }

                            break;
                        }
                    }
                }
            }

            return result;
        }

        void addNodeInventory(NodeInventory* nodeInventory) {
            nodeInventories.push_back(nodeInventory);
        }

        void addInventory(Inventory* inventory) {
            inventories.push_back(inventory);
        } 
};

class ZeptoService {
    private:
        vector<User*>users;
        vector<Node*>nodes;
        vector<Inventory*>inventories;
        SearchService* searchService;
        vector<NodeInventory*> nodeInventories;
        unordered_map<int, Cart*>userCartMap;

        ZeptoService() {
            searchService = new SearchService();
        }

        ZeptoService(ZeptoService&) = delete;
        ZeptoService& operator=(ZeptoService&) = delete;

    public: 
        static ZeptoService& getInstance() {
            static ZeptoService instance;
            return  instance;
        }

        User* addUser(string name, string email, UserRole role) {
            for(auto user: users) {
                if(user->getUserEmail() == email) {
                    cout<< "User already exists with this email" << endl;
                    return NULL;
                }
            }
            User* user = new User(name, email, role);
            users.push_back(user);
            return user;
        }

        Node* addNode(string name, string city, string pincode, string area, string state) {
            Address* address = new Address(city, area, pincode, state);
            Node* node = new Node(name, address);
            nodes.push_back(node);
            return node;
        }

        Inventory* addInventory(string name, string description, InventoryCategory category, double price, UOM uom) {
            Inventory* inventory = new Inventory(name, description, category, price, uom);
            inventories.push_back(inventory);
             searchService->addInventory(inventory);
            return inventory;
        }

        NodeInventory* addNodeInventory(Node* node, Inventory* inventory, int quantity) {
            int nodeId = node->getNodeId();
            int inventoryId = inventory->getInventoryId();
            NodeInventory* nodeInventory = new NodeInventory(nodeId, inventoryId, quantity);
            searchService->addNodeInventory(nodeInventory);
            nodeInventories.push_back(nodeInventory);
            return nodeInventory;
        }

        NodeInventory *getNodeInventoryById(int nodeInventoryId) {
            for (auto nodeInventory : nodeInventories) {
                if (nodeInventory->getNodeInventoryId() == nodeInventoryId) {
                    return nodeInventory;
                }
            }

            return nullptr;
        }

        vector<NodeInventory*> searchItem(string query, Node* node) {
            vector<NodeInventory*> searchResults = searchService->searchItem(query, node);
            return searchResults;
        }

        Cart* addItemToCart(User* user, NodeInventory* nodeInventory, double quantity) {
            if (quantity <= 0) {
                cout << "Invalid quantity" << endl;
                return nullptr;
            }

            bool reserved = nodeInventory->reserveQuantity(quantity);

            if (!reserved) {
                cout << "Insufficient inventory" << endl;
                return nullptr;
            }

            int nodeInventoryId = nodeInventory->getNodeInventoryId();
            int inventoryId = nodeInventory->getInventoryId();
            double itemPrice;

            for(auto inventory: inventories) {
                if (inventory->getInventoryId() == inventoryId) {
                    itemPrice = inventory->getPrice();
                }
            }

            Item* item = new Item(nodeInventoryId, quantity, itemPrice);
            int userId = user->getUserId();
            Cart* userCart;

            if (userCartMap.find(userId) != userCartMap.end()) {
                userCart = userCartMap[userId];
            } else {
                userCart = new Cart(userId);
                userCartMap[userId] = userCart;
            }

            userCart->addItem(item);

            cout << "Item added to Cart" << endl;
            return userCart;
        }

        Cart* removeItemFromCart(User* user, Item* item) {
            int userId = user->getUserId();
            Cart* userCart = userCartMap[userId];
            userCart->removeItem(item);
            return userCart;
        }

        Order *checkoutCart(Cart *cart) {
            vector<pair<NodeInventory *, double>> reservedItems;

            for (auto item : cart->getCartItems()) {
                NodeInventory *nodeInventory = getNodeInventoryById(item->getNodeInventoryId());

                if (nodeInventory == nullptr) {
                    return nullptr;
                }

                bool reserved = nodeInventory->reserveQuantity(item->getQuantity());

                if (!reserved) {
                    for (auto reservedItem : reservedItems) {
                        reservedItem.first->releaseQuantity(reservedItem.second);
                    }

                    cout << "Insufficient inventory" << endl;

                    return nullptr;
                }

                reservedItems.push_back({nodeInventory,item->getQuantity()});
            }

            double cartTotalAmount = cart->getCartTotal();

            Order *order = new Order(
                cart->getUserId(),
                cart->getCartId(),
                cartTotalAmount,
                0);

            cout << "Order Created Successfully" << endl;

            return order;
        }

        Payment* makePayment(Order* order, PaymentMode mode) {
            int orderId = order->getOrderId();
            double paymentAmount = order->getOrderDiscountedAmount();
            Payment* payment = new Payment(orderId, mode, paymentAmount);

            bool paymentStatus = payment->makePayment();

            if (paymentStatus) {
                order->updateOrderStatus(OrderStatus::CONFIRMED);
                int userId = order->getUserId();

                Cart* cart = userCartMap[userId];

                cart->clearCart();  
            }
        }

        void cancelOrder(Order *order) {
            if (order == nullptr) {
                return;
            }

            Cart *cart = userCartMap[order->getUserId()];

            if (cart != nullptr) {
                for (auto item : cart->getCartItems()) {

                    NodeInventory *nodeInventory = getNodeInventoryById(item->getNodeInventoryId());

                    if (nodeInventory != nullptr) {
                        nodeInventory->releaseQuantity(item->getQuantity());
                    }
                }
            }

            order->updateOrderStatus(OrderStatus::CANCELLED);
        }

        void deliverOrder(Order* order) {
            order->updateOrderStatus(OrderStatus::COMPLETED);
        }

};

int main() {
    ZeptoService& zeptoService = ZeptoService::getInstance();

    User* consumer = zeptoService.addUser("Ravindra", "ravindra@gmail.com", UserRole::CONSUMER);

    User* admin = zeptoService.addUser("Admin", "admin@gmail.com", UserRole::ADMIN);

    User* deliveryPartner = zeptoService.addUser("Ramesh", "ramesh@gmail.com", UserRole::DELIVERY_PARTNER);

    Node* node = zeptoService.addNode("sector 46", "Gurgaon", "122202", "sector 46", "Haryana");

    Inventory* banana = zeptoService.addInventory("Banana", "Fruits, Kela", InventoryCategory::FRUIT, 10, UOM::NOS);
    Inventory* mango = zeptoService.addInventory("Mango", "Fruits, aam", InventoryCategory::FRUIT, 10, UOM::KG);

    NodeInventory* bananaNodeInventory = zeptoService.addNodeInventory(node, banana, 500);
    NodeInventory* mangoNodeInventory = zeptoService.addNodeInventory(node, mango, 100);

    Cart* cart = zeptoService.addItemToCart(consumer, bananaNodeInventory, 10);
    cart = zeptoService.addItemToCart(consumer, mangoNodeInventory, 20);

    Order* order = zeptoService.checkoutCart(cart);

    Payment* payment = zeptoService.makePayment(order, PaymentMode::UPI);

    zeptoService.deliverOrder(order);
}