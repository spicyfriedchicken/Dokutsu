#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <cstdint>
#include <iostream>
#include <memory>
#include <type_traits>
#include <cstring>

namespace net {

template <typename T>
struct messageHeader {
    T id{};
    uint32_t size = 0;
};

template <typename T>
struct message {
    messageHeader<T> header_;
    std::vector<uint8_t> data_;

    friend std::ostream& operator<<(std::ostream& os, const message<T>& msg) {
        os << "ID: " << int(msg.header_.id) << " Size: " << msg.header_.size;
        return os;
    }

    template <typename DataType>
    friend message<T>& operator<<(message<T>& msg, const DataType& data) {
        static_assert(std::is_standard_layout<DataType>::value, "DataType must be standard layout");
        size_t i = msg.data_.size();
        msg.data_.resize(i + sizeof(DataType));
        std::memcpy(msg.data_.data() + i, &data, sizeof(DataType));
        msg.header_.size = msg.data_.size();
        return msg;
    }

    template <typename DataType>
    friend message<T>& operator>>(message<T>& msg, DataType& data) {
        static_assert(std::is_standard_layout<DataType>::value, "DataType must be standard layout");
        size_t i = msg.data_.size() - sizeof(DataType);
        std::memcpy(&data, msg.data_.data() + i, sizeof(DataType));
        msg.data_.resize(i);
        msg.header_.size = msg.data_.size();
        return msg;
    }

    size_t size() const { return sizeof(messageHeader<T>) + data_.size(); }
};

template <typename T>
class connection;

template <typename T>
struct owned_message {
    std::shared_ptr<connection<T>> remote = nullptr;
    message<T> msg;

    friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg) {
        os << msg.msg;
        return os;
    }
};

} // namespace net

#endif // COMMON_H
