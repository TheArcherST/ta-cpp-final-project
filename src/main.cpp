#include "database.cpp"
#include "models.cpp"

using namespace std;


int main() {
    Database<OrderModel> order_database ("/Users/mihailsapovalov/Desktop/C++/ITStepProjCPP/db.txt");
    order_database.load();

    shared_ptr<OrderModel> order (new OrderModel());
    order->id = shared_ptr<int> (new int (1));

    shared_ptr<OrderModel> query_select = order_database.find_one(order);
    *query_select->status = 0;

    order_database.save();
}
