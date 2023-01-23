#include "database.h"

#define macro_query_match(field_name) && ((query->field_name == nullptr) || ((this->field_name == nullptr) ? false : *query->field_name == *this->field_name))
#define macro_parse(field_name, cast_type, cast_function) this->field_name = (data[__pos++].length()) ? (shared_ptr<cast_type>) new cast_type (cast_function(data[__pos-1])) : nullptr;
#define macro_parse_raw(field_name) this->field_name = (data[__pos++].length()) ? shared_ptr<string> (new string (data[__pos-1])) : nullptr;
#define macro_dump(field_name) result.push_back((this->field_name != nullptr) ? to_string(*this->field_name) : "");
#define macro_dump_raw(field_name) result.push_back((this->field_name != nullptr) ? *this->field_name : "");

using namespace std;


namespace models {

enum UserRole {
    CLIENT, CABBIE, ADMIN
};


struct UserModel : public BaseModel {
    shared_ptr<string> login;
    shared_ptr<string> password;
    shared_ptr<int> role;

    UserModel() : BaseModel(), login(nullptr), password(nullptr), role(nullptr) {};

    bool match_query(shared_ptr<UserModel> query) {
        return (
        BaseModel::match_query(query)
        macro_query_match(login)
        macro_query_match(password)
        macro_query_match(role)
        );
    }

    void parse (const vector<string> data, int& __pos) {
        BaseModel::parse(data, __pos);
        macro_parse_raw(login)
        macro_parse_raw(password)
        macro_parse(role, int, stoi)
    }

    vector<string> dump() {
        vector<string> result = BaseModel::dump();
        macro_dump_raw(login)
        macro_dump_raw(password);
        macro_dump(role);
        return result;
    }
};


enum OrderStatus {
    WAIT = 0,
    IN_PROCESS = 1,
    DONE = 2
};


struct OrderModel : public BaseModel {
    shared_ptr<int> user_id;
    shared_ptr<int> cabbie_id;
    shared_ptr<int> status;

    OrderModel() : BaseModel(), user_id(nullptr), cabbie_id(nullptr), status(nullptr) {};

    bool match_query(const shared_ptr<OrderModel> query)
    {
        return (
        BaseModel::match_query(query)
        macro_query_match(user_id)
        macro_query_match(cabbie_id)
        macro_query_match(status)
        );
    }

    void parse (const vector<string> data, int& __pos)
    {
        BaseModel::parse(data, __pos);
        macro_parse(user_id, int, stoi)
        macro_parse(cabbie_id, int, stoi)
        macro_parse(status, int, stoi)
    };
    
    vector<string> dump() {
        vector<string> result = BaseModel::dump();
        macro_dump(user_id)
        macro_dump(cabbie_id)
        macro_dump(status)
        return result;
    };
};

}
