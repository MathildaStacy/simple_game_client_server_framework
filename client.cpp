#include <iostream>
#include <chrono>
#include <thread>
#include "net_client.hpp"
#include "net_common.hpp"
#include "net_connection.hpp"



enum class CustomMsgTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
};

class CustomClient : public fbr::net::client_interface<CustomMsgTypes>
{
public:
    void PingServer()
    {
        fbr::net::message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::ServerPing;

        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

        msg << timeNow;
        Send(msg);
    }

    void MessageAll()
    {
        fbr::net::message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::MessageAll;
        Send(msg);
    }
};

int main()
{
    CustomClient c;
    c.Connect("127.0.0.1", 60000);

    bool bQuit = false;
    while (!bQuit)
    {
        std::cout << "Press '1' to Ping Server, '2' to Message All, '3' to Quit: ";
        std::string input;
        std::getline(std::cin, input); // Read input from user

        if (input == "1")
            c.PingServer();
        else if (input == "2")
            c.MessageAll();
        else if (input == "3")
            bQuit = true;

        if (c.IsConnected())
        {
            if (!c.Incoming().empty())
            {
                auto msg = c.Incoming().pop_front().msg;

                switch (msg.header.id)
                {
                case CustomMsgTypes::ServerAccept:
                {
                    std::cout << "Server Accepted Connection\n";
                }
                break;

                case CustomMsgTypes::ServerPing:
                {
                    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                    std::chrono::system_clock::time_point timeThen;
                    msg >> timeThen;
                    std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
                }
                break;

                case CustomMsgTypes::ServerMessage:
                {
                    uint32_t clientID;
                    msg >> clientID;
                    std::cout << "Hello from [" << clientID << "]\n";
                }
                break;
                }
            }
        }
        else
        {
            std::cout << "Server Down\n";
            bQuit = true;
        }

        // Adding a small delay to make the loop more manageable
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
