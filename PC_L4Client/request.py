from enum import Enum
import math
import sys

class COMMAND_TYPES(Enum):
    unknown = 0
    upload = 1
    start = 3
    status = 4

    r_idle = 100
    r_data_accepted = 101
    r_processing = 102
    r_success = 103
    r_error = 104

class MESSAGE_MAP(Enum):
    messageLength = 0
    command = 4
    rows = 8
    columns = 12
    min = 16
    max = 20
    data = 24

class request:
    command = 0
    min = 0
    max = 0
    data = 0

    def __init__(self):
        self.command = 0
        self.min = 0
        self.max = 0
        self.data = []

    def __str__(self):
        data_str = ''
        for row in self.data:
            for cell in row:
                data_str += str(cell).ljust(10)[:10]
            data_str +="\n"
        return "Command: " + str(self.command.name) + "\nMin: " + str(self.min) + "\nMax: " + str(self.max) + "\nData:\n"+ data_str

    def __bytes__(self):
        rows = 0
        columns = 0
        if len(self.data) != 0:
            rows = len(self.data)
            columns = len(self.data[0])
        
        data_bytes = bytearray(b'')
        for row in self.data:
            for cell in row:
                data_bytes.extend(cell.to_bytes(4,'big')) # here number depends on matrix dtype
        
        message = bytearray(int(0).to_bytes(4,'big') + self.command.value.to_bytes(4,'big') + rows.to_bytes(4,'big') + columns.to_bytes(4,'big') + self.min.to_bytes(4,'big') + self.max.to_bytes(4,'big'))
        message.extend(data_bytes)
        
        message[0:4] = bytearray(len(message).to_bytes(4,'big'))

        return bytes(message)

def read_request(message: bytes) -> request:
    command = int.from_bytes(message[MESSAGE_MAP.command.value:MESSAGE_MAP.command.value+4],'big')
    rows = int.from_bytes(message[MESSAGE_MAP.rows.value:MESSAGE_MAP.rows.value+4],'big')
    columns = int.from_bytes(message[MESSAGE_MAP.columns.value:MESSAGE_MAP.columns.value+4],'big')
    min_val = int.from_bytes(message[MESSAGE_MAP.min.value:MESSAGE_MAP.min.value+4],'big')
    max_val = int.from_bytes(message[MESSAGE_MAP.max.value:MESSAGE_MAP.max.value+4],'big')

    req = request()
    req.command = COMMAND_TYPES(command)
    req.max = max_val
    req.min = min_val
    req.data = [[0] * columns for _ in range(rows)]

    iterator = 0
    for i in range(rows):
        for j in range(columns):
            req.data[i][j] = int.from_bytes(message[MESSAGE_MAP.data.value + iterator:MESSAGE_MAP.data.value + iterator+4],'big')
            iterator += 4

    return req

def upload_request(matrix, min, max):
    req = request();
    req.command = COMMAND_TYPES.upload
    req.data = matrix
    req.min = min
    req.max = max
    return req

def start_request():
    req = request();
    req.command = COMMAND_TYPES.start
    return req

def status_request():
    req = request();
    req.command = COMMAND_TYPES.status
    return req


