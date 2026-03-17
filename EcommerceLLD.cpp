#include<iostream>
#include<vector>
#include<unordered_map>
#include<memory>
#include<mutex>

using namespace std;

enum OrderStatus { CREATED, PAID, FAILED };

class User {
   private:
      int userId;
      string name;
      string location;
   
   public: 
      User(int id, string name, string loc) 
         : userId(id), name(name), location(loc) {}
      
      int getUserId() {
         return userId;
      }

      string getName() {
         return name;
      }

      string getLocation() {
         return location;
      }
};

class Product {
   private:
     int productId;
     string name;
     double price;
   
   public: 
      Product(int id, string name, double price) 
         : productId(id), name(name), price(price) {}
      
      int getId() {
         return productId;
      }

      string getName() {
         return name;
      }

      double getPrice() {
         return price;
      }  
};

class Node {
   private:
      int nodeId;
      string location;
   
   public: 
      Node(int id, string loc) 
         : nodeId(id), location(loc) {}
      
      int getId() {
         return nodeId;
      }

      string getLocation() {
         return location;
      }    
};

class NodeInventory {
   private:
      Node* node;
      Product* product;
      int stock;
      mutex mtx;
   
   public:
      NodeInventory(Node* node, Product* product, int stock) 
         : node(node), product(product), stock(stock) {}
      
      bool reserveStock(int qty) {
         lock_guard<mutex>lock(mtx);
         if (stock >= qty) {
            stock -= qty;
            return true;
         }
         return false;
      }

      void addStock(int qty) {
         lock_guard<mutex>lock(mtx);
         stock += qty;
      }

      Product* getProduct() {
         return product;
      }
};

class CartItem {
   private:
      Product* product;
      int quantity;
   
   public:
      CartItem(Product* p, int qty) 
         : product(p), quantity(qty) {}
      
      Product* getProduct() {
         return product;
      }

      int getQuantity() {
         return quantity;
      }

      double getTotalPrice() {
         return product->getPrice() * quantity;
      }
};

class Cart {
   private:
      vector<CartItem*>items;
      User* user;
   
   public:
      Cart(User* user) 
         : user(user) {}
      
      Cart(User* user, Product* product, int qty) {
         this->user = user;
         CartItem* cartItem = new CartItem(product, qty);
         items.push_back(cartItem);
      }

      void addItem(Product* product, int qty) {
         CartItem* cartItem = new CartItem(product, qty);
         items.push_back(cartItem);
      }
      
      vector<CartItem*>getItems() {
         return items;
      }

      double getTotalAmount() {
         double total = 0;
         for (auto item: items) {
            total += item->getTotalPrice();
         }
         return total;
      }
};

class OrderItem {
   private:
      int productId;
      string productName;
      double priceAtPurchase;
      int quantity;
   
   public: 
      OrderItem(Product* product, int qty) 
         : productId(product->getId()),
           productName(product->getName()),
           priceAtPurchase(product->getPrice()),
           quantity(qty) {}
      
      double getTotal() {
         return priceAtPurchase * quantity;
      }
};

class Order {
   private:
      int orderId;
      User* user;
      vector<OrderItem*>items;
      OrderStatus status;
      double totalAmount;
   
   public:
      Order(int id, User* user) 
         : orderId(id), user(user), status(CREATED), totalAmount(0) {}
      
      void addItem(OrderItem* item) {
         items.push_back(item);
         totalAmount += item->getTotal();
      }

      double getTotalAmount() {
         return totalAmount;
      }

      void setStatus(OrderStatus s) {
         status = s;
      }

      int getUserId() {
         return user->getUserId();
      }
};

class PaymentStrategy {
   public: 
      virtual bool pay(double amount) = 0;
      virtual ~PaymentStrategy() {}
};

class CardPayment : public PaymentStrategy {
   public:
      bool pay(double amount) override {
         cout << "Paid " << amount << " via Card\n";
         return true;
      }
};

class UPIPayment : public PaymentStrategy {
   public: 
      bool pay(double amount) override {
         cout << "Paid " << amount << " via UPI\n";
         return true;
      }
};

class PaymentFactory {
   public: 
      static PaymentStrategy* createPayment(string type) {
         if (type == "CARD") 
            return new CardPayment();
         else if (type == "UPI")
            return new UPIPayment();
         return nullptr;
      }
};

class Observer {
   public:
      virtual void update(string msg) = 0;
};

class EmailNotification : public Observer {
   public: 
      void update(string msg) override {
         cout << "[Email] " << msg << endl;
      }
};

class NotificationService {
   private:
      vector<Observer*>observers;
   
   public:
      void addObserver(Observer* obs) {
         observers.push_back(obs);
      }

      void notifyAll(string msg) {
         for (auto obs: observers) {
            obs->update(msg);
         }
      }
};

class EcommerceService {
   private:
      static EcommerceService* instance;
      NotificationService notificationService;
      int orderCounter;
      int nodeCounter;
      int userCounter;
      int cartCounter;


      EcommerceService() {
         orderCounter = 1;
         nodeCounter = 1;
         userCounter = 1;
         cartCounter = 1;
      }
   
   public: 
      static EcommerceService* getInstance() {
         if (!instance) {
            instance = new EcommerceService();
         }
         return instance;
      }

      NotificationService* getNotificationService() {
         return &notificationService;
      }
};

EcommerceService* EcommerceService :: instance = nullptr;

int main() {
   EcommerceService* service = EcommerceService::getInstance();

   User* user1 = new User(1, "Rahul", "Bangalore");

   Product* laptop = new Product(101, "Laptop", 50000);
   Product* phone  = new Product(102, "Phone", 20000);

   Node* node1 = new Node(1, "Bangalore Warehouse");

   NodeInventory* inv1 = new NodeInventory(node1, laptop, 5);
   NodeInventory* inv2 = new NodeInventory(node1, phone, 10);

   Cart* cart = new Cart(user1);
   cart->addItem(laptop, 1);
   cart->addItem(phone, 2);

   cout << "Cart Total: " << cart->getTotalAmount() << endl;

   bool stockAvailable = true;

   for (auto item : cart->getItems()) {

      Product *product = item->getProduct();
      int qty = item->getQuantity();

      bool reserved = false;

      if (product->getId() == laptop->getId()) {
         reserved = inv1->reserveStock(qty);
      }
      else if (product->getId() == phone->getId()) {
         reserved = inv2->reserveStock(qty);
      }

      if (!reserved){
         stockAvailable = false;
         cout << "Stock not available for product: " << product->getName() << endl;
         break;
      }
   }

   if (!stockAvailable) {
      cout << "Order Failed due to stock issue\n";
      return 0;
   }

   Order* order = new Order(1, user1);

   for (auto item : cart->getItems()) {
      OrderItem *orderItem = new OrderItem(item->getProduct(),item->getQuantity());
      order->addItem(orderItem);
   }

   cout << "Order Created. Total: " << order->getTotalAmount() << endl;

   PaymentStrategy* payment = PaymentFactory::createPayment("CARD");

   bool paymentSuccess = payment->pay(order->getTotalAmount());

   if (paymentSuccess) {
      order->setStatus(PAID);
      cout << "Payment Successful\n";
   } else {
      order->setStatus(FAILED);
      cout << "Payment Failed\n";
   }
    
   delete payment;

   EmailNotification* emailObs = new EmailNotification();

   service->getNotificationService()->addObserver(emailObs);

   service->getNotificationService()->notifyAll("Order placed successfully!");
}