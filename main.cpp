#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <exception>
#include <unistd.h>
#include <vector>

#define SERVER_HOST "185.104.248.207"
#define SERVER_PORT 8080


namespace TaxiProtocol {
    enum MessageType {
        authorize_request          = 0x00,
        new_order_request          = 0x01,
        get_orderStatus_request    = 0x02,
        get_orders_request         = 0x03,
        accept_order_request       = 0x04,
        authorize_response         = 0xFF,
        new_order_response         = 0xFE,
        get_orderStatus_response   = 0xFD,
        get_orders_response        = 0xFC,
        accept_order_resopnse      = 0xFB,
    };

    enum ArgumentType {
        char_                      = 0x00,
        uint16_t_                  = 0x01,
        string_                    = 0x02,
    };

    class BaseModel {
    protected:
        int _length = 0xFF;
        int _deserialization_progress = 0;

    public:
        /**
         * @brief Serialize method
         * 
         * Serialize this structure to the vector of bytes.
         * 
         * @throws 
         * @return std::vector<char>&
         */
        virtual std::vector<char>* serialize() = 0;

        /**
         * @brief Partial deserialization method
         * 
         * Method that apply vector of bytes to the this structure.
         * You can provide data by parts. To check is structure
         * Filled, you can call method `is_filled`. Do not access to
         * fields before calling it.
         * 
         * @param data The vector of bytes
         */
        virtual void fill_bytes(char* data, int count, int offset=0) = 0;

        /**
         * @brief Deserialization progress check method
         * 
         * @return true if deserialization if finished 
         * @return false if deserialization if not finished
         */
        bool is_filled() {
            return !this->lost_bytes();
        }

        /**
         * @brief Deserialization progress check method
         * 
         * @return int How many bytes need is filled due deserealization
         */
        int filled_bytes() {
            return this->_deserialization_progress;
        }

        /**
         * @brief Deserialization progress check method
         * 
         * Checks how many bytes need to finish deserialization.
         * 
         * @return int The bytes, needs to fully initialize the model.
         */
        int lost_bytes() {
            return this->_length - this->_deserialization_progress;
        }
    };


    struct Argument : public BaseModel {
        char length = 0xFF;
        char type;
        std::vector<char> data;

    public:
        Argument() = default;
        Argument(char* data, int count) {
            this->fill_bytes(data, count);

            if (!this->is_filled()) {
                throw std::invalid_argument("Parial initilization in constructor is not allowed");
            }
        }

        std::vector<char>* serialize() {
            if (!this->is_filled()) {
                throw std::logic_error("Serializing of not fully filled models is not allowed");
            }

            auto result = new std::vector<char>(this->_length);
            auto current = result->begin();

            result->insert(current++, this->length);
            result->insert(current++, this->type);
            result->insert(current++, this->data.begin(), this->data.end());

            return result;
        }

        void fill_bytes(char* data, int count, int offset=0) {
            int &current = this->_deserialization_progress;

            while (current < this->_length && (current+offset) < count) {
                switch (current)
                {
                case 0:
                    this->length = data[current++ + offset];
                    this->_length = this->length;
                    break;
                case 1:
                    this->type = data[current++ + offset];
                    break;
                default:
                    for (; current < this->_length && (current+offset) < count; current++) {
                        this->data[current-2] = data[current + offset];
                    }
                    break;
                }
            }
        }
    };

    struct Message : public BaseModel {
        char length;
        char type;
        std::vector<struct Argument> arguments;

        std::vector<char>* serialize() {
            if (!this->is_filled()) {
                throw std::logic_error("Serializing of not fully filled models is not allowed");
            }

            std::vector<char>* result = new std::vector<char>(this->_length);

            (*result)[0] = this->length;
            (*result)[1] = this->type;
            int offset = 2;
            for (auto argument : this->arguments) {
                auto ser_arg = argument.serialize();
                result->insert(result->begin() + offset, ser_arg->begin(), ser_arg->end());
            }

            return result;
        }

        void fill_bytes(char* data, int count, int offset=0) {
            int &current = this->_deserialization_progress;
            while ((current < this->_length) && ((current + offset) < count)) {
                switch (current)
                {
                case 0:
                    this->length = data[current++ + offset];
                    this->_length = this->length;
                    break;
                case 1:
                    this->type = data[current++ + offset];
                    break;
                default:
                    while ((current < this->_length) && ((current + offset) < count)) {
                        Argument arg;
                        arg.fill_bytes(&data[0], count, current+offset);
                        this->arguments.push_back(arg);
                        current += arg.filled_bytes();
                    }
                }
            }
        }
    };
};




namespace TaxiHelper {
    using namespace TaxiProtocol;

    class Client {
        struct sockaddr_in server_address;
        int sockid;
        int client_fd;
        bool is_connected;

    public:
        
        /**
         * Client constructor
         * Initialize client: allocate a socket for future connections, normalize input host and port of the server.
         * :throws: std::runtime_error on socket allocating error
         * :throws: std::invalid_argument if host is invalid
         */
        Client (const char* host, int port) {

            if ( // try to create new socket
                (this->sockid = socket(AF_INET, SOCK_STREAM, 0)) < 0
                )
            {
                throw std::runtime_error("Socket creation failed");
            }

            if ( // try to convert IP address to binary format
                inet_pton(AF_INET, host, &this->server_address.sin_addr) <= 0
                )
            {
                throw std::invalid_argument("Invalid argument `char* host`");
            }
            
            this->server_address.sin_family = AF_INET;
            this->server_address.sin_port = htons(port);  // ::port bytes order changed from host to network

            this->is_connected = false;
        }
        
        ~Client() {
            this->close();
        }

        /**
         * Connnect method
         * Connects client to the server, if client not connected yet.
         * :throws: std::runtime_error: on connection failure
         */
        void connect() {
            if (this->is_connected) {
                return;
            } else {
                this->client_fd = ::connect(sockid, (struct sockaddr*)&this->server_address, sizeof(this->server_address));

                if (this->client_fd < 0) {
                    throw std::runtime_error("Connection failed");
                } else {
                    this->is_connected = true;
                }
            }
        }
        
        /**
         * Close method
         * Close connection, if connected
         */
        void close() {
            if (this->is_connected) {
                ::close(this->client_fd);
            }
        }

        /**
         * Make request method
         * Method sends request to the server, and waits for server response.
         *
         * :raises: std::logic_error, If client is not connected to the server.
         */
        Message* request(struct Message& request) {
            if (this->is_connected) {
                throw std::logic_error("Client not connected");
            }

            auto data = request.serialize();
            send(this->sockid, data, data->size(), 0);

            const int buffer_size = 1024;
            char buffer[buffer_size];

            Message* response = new Message();
            while (!response->is_filled()) {
                int total_bytes = read(this->sockid, &buffer, buffer_size);
                response->fill_bytes(buffer, total_bytes);
            }
            return response;
        }
    };
};


void test() {
    TaxiHelper::Client client (SERVER_HOST, SERVER_PORT);
    client.connect();

    TaxiProtocol::Message msg;

    char data[6] = {6, 0, 2, 5, 0, 'a'};
    msg.fill_bytes(&data[0], 6);
    std::cout << (int)(msg.length) << std::endl;
    std::cout << (int)(msg.arguments[0].length) << std::endl;
    std::cout << msg.is_filled() << msg.arguments[0].is_filled() << std::endl;
}


int main(void)
{
    test();
}
