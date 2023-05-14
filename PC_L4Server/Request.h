#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <ostream>
#include "BitDecypher.h"
#include <string>

typedef std::vector<std::vector<int32_t>> matrix_t;

enum COMMAND_TYPES
{
    unknown = 0,
    upload = 1,
    start = 3,
    status = 4,

    r_idle = 100,
    r_data_accepted = 101,
    r_processing = 102,
    r_success = 103,
    r_error = 104
};
enum MESSAGE_MAP
{
    messageLength = 0,
    command = 4,
    rows = 8,
    columns = 12,
    min = 16,
    max = 20,
    data = 24

};

class Request
{
private:
    COMMAND_TYPES _command = unknown;
    int32_t _min = 0, _max = 0;
    matrix_t _data;
public:
    Request() {}

    Request(COMMAND_TYPES command, int32_t min, int32_t max) :
        _command(command), _min(min), _max(max)
    {}
    Request(COMMAND_TYPES command, int32_t min, int32_t max, matrix_t& data) :
        _command(command), _min(min), _max(max), _data(data)
    {}

    COMMAND_TYPES command() const { return _command; }
    int32_t getMin() const { return _min; }
    int32_t getMax() const { return _max; }
    matrix_t data() const { return _data; }

    static Request readRequest(std::vector<char> message);
    std::vector<char> to_bytes();
};

std::ostream& operator<<(std::ostream& os, Request const& m);

#define responseIdle Request(COMMAND_TYPES::r_idle, 0, 0)
#define responseDataAccepted Request(COMMAND_TYPES::r_data_accepted, 0, 0)
#define responseProcessing Request(COMMAND_TYPES::r_processing, 0, 0)
#define responseSuccess(data) Request(COMMAND_TYPES::r_success, 0, 0, data)
#define responseError Request(COMMAND_TYPES::r_error, 0, 0)