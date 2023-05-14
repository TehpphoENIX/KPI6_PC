#include "Request.h"

Request Request::readRequest(std::vector<char> message)
{
    uint32_t command;
    bitsToInt(command, (unsigned char*) message.data() + MESSAGE_MAP::command, false);
    uint32_t rows, columns;
    bitsToInt(rows, (unsigned char*)message.data() + MESSAGE_MAP::rows, false);
    bitsToInt(columns, (unsigned char*)message.data() + MESSAGE_MAP::columns, false);
    int32_t min, max;
    bitsToInt(min, (unsigned char*)message.data() + MESSAGE_MAP::min, false);
    bitsToInt(max, (unsigned char*)message.data() + MESSAGE_MAP::max, false);

    Request req;
    req._command = static_cast<COMMAND_TYPES>(command);
    req._max = max;
    req._min = min;
    req._data = std::vector<std::vector<int32_t>>(rows, std::vector<int32_t>(columns, 0));

    int itterator = 0;
    for (uint32_t i = 0; i < rows; i++)
    {
        for (uint32_t j = 0; j < columns; j++)
        {
            bitsToInt(req._data[i][j], (unsigned char*)message.data() + MESSAGE_MAP::data + itterator, false);
            itterator += 4;
        }
    }

    return req;
}

template <typename T>
void _insertBytesToMessage(std::vector<char>& message, const T& varToInsert, const unsigned int sizeOfVar)
{
    std::vector<char> varBytes(sizeOfVar);
    intToBits<T>(varToInsert, (unsigned char*)varBytes.data(), false, sizeOfVar);
    message.insert(std::end(message), std::begin(varBytes), std::end(varBytes));
}

std::vector<char> Request::to_bytes()
{
    uint32_t rows = 0, columns = 0;

    if (_data.size() != 0)
    {
        rows = _data.size();
        columns = _data[0].size();
    }

    std::vector<char> dataBytes;

    unsigned int size = sizeof(_data[0][0]);
    for (auto row : _data)
    {
        for (auto cell : row)
        {
            std::vector<char> cellBytes(size, 0);
            intToBits(cell, (unsigned char*)cellBytes.data(), false);
            dataBytes.insert(std::end(dataBytes), std::begin(cellBytes), std::end(cellBytes));
        }
    }

    std::vector<char> message(sizeof(uint32_t), 0);

    _insertBytesToMessage(message, (uint32_t)_command, 4);
    _insertBytesToMessage(message, rows, 4);
    _insertBytesToMessage(message, columns, 4);
    _insertBytesToMessage(message, _min, 4);
    _insertBytesToMessage(message, _max, 4);

    message.insert(std::end(message), std::begin(dataBytes), std::end(dataBytes));

    std::vector<char> varBytes(4);
    intToBits(message.size(), (unsigned char*)varBytes.data(), false, size = 4);

    std::copy(std::begin(varBytes), std::end(varBytes), std::begin(message));

    return message;
}

std::ostream& operator<<(std::ostream& os, Request const& m)
{

    std::string data_to_s;

    for (auto row : m.data())
    {
        for (auto cell : row)
        {
            auto string = std::to_string(cell);
            string.resize(10,' ');
            data_to_s += string;
        }
        data_to_s += "\n";
    }


    return os << "Command: " << m.command()
        << "\nMin: " << m.getMin()
        << "\nMax: " << m.getMax()
        << "\nData:\n"
        << data_to_s;
}