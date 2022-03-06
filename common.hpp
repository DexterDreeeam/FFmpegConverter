#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>

#pragma comment(lib, "./lib/avcodec.lib")
#pragma comment(lib, "./lib/avdevice.lib")
#pragma comment(lib, "./lib/avfilter.lib")
#pragma comment(lib, "./lib/avformat.lib")
//#pragma comment(lib, "./lib/avresample.lib")
#pragma comment(lib, "./lib/avutil.lib")
#pragma comment(lib, "./lib/swresample.lib")
#pragma comment(lib, "./lib/swscale.lib")

#define __STDC_CONSTANT_MACROS
extern "C"
{
#include "include/libavcodec/avcodec.h"
#include "include/libavformat/avformat.h"
#include "include/libavutil/opt.h"
#include "include/libavutil/avutil.h"
#include "include/libswscale/swscale.h"
}

namespace ffc
{

    using u8 = std::uint8_t;
    using s8 = std::int8_t;
    using u16 = std::uint16_t;
    using s16 = std::int16_t;
    using u32 = std::uint32_t;
    using s32 = std::int32_t;
    using u64 = std::uint64_t;
    using s64 = std::int64_t;

    using thread = std::thread;
    using mutex = std::mutex;
    using mutex_guard = std::unique_lock<std::mutex>;
    using rw_mtx = std::shared_mutex;
    using rw_unique_guard = std::unique_lock<std::shared_mutex>;
    using rw_shared_guard = std::shared_lock<std::shared_mutex>;

    template<typename T>               using atom = std::atomic<T>;
    template<typename T1, typename T2> using pair = std::pair<T1, T2>;
    template<typename T>               using vec = std::vector<T>;
    template<typename T>               using list = std::list<T>;
    template<typename T>               using ref = std::shared_ptr<T>;

    void yield() { std::this_thread::yield(); }
    void sleep_ms(u32 s) { std::this_thread::sleep_for(std::chrono::milliseconds(s)); }
    void sleep_us(u32 s) { std::this_thread::sleep_for(std::chrono::microseconds(s)); }

    // ================== custom ==================
    const bool print_msg = true;

    // ***************** constant *****************\

    struct _eu_endl {} __declspec(selectany) endl;

    class _eu_cout
    {
    public:
        _eu_cout() = default;
        ~_eu_cout() = default;

        template<typename Ty>
        _eu_cout operator <<(const Ty& x)
        {
            if (print_msg)
            {
                std::cout << x;
            }
            return *this;
        }

        _eu_cout operator <<(const _eu_endl&)
        {
            if (print_msg)
            {
                std::cout << std::endl;
            }
            return *this;
        }
    };

    __declspec(selectany) _eu_cout cout;
}
