#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;


vector<string> split_csv_line(string line);

string join_csv_line(vector<string> data);

struct BaseModel {
    shared_ptr<int> id;
    BaseModel();

    bool match_query(const shared_ptr<BaseModel> query);
    void parse(const vector<string> data,  int& __pos);
    vector<string> dump();
};


/**
 * @brief The base database class
 * 
 * Use this class to make simple file databases, oreinted on deveeloper convenient.
 * The load method will load all data from CSV file. You can find specified values by builtin
 * find method (fill fields match), or access to the data by yourself by calling method `getData`.
 * 
 * After data change, you can just call method `save`, all changes will be written into the source file.
 * 
 */
template <class T>
class Database {
    static_assert(is_base_of<BaseModel, T>::value, "T must derive from BaseModel");

private:
    string filepath;
    vector<shared_ptr<T> > data;
    bool is_loaded;

public:
    /**
     * @brief Construct a new Database object
     * 
     * @param filepath - path to the database file. supports only csv format.
     */
    Database(string filepath);

    /**
     * @brief The find model method
     * 
     * @param query - the model object, with not fully initialied fields. find protocol
     *        check for all initialized fields on full match with source models.
     * @param one - of `true`, break search after first match.
     * @return vector< shared_ptr<T> > - the search result.
     */
    vector< shared_ptr<T> > find(shared_ptr<T> query, bool one = false);
    vector< shared_ptr<T> > find(const T query, bool one = false);

    /**
     * @brief The find_one model method
     * 
     * Actually, a spercial case of method `find`.
     * 
     * @param query - the model object, with not fully initialied fields. find protocol
     *        check for all initialized fields on full match with source models. 
     * @throw value_error - if object not found
     * @return shared_ptr<T> 
     */
    shared_ptr<T> find_one(shared_ptr<T> query);
    shared_ptr<T> find_one(const T query);

    /**
     * @brief The insert model method
     * 
     * Inserts the model in database. If model's standard `id` field is empty,
     * identifier will be evaluated automatically.
     * 
     * @param model - the model to insert
     * @return int - the identifier of inserted model
     */
    int insert(shared_ptr<T> model);
    int insert(const T model);

    /**
     * @brief The load data method
     * 
     * Method loads all data from file, passed into constructor.
     * You can access to the data via method `getData`.
     */
    void load();

    /**
     * @brief The save data method
     * 
     * Method saves the current state of the data into file, passed into constructor.
     */
    void save();
};
