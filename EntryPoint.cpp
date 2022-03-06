
#include "converter.hpp"

void yuv420p_to_h264()
{
    char buf[64];
    auto* f = fopen("./test.h264", "wb+");
    int idx = 0;

    ffc::EncoderDesc desc = {};
    desc.type = ffc::YUV_420P_to_H264;
    desc.kbps = 2000;
    desc.width = 640;
    desc.height = 480;
    desc.fps = 25;

    auto encoder = ffc::Encoder::Build(desc);
    while (1)
    {
        if (idx < 200)
        {
            encoder->Input(buf);
        }
        else if (idx == 200)
        {
            encoder->Input(nullptr);
        }

        auto out = encoder->Output();
        if (out.first == 0 || out.second == nullptr)
        {
            if (idx < 200)
            {
                continue;
            }
            break;
        }
        fwrite(out.second, 1, out.first, f);
        ++idx;
    }
    fclose(f);
}

void h264_to_yuv420p()
{
    const int buffer_size = 2048;
    char* buf = new char[buffer_size + AV_INPUT_BUFFER_PADDING_SIZE];
    char* output_buf = new char[1024 * 1024 * 4];
    auto* f_in = fopen("./test.h264", "rb");
    auto* f_out = fopen("./test.420p", "wb+");

    ffc::DecoderDesc desc = {};
    desc.type = ffc::H264_to_YUV_420P;

    auto decoder = ffc::Decoder::Build(desc);

    while (!feof(f_in))
    {
        int file_read_len = fread(buf, 1, buffer_size, f_in);
        if (!file_read_len)
        {
            break;
        }
        int decoder_read_len = 0;
        while (decoder_read_len < file_read_len)
        {
            decoder_read_len += decoder->Input(buf + decoder_read_len, file_read_len - decoder_read_len);
            while (decoder->Output(output_buf))
            {
                int size = decoder->GetInfo().width * decoder->GetInfo().height * 3 / 2;
                fwrite(output_buf, 1, size, f_out);
            }
        }
    }

    delete[] buf;
    delete[] output_buf;
    fclose(f_in);
    fclose(f_out);
}

int main()
{
    //yuv420p_to_h264();
    h264_to_yuv420p();
    return 0;
}
