#include<unistd.h>

#include "database.cpp"
#include "models.cpp"

#define alloc_shared(typename, value) (shared_ptr<typename>) new typename (value)

using namespace std;
using namespace models;


int client_cli(Database<OrderModel> &order_database, shared_ptr<UserModel> user) {
    cout << "Hi, client! Do you want to book taxi? (y/n): ";
    string buf;
    cin >> buf;

    if (buf ==  "y") {
        OrderModel new_order;
        new_order.user_id = alloc_shared(int, *user->id);
        new_order.status = alloc_shared(int, OrderStatus::WAIT);

        int order_id = order_database.insert(new_order);
        order_database.save();

        cout << "Order is created. Waiting for acception." << endl;

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
                    cout << "Cabbie finds you! You'reon your way. Waiting for the end of trip..." << endl;
                    is_process_reported = true;
                }
                break;
            case OrderStatus::DONE:
                cout << "Congratulations, you finally arrived! " << endl;
                    return 0;
                break;
            default:
                throw runtime_error("Data model contains bad enum key (invalid order status)");
                break;
            }
        }
    } else {
        return 0;
    }
};


int cabbie_cli(Database<OrderModel> &order_database, shared_ptr<UserModel> user) {
    cout << "Hello, cabbie!" << endl;
    cout << "Here will be displayed all orders for you. You can approve or reject them." << endl << endl;

    vector<int> reviewed_orders;
    bool flag;

    while (true) {
        sleep(1);
        order_database.load();
        OrderModel query;
        query.status = alloc_shared(int, OrderStatus::WAIT);
        auto all_orders = order_database.find(query);

        for (auto order : all_orders) {
            flag = true;
            for (int rev : reviewed_orders) {
                if (rev == *order->id) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                reviewed_orders.push_back(*order->id);
                cout << "New order! Approve? (y/n): ";
                string buf;
                cin >> buf;

                if (buf ==  "y") {
                    cout << "Approved! Input the phrase `done`, when the trip is over." << endl;
                    *order->status = OrderStatus::IN_PROCESS;
                    order->cabbie_id = user->id;
                    order_database.save();

                    while (true){
                        buf.clear();
                        cin >> buf;
                        if (buf == "done") {
                            break;
                        } else {
                            cout << "what?" << endl; 
                        }
                    }
                    cout << "Ok, done! Search for new orders..." << endl << endl; 

                    *order->status = OrderStatus::DONE;
                    order_database.save();

                }
            }
        }
    }
};


int admin_cli(shared_ptr<UserModel> user) {

};


int main() {

    Database<UserModel> user_database ("/Users/mihailsapovalov/Desktop/C++/ITStepProjCPP/user.csv");
    
    try {
        user_database.load();
    } catch (runtime_error) {
        cout << "User database file not found" << endl;
        return -1;
    }

    Database<OrderModel> order_database ("/Users/mihailsapovalov/Desktop/C++/ITStepProjCPP/order.csv");
    
    try {
        order_database.load();
    } catch (runtime_error) {
        cout << "Order database file not found" << endl;
        return -1;
    }

    string login;
    string password;

    cout << "Enter login: " << endl;
    cin >> login;

    cout << "Enter password: " << endl;
    cin >> password;

    shared_ptr<UserModel> user_q (new UserModel());
    user_q->login = shared_ptr<string> (new string (login));

    shared_ptr<UserModel> user;

    try {
        user = user_database.find_one(user_q);
    } catch (runtime_error) {
        cout << "User not found" << endl;
        return -1;
    }

    if (*user->password != password) {
        cout << "Access denied" << endl;
        return -1;
    }

    switch (*user->role)
    {
    case UserRole::CLIENT:
        client_cli(order_database, user);
        break;
    case UserRole::CABBIE:
        cabbie_cli(order_database, user);
        break;
    case UserRole::ADMIN:
        admin_cli(user);
        break;
    
    default:
        break; 
    }
}
