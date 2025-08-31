#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <iostream>
#include <system_error>
#include <thread>
#include <unistd.h>
#include <vector>

std::vector<char> vBuffer(256 * 1);

void GrabData(asio::ip::tcp::socket& socket) {
    socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
    [&](std::error_code error, std::size_t len) {
        if (!error) {
            std::cout << "\n\nRead " << len << " bytes\n\n";
            std::cout.write(vBuffer.data(), static_cast<std::streamsize>(len));
            std::cout.flush();
            GrabData(socket);
        } else {
            std::cerr << "Read error: " << error.message() << "\n";
        }
    });
}

int main() {
    asio::io_context context_;
    asio::error_code error_;

    auto work = asio::make_work_guard(context_);
    std::thread t([&]{ context_.run(); });

    auto addr = asio::ip::make_address("127.0.0.1", error_);
    if (error_) {
        std::cerr << "Invalid address: " << error_.message() << "\n";
        context_.stop();
        if (t.joinable()) t.join();
        return 1;
    }

    asio::ip::tcp::endpoint endpoint(addr, 8000);
    asio::ip::tcp::socket sock(context_);

    sock.connect(endpoint, error_);
    if (error_) {
        std::cerr << " There was an error:\n" << error_.message() << std::endl;
        context_.stop();
        if (t.joinable()) t.join();
        return 1;
    }

    if (sock.is_open()) {
        const std::string sRequest =
            "GET / HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Connection: close\r\n"
            "\r\n";

        sock.write_some(asio::buffer(sRequest.data(), sRequest.size()), error_);

        GrabData(sock);

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(20000ms);

        context_.stop();
        if (t.joinable()) t.join();
    }

    return 0;
}
