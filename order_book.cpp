#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

class order_book {
  public:
    // order types and sides
    enum class order_type {
        market,
        limit,
        good_till_canceled,
        for_or_kill_limit
    };
    enum class side { buy, sell };

    // order class representing individual orders
    class order {
      public:
        // default constructor
        order(int id, order_type type, side _side, double price, int quantity)
            : id(id), type(type), _side(_side), price(price),
              quantity(quantity) {}

        // getter functions for order attributes
        int get_id() const { return id; }
        order_type get_type() const { return type; }
        side get_side() const { return _side; }
        double get_price() const { return price; }
        int get_quantity() const { return quantity; }

        // to update order quantity
        void set_quantity(int new_quantity) { quantity = new_quantity; }

      private:
        int id;
        order_type type;
        side _side;
        double price;
        int quantity;
    };

    // to add an order to the order book
    void add_order(const order& order) { orders.push_back(order); }

    // to cancel an order in the order book
    void cancel_order(int order_id) {
        // remove_if moves all the elements for which it returns true to the end
        // of the vector and returns an iterator to the new end of the vector
        auto it = std::remove_if(orders.begin(), orders.end(),
                                 [order_id](const order &order) {
                                     return order.get_id() == order_id;
                                 });
        if (it != orders.end()) {
            std::cout << "Canceled Order ID : " << order_id << std::endl;
            orders.erase(it, orders.end());
        }
    }

    // to match orders in the order book
    void match_orders() {
        // first handle the market orders
        for (auto it = orders.begin(); it != orders.end();) {
            if (it->get_type() == order_type::market) {
                auto match_it = find_match(it, it->get_quantity());
                if (match_it != orders.end()) {
                    execute_order(it, match_it);
                    it = orders.erase(it);
                } else {
                    ++it;
                }
            } else {
                ++it;
            }
        }

        // Second, handle good till canceled orders
        for (auto it = orders.begin(); it != orders.end();) {
            if (it->get_type() == order_type::good_till_canceled) {
                auto match_it = find_match(it, it->get_quantity());
                if (match_it != orders.end()) {
                    execute_order(it, match_it);
                    it = orders.erase(it);
                } else {
                    ++it;
                }
            } else {
                ++it;
            }
        }

        // Finally, handle Limit orders that were not matched by Market or GTC
        // orders
        for (auto it = orders.begin(); it != orders.end();) {
            if (it->get_type() == order_type::limit) {
                auto match_it = find_match(it, it->get_quantity());
                if (match_it != orders.end()) {
                    execute_order(it, match_it);
                    it = orders.erase(it);
                } else {
                    ++it;
                }
            } else {
                ++it;
            }
        }
    }

    // to print all orders in the order book
    void print_orders() {
        for (auto &order : orders) {
            print_order(order);
        }
    }

  private:
    std::vector<order> orders;

    // helper function to find a match for the given order
    std::vector<order>::iterator
    
    find_match(std::vector<order>::iterator& order_it, int quantity, bool full_match = false) {
        
        for (auto it = orders.begin(); it != orders.end(); ++it) {
            if (it->get_side() != order_it->get_side() &&
                ((order_it->get_side() == side::buy && it->get_price() <= order_it->get_price()) ||
                 (order_it->get_side() == side::sell && it->get_price() >= order_it->get_price())) &&
                 (!full_match || it->get_quantity() >= quantity)) {
                return it;
            }
        }
        return orders.end();
    }

    // helper function to execute an order
    void execute_order(std::vector<order>::iterator& order_it, std::vector<order>::iterator& match_it){
        double fill_price = match_it->get_price();
        std::cout << "Matched Order ID: " << order_it->get_id() << " with Order ID: " << match_it->get_id() << " at Price: " << std::fixed << std::setprecision(2) << fill_price << " quantity: " << order_it->get_quantity() << std::endl;
        match_it->set_quantity(match_it->get_quantity() - order_it->get_quantity());
        if(match_it->get_quantity()==0){
            orders.erase(match_it);
        }
    }

    // helper function to print an individual order
    void print_order(const order& order) const {
        std::cout << "Order ID: " << order.get_id() 
                  << ", Type: " << static_cast<int>(order.get_type()) 
                  << ", Side: " << (order.get_side() == side::buy ? "Buy" : "Sell")
                  << ", Price: " << order.get_price() 
                  << ", Quantity: " << order.get_quantity() << std::endl;
    }

};


int main() {
    order_book ob;

    // initial limit orders 
    ob.add_order(order_book::order(1, order_book::order_type::limit, order_book::side::sell, 101.0, 100));  // ask 101 x100
    ob.add_order(order_book::order(2, order_book::order_type::limit, order_book::side::sell, 102.0, 50));   // ask 102 x50
    ob.add_order(order_book::order(3, order_book::order_type::limit, order_book::side::buy,  99.0, 120));   // bid 99 x120
    ob.add_order(order_book::order(4, order_book::order_type::limit, order_book::side::buy,  98.5, 80));    // bid 98.5 x80

    std::cout << "\n--- After adding initial limit orders ---\n";
    ob.print_orders();

    // market buy that should match best sell (101)
    ob.add_order(order_book::order(5, order_book::order_type::market, order_book::side::buy, 0.0, 60));
    // market sell that should match best buy (99)
    ob.add_order(order_book::order(6, order_book::order_type::market, order_book::side::sell, 0.0, 50));

    std::cout << "\n--- After adding market buy/sell orders ---\n";
    ob.print_orders();

    std::cout << "\n--- Matching market orders ---\n";
    ob.match_orders();  // should execute order 5 vs 6, and 6 vs 3
    std::cout << "\n--- Order book after matching market orders ---\n";
    ob.print_orders();

    // good till canceled orders that can match if prices allow
    ob.add_order(order_book::order(7, order_book::order_type::good_till_canceled, order_book::side::buy, 100.5, 40));
    ob.add_order(order_book::order(8, order_book::order_type::good_till_canceled, order_book::side::sell, 98.8, 70));

    std::cout << "\n--- After adding good till cancled buy/sell ---\n";
    ob.print_orders();

    std::cout << "\n--- Matching good till canceled orders ---\n";
    ob.match_orders();
    std::cout << "\n--- Order book after matching GTC ---\n";
    ob.print_orders();

    // limit orders that could cross
    ob.add_order(order_book::order(9,  order_book::order_type::limit, order_book::side::buy, 101.0, 30));
    ob.add_order(order_book::order(10, order_book::order_type::limit, order_book::side::sell, 99.0, 30));

    std::cout << "\n--- After adding crossing limit orders ---\n";
    ob.print_orders();

    std::cout << "\n--- Matching remaining limit orders ---\n";
    ob.match_orders();
    std::cout << "\n--- Order book after matching remaining limits ---\n";
    ob.print_orders();

    // cancel an existing order (if still present)
    std::cout << "\n--- Cancel order id 2 (if present) ---\n";
    ob.cancel_order(2);
    std::cout << "\n--- Final order book ---\n";
    ob.print_orders();

    return 0;
}
