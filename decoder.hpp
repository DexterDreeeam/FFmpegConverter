#pragma once

#include "common.hpp"

namespace ffc
{

enum _DecoderType
{
    H264_to_YUV_420P,
};

struct DecoderDesc
{
    _DecoderType type;
};

struct StreamInfo
{
    u32   width;
    u32   height;
    u32   size;
    float fps;
};

class Decoder
{
public:
    static ref<Decoder> Build(const DecoderDesc& desc);

    virtual ~Decoder() = default;

    virtual s32 Input(void* src, u32 len) = 0;

    virtual bool Output(void* mem) = 0;

    virtual StreamInfo GetInfo() = 0;

protected:
    Decoder() = default;

    virtual bool Init(const DecoderDesc& desc) = 0;
};

class Decoder_H264_420P : public Decoder
{
    friend class Decoder;

public:
    virtual ~Decoder_H264_420P() override
    {
        if (_frame)
        {
            av_frame_free(&_frame);
        }
        if (_pkt)
        {
            av_packet_unref(_pkt);
            av_packet_free(&_pkt);
        }
        if (_codec_ctx)
        {
            avcodec_free_context(&_codec_ctx);
        }
        if (_parser)
        {
            av_parser_close(_parser);
            _parser = nullptr;
        }
    }

    virtual s32 Input(void* src, u32 len) override
    {
        if (!src || len == 0)
        {
            return -1;
        }

        u8* pSrc = (u8*)src;
        memset(pSrc + len, 0, AV_INPUT_BUFFER_PADDING_SIZE);

        s32 input_len = av_parser_parse2(
            _parser, _codec_ctx, &_pkt->data, &_pkt->size,
            pSrc, len, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

        if (input_len < 0)
        {
            // if input_len == 0, it doesn't mean there is no packet
            return -1;
        }

        if (_pkt->size > 0)
        {
            if (avcodec_send_packet(_codec_ctx, _pkt) < 0)
            {
                return -1;
            }
        }
        return input_len;
    }

    virtual bool Output(void* mem) override
    {
        if (!mem)
        {
            return false;
        }
        int val = avcodec_receive_frame(_codec_ctx, _frame);
        if (val < 0)
        {
            return false;
        }
        if (!_frame || !_frame->data || !_frame->data[0])
        {
            return false;
        }
        if (!_frame->linesize[0] || !_frame->width || !_frame->height)
        {
            return false;
        }
        char* pMem = (char*)mem;
        for (s32 r = 0; r < _frame->height; ++r)
        {
            memcpy(pMem, _frame->data[0] + r * _frame->linesize[0], _frame->width);
            pMem += _frame->width;
        }
        for (s32 r = 0; r < _frame->height / 2; ++r)
        {
            for (s32 c = 0; c < _frame->width / 2; ++c)
            {
                *pMem = _frame->data[1][r * _frame->linesize[1] + c];
                ++pMem;
                *pMem = _frame->data[2][r * _frame->linesize[2] + c];
                ++pMem;
            }
        }
        return true;
    }

    virtual StreamInfo GetInfo() override
    {
        StreamInfo info = {};
        if (!_frame || !_frame->width || !_frame->height)
        {
            return info;
        }
        if (!_codec_ctx)
        {
            return info;
        }
        info.width = _frame->width;
        info.height = _frame->height;
        info.size = _frame->width * _frame->height * 3 / 2;
        info.fps = (float)_codec_ctx->framerate.num / (float)_codec_ctx->framerate.den;
        return info;
    }

protected:
    Decoder_H264_420P() :
        Decoder(),
        _codec(nullptr),
        _parser(nullptr),
        _codec_ctx(nullptr),
        _pkt(nullptr),
        _frame(nullptr),
        _pts(0)
    {
    }

    virtual bool Init(const DecoderDesc& desc) override
    {
        _codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!_codec)
        {
            return false;
        }

        _parser = av_parser_init(_codec->id);
        if (!_parser)
        {
            return false;
        }

        _codec_ctx = avcodec_alloc_context3(_codec);
        if (!_codec_ctx)
        {
            return false;
        }

        if (avcodec_open2(_codec_ctx, _codec, nullptr) < 0)
        {
            return false;
        }

        _pkt = av_packet_alloc();
        if (!_pkt)
        {
            return false;
        }

        _frame = av_frame_alloc();
        if (!_frame)
        {
            return false;
        }
        return true;
    }

private:
    const AVCodec* _codec;
    AVCodecParserContext* _parser;
    AVCodecContext* _codec_ctx;
    AVPacket* _pkt;
    AVFrame* _frame;
    s64 _pts;
};

ref<Decoder> Decoder::Build(const DecoderDesc& desc)
{
    ref<Decoder> r;
    switch (desc.type)
    {
    case H264_to_YUV_420P:
        r = ref<Decoder>(new Decoder_H264_420P());
        break;
    default:
        break;
    }
    return r->Init(desc) ? r : ref<Decoder>();
}

}
