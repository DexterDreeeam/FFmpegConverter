#pragma once

#include <winsock.h>
#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <algorithm>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

class TcpReceiver
{
public:
    TcpReceiver() = default;

    ~TcpReceiver() = default;

public:
    bool Init(int port)
    {
        WSADATA wsa_data = {};
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
        {
            std::cout << __FUNCTION__ << "::WSAStartup" << " Failed!" << std::endl;
            return false;
        }

        char host_name[256] = {};
        if (gethostname(host_name, sizeof(host_name)) != 0)
        {
            std::cout << __FUNCTION__ << "::gethostname" << " Failed!" << std::endl;
            return false;
        }

        _sk = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (_sk == INVALID_SOCKET)
        {
            std::cout << __FUNCTION__ << "::socket" << " Failed!" << std::endl;
            return false;
        }

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons((u_short)port);
        if (bind(_sk, (SOCKADDR*)&addr, sizeof(SOCKADDR)) != 0)
        {
            std::cout << __FUNCTION__ << "::bind" << " Failed!" << std::endl;
            return false;
        }

        int sk_buf_len = 64 * 1024 * 1024;
        setsockopt(_sk, SOL_SOCKET, SO_RCVBUF, (char*)&sk_buf_len, sizeof(sk_buf_len));

        if (listen(_sk, SOMAXCONN) != 0)
        {
            std::cout << __FUNCTION__ << "::listen" << " Failed!" << std::endl;
            return false;
        }

        return true;
    }

    void Uninit()
    {
        closesocket(_sk);
    }

    SOCKET WaitClient()
    {
        sockaddr _addr;
        SOCKET client_sk = accept(_sk, &_addr, nullptr);
        if (client_sk < 0)
        {
            return -1;
        }
        return client_sk;
    }

    int Read(SOCKET client_sk, void* buf)
    {
        struct
        {
            unsigned long session_id;
            unsigned long data_len;
        } header;

        int recv_len = recv(client_sk, (char*)&header, sizeof(header), 0);
        if (recv_len < 0 || header.data_len <= 0)
        {
            return -1;
        }

        bool is_success = recv(client_sk, (char*)buf, header.data_len, 0) == header.data_len;
        std::cout << "Session Id: " << header.session_id << ", Data Len: " << header.data_len << ", ";
        std::cout << (is_success ? "Success" : "Fail") << std::endl;
        return (int)header.data_len;
    }

private:
    SOCKET                   _sk;

};
