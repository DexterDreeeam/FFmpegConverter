
#include "../converter.hpp"
#include "TcpReceiver.hpp"

void receive_h264_to_yuv420p_file()
{
    ffc::DecoderDesc desc = {};
    desc.type = ffc::H264_to_YUV_420P;
    auto decoder = ffc::Decoder::Build(desc);

    std::cout << ">>>>>>>>> Listen Port: ";
    int port;
    std::cin >> port;

    TcpReceiver receiver;
    receiver.Init(port);

    const int buffer_size = 1024 * 1024 * 16;
    char* buf = new char[buffer_size + AV_INPUT_BUFFER_PADDING_SIZE];
    char* output_buf = new char[1024 * 1024 * 4];
    auto* f_out = fopen("./test.420p", "wb+");

    auto client = receiver.WaitClient();

    while (1)
    {
        int packet_len = receiver.Read(client, buf);
        int decoder_read_len = 0;
        while (decoder_read_len < packet_len)
        {
            decoder_read_len += decoder->Input(buf + decoder_read_len, packet_len - decoder_read_len);
            while (decoder->Output(output_buf))
            {
                int size = decoder->GetInfo().width * decoder->GetInfo().height * 3 / 2;
                fwrite(output_buf, 1, size, f_out);
            }
        }
    }

    delete[] buf;
    delete[] output_buf;
    fclose(f_out);
}

int main()
{
    //yuv420p_to_h264();
    receive_h264_to_yuv420p_file();
    return 0;
}
