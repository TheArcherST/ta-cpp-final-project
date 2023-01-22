#include "database.h"

using namespace std;

enum UserRole {
    CLIENT, CABBIE, ADMIN
};


struct UserModel : public BaseModel {
    shared_ptr<string> username;
    shared_ptr<string> password;
    shared_ptr<int> role;

    UserModel() : BaseModel(), username(nullptr), password(nullptr), role(nullptr) {};

    bool match_query(shared_ptr<UserModel> query) {
        return (
            BaseModel::match_query(query)
            && ((query->username == nullptr) || (*query->username == *this->username))
            && ((query->password == nullptr) || (*query->password == *this->password))
            && ((query->role == nullptr) || (*query->role == *this->role))
            );
    }

    void parse (const vector<string> data, int& __pos) {
        BaseModel::parse(data, __pos);
        username = shared_ptr<string> (new string (data[__pos++]));
        password = shared_ptr<string> (new string (data[__pos++]));
        role = shared_ptr<int> (new int (stoi(data[__pos++])));
    }

    vector<string> dump() {
        vector<string> result = BaseModel::dump();
        result.push_back(*this->username);
        result.push_back(*this->password);
        result.push_back(to_string(*this->role));
        return result;
    }
};


enum OrderStatus {
    WAIT, IN_PROCESS, DONE 
};


struct OrderModel : public BaseModel {
    shared_ptr<int> user_id;
    shared_ptr<int> status;

    OrderModel() : BaseModel(), user_id(nullptr), status(nullptr) {};

    bool match_query(const shared_ptr<OrderModel> query)
    {
        return (
            BaseModel::match_query(query)
            && ((query->user_id == nullptr) || (*query->user_id == *this->user_id))
            && ((query->status == nullptr) || (*query->status == *this->status))
            );
    }

    void parse (const vector<string> data, int& __pos)
    {
        BaseModel::parse(data, __pos);
        user_id = shared_ptr<int> (new int (stoi(data[__pos++])));
        status = shared_ptr<int> (new int (stoi(data[__pos++])));
    };
    
    vector<string> dump() {
        vector<string> result = BaseModel::dump();
        result.push_back(to_string(*this->user_id));
        result.push_back(to_string(*this->status));
        return result;
    };
};
