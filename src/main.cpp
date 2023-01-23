#include<unistd.h>

#include "database.cpp"
#include "models.cpp"

#define alloc_shared(typename, value) (shared_ptr<typename>) new typename (value)

using namespace std;
using namespace models;


class Console {
    vector<string> input_buffer;

public:
    void print(string text) {
        cout << text;
    }
    string input(string quote = "") {
        string buf;

        if (this->input_buffer.size()) {
            buf = this->input_buffer[0];
            this->input_buffer.erase(this->input_buffer.begin());
        } else {
            cout << quote;
            cin >> buf;
        }
        return buf;
    }
    bool approve(string quote = "", string y_literal = "y") {
        string text = this->input(quote);
        return text == y_literal;
    }
    string& operator << (string& str) {
        print(str);
        return str;
    }
    const string operator << (const string str) {
        print(str);
        return str;
    }
    
    void emit_input(string text) {
        this->input_buffer.push_back(text);
    }
};



int client_cli(Console &console, Database<OrderModel> &order_database, shared_ptr<UserModel> user) {
    
    if (!console.approve("Hi, client! Do you want to book taxi? (y/n): ")) {
        return 0;
    }

    OrderModel new_order;
    new_order.user_id = alloc_shared(int, *user->id);
    new_order.status = alloc_shared(int, OrderStatus::WAIT);

    int order_id = order_database.insert(new_order);
    order_database.save();
    
    console.print("Order is created. Waiting for acception.\n");

    OrderModel oreder_q;
    oreder_q.id = alloc_shared(int, order_id);
    
    bool is_process_reported = false;

    while (true) {
        sleep(1);
        order_database.load();
        shared_ptr<OrderModel> order (order_database.find_one(oreder_q));
        switch (*order->status)
        {
        case OrderStatus::WAIT:
            // nope
            break;
        case OrderStatus::IN_PROCESS:
            if (!is_process_reported) {
                console.print("Cabbie finds you! You'reon your way. Waiting for the end of trip...\n");
                is_process_reported = true;
            }
            break;
        case OrderStatus::DONE:
            console.print("Congratulations, you finally arrived!\n");
            return 0;
            break;
        default:
            throw runtime_error("Data model contains bad enum key (invalid order status)");
            break;
        }
    }
};


int cabbie_cli(Console &console, Database<OrderModel> &order_database, shared_ptr<UserModel> user) {
    console.print("Hello, cabbie!\n");
    console.print("Here will be displayed all orders for you. You can approve or reject them.\n\n");

    vector<int> reviewed_orders;
    bool is_new_order;

    while (true) {
        order_database.load();
        OrderModel query;
        query.status = alloc_shared(int, OrderStatus::WAIT);
        auto wait_orders = order_database.find(query);
        
        shared_ptr<OrderModel> current_order;
        
        is_new_order = false;

        for (int i = 0; i < wait_orders.size(); i++) {
            current_order = wait_orders[i];
            is_new_order = true;
            for (int rev : reviewed_orders) {
                if (rev == *current_order->id) {
                    is_new_order = false;
                    break;
                }
            }
            if (is_new_order) {
                break;
            }
        }
        
        if (!is_new_order) {
            sleep(1);
            continue;
        }
        
        reviewed_orders.push_back(*current_order->id);
        
        if (!console.approve("New order! Approve? (y/n): ")) {
            continue;
        }

        *current_order->status = OrderStatus::IN_PROCESS;
        current_order->cabbie_id = user->id;
        order_database.save();

        while (!console.approve("Approved! Input the phrase `done`, when the trip is over.\n", "done")) {};
        
        console.print("Ok, done! Search for new orders...\n");

        *current_order->status = OrderStatus::DONE;
        order_database.save();
    }
};


int admin_cli(Console &console, Database<UserModel> &user_database, Database<OrderModel> &order_database, shared_ptr<UserModel> user) {
    
    console.print("Hello, admin!\n");
    console.print("You can check database of application. Possible actions:\n\n1 - get users\n2 - get orders\n\n");
    while (true) {
        string request = console.input("> ");
        switch (request[0]) {
            case '1':
            {
                user_database.load();
                auto all_users = user_database.find(UserModel());
                for (auto user : all_users) {
                    console.print(join_csv_line(user->dump()));
                    console.print("\n");
                }
                break;
            }
            case '2':
            {
                order_database.load();
                auto all_orders = order_database.find(OrderModel());
                for (auto order : all_orders) {
                    console.print(join_csv_line(order->dump()));
                    console.print("\n");
                };
                break;
            }
            default:
            {
                console.print("Unknown literal");
                break;
            }
        }
    }
};


int main() {
    
    Console console;

    Database<UserModel> user_database ("/Users/mihailsapovalov/Desktop/C++/ITStepProjCPP/user.csv");
    
    try {
        user_database.load();
    } catch (const runtime_error& ex) {
        console.print("Error, while loading user database: ");
        console.print(ex.what());
        console.print("\n");
        return -1;
    }

    Database<OrderModel> order_database ("/Users/mihailsapovalov/Desktop/C++/ITStepProjCPP/order.csv");
    
    try {
        order_database.load();
    } catch (const runtime_error &ex) {
        console.print("Error, while loading order database: ");
        console.print(ex.what());
        console.print("\n");
        return -1;
    }

    string login = console.input("Enter login:\n");
    string password = console.input("Enter password:\n");

    shared_ptr<UserModel> user_q (new UserModel());
    user_q->login = shared_ptr<string> (new string (login));

    shared_ptr<UserModel> user;

    try {
        user = user_database.find_one(user_q);
    } catch (const runtime_error &ex) {
        console.print("User not found");
        return -1;
    }

    if (*user->password != password) {
        console.print("Access denied");
        return -1;
    }
    
    switch (*user->role)
    {
    case UserRole::CLIENT:
        client_cli(console, order_database, user);
        break;
    case UserRole::CABBIE:
        cabbie_cli(console, order_database, user);
        break;
    case UserRole::ADMIN:
        admin_cli(console, user_database, order_database, user);
        break;
    
    default:
        break; 
    }
}
