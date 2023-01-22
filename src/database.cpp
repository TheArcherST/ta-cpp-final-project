#include "database.h"


vector<string> split_csv_line(string line) {
    vector<string> result;
    string curr;

    for (char ch : line) {
        switch (ch)
        {
        case ',':
            result.push_back(curr);
            curr.clear();
            break;
        case '\n':
            // nope
            break;
        default:
            curr += ch;
            break;
        }
    }
    result.push_back(curr);
    return result;
}


string join_csv_line(vector<string> data) {
    string result;

    for (int i = 0; i < data.size(); i++) {
        result += data[i] + ",";
    }

    return result;
}


void BaseModel::parse(const vector<string> data, int& __pos)
{
    id = shared_ptr<int> (new int (stoi(data[__pos++])));
}

bool BaseModel::match_query(const shared_ptr<BaseModel> query)
{
    return (query->id == nullptr) || (*query->id == *this->id);
}

vector<string> BaseModel::dump() {
    vector<string> result;
    result.push_back(to_string(*this->id));
    return result;
}


template <class T>
Database<T>::Database(string filepath) {
    this->filepath = filepath;
    this->is_loaded = false;
}


template <class T>
shared_ptr<T> Database<T>::find_one(shared_ptr<T> query) {
    auto result = find(query);
    if (!result.size()) {
        throw runtime_error("Not found");
    } else {
        return result[0];
    }
}


template <class T>
vector< shared_ptr<T> > Database<T>::find(shared_ptr<T> query, bool one) {
    if (!this->is_loaded) throw runtime_error("Find request to the unloaded database");

    vector<shared_ptr<T> > result;

    for (shared_ptr<T> current : data) {

        if (current->match_query(query)) {
            result.push_back(current);

            if (one) return result;
        }
    }
    return result;
}

template <class T>
void Database<T>::load() {
    fstream stream (this->filepath);

    if (!stream.is_open()) {
        throw runtime_error("Can't open database file");
    }

    string buffer;

    while (getline(stream, buffer)) {
        vector<string> data = split_csv_line(buffer);

        shared_ptr<T> model (new T());

        int pos = 0;
        model->parse(data, pos);

        this->data.push_back(model);
    }

    this->is_loaded = true;
}


template <class T>
void Database<T>::save() {
    fstream stream (this->filepath);

    if (!stream.is_open()) {
        throw runtime_error("Can't open database file");
    }

    for (shared_ptr<T> current : this->data) {
        stream << join_csv_line(current->dump()) << "\n";
    }
}
