
#define _CRT_SECURE_NO_WARNINGS
#include "../ExSocket/ex_socket.hpp"
#include "converter.hpp"

void tcp_h264_to_yuv420p()
{
    char* buf = new char[1024 * 1024 * 64];
    char* out = new char[1024 * 1024 * 64];
    auto* f_h264 = fopen("./test.h264", "wb+");
    auto* f_420p = fopen("./test.420p", "wb+");

    ffc::DecoderDesc desc = {};
    desc.type = ffc::H264_to_YUV_420P;
    auto decoder = ffc::Decoder::Build(desc);
    auto receiver = Es::Tcp::Receiver::Build(10086);
    auto client = receiver->WaitClient();
    while (1)
    {
        int receive_len = receiver->Read(client, buf);
        if (receive_len <= 0)
        {
            break;
        }
        fwrite(buf, 1, receive_len, f_h264);
        int decode_len = 0;
        while (decode_len < receive_len)
        {
            decode_len += decoder->Input(buf + decode_len, receive_len);
            while (decoder->Output(out))
            {
                auto info = decoder->GetInfo();
                fwrite(out, 1, info.size, f_420p);
            }
        }
    }
    fclose(f_h264);
    fclose(f_420p);
}

void local_h264_to_yuv420p()
{
    char* buf = new char[1024 * 1024 * 64];
    char* out = new char[1024 * 1024 * 64];
    auto* f_h264 = fopen("./test.h264", "rb");
    auto* f_420p = fopen("./test.420p", "wb+");

    ffc::DecoderDesc desc = {};
    desc.type = ffc::H264_to_YUV_420P;
    auto decoder = ffc::Decoder::Build(desc);
    while (1)
    {
        int read_len = (int)fread(buf, 1, 1024 * 512, f_h264);
        if (read_len <= 0)
        {
            break;
        }
        int decode_len = 0;
        while (decode_len < read_len)
        {
            decode_len += decoder->Input(buf + decode_len, read_len);
            while (decoder->Output(out))
            {
                auto info = decoder->GetInfo();
                fwrite(out, 1, info.size, f_420p);
            }
        }
    }
    fclose(f_h264);
    fclose(f_420p);
}

int main()
{
    tcp_h264_to_yuv420p();
    //local_h264_to_yuv420p();
    return 0;
}
