#pragma once

#include "common.hpp"
#include "encoder.hpp"

namespace ffc
{

inline ref<Encoder> Encoder::Build(const EncoderDesc& desc)
{
    ref<Encoder> r;
    switch (desc.type)
    {
    case YUV_420P_to_H264:
        r = ref<Encoder>(new Encoder_420P_H264());
        break;
    case YUV_NV12_to_H264:
        r = ref<Encoder>(new Encoder_NV12_H264());
        break;
    default:
        break;
    }
    return r->Init(desc) ? r : ref<Encoder>();
}

inline Encoder_YUV_H264::~Encoder_YUV_H264()
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
}

inline bool Encoder_YUV_H264::Input(const void* src)
{
    if (!src)
    {
        if (avcodec_send_frame(_codec_ctx, nullptr) < 0)
        {
            return false;
        }
        return true;
    }
    if (av_frame_make_writable(_frame) < 0)
    {
        return false;
    }

    UpdateFrameYuv(src);

    if (avcodec_send_frame(_codec_ctx, _frame) < 0)
    {
        return false;
    }
    return true;
}

inline int Encoder_YUV_H264::Output(void* buf)
{
    if (!_pkt)
    {
        return -1;
    }
    // av_packet_unref(_pkt);
    if (avcodec_receive_packet(_codec_ctx, _pkt) != 0)
    {
        return -1;
    }
    if (_pkt->size <= 0)
    {
        return 0;
    }
    memcpy(buf, _pkt->data, _pkt->size);
    return _pkt->size;
}

inline Encoder_YUV_H264::Encoder_YUV_H264() :
    Encoder(),
    _codec(nullptr),
    _codec_ctx(nullptr),
    _pkt(nullptr),
    _frame(nullptr),
    _pts(0)
{
}

inline bool Encoder_YUV_H264::Init(const EncoderDesc& desc)
{
    _codec = avcodec_find_encoder_by_name("libx264");
    if (!_codec)
    {
        return false;
    }
    _codec_ctx = avcodec_alloc_context3(_codec);
    if (!_codec_ctx)
    {
        return false;
    }
    _codec_ctx->bit_rate = 1024 * (s64)desc.kbps;
    _codec_ctx->width = desc.width;
    _codec_ctx->height = desc.height;
    _codec_ctx->time_base = AVRational(1, (int)desc.fps);
    _codec_ctx->framerate = AVRational((int)desc.fps, 1);
    _codec_ctx->gop_size = 16;
    _codec_ctx->max_b_frames = 0;
    _codec_ctx->pix_fmt = AVPixelFormat::AV_PIX_FMT_YUV420P;
    if (avcodec_open2(_codec_ctx, _codec, nullptr) < 0)
    {
        return false;
    }

    //if (_codec->id == AVCodecID::AV_CODEC_ID_H264)
    //{
    //    if (av_opt_set(_codec_ctx->priv_data, "preset", "slow", 0) < 0)
    //    {
    //        return false;
    //    }
    //}

    _frame = av_frame_alloc();
    if (!_frame)
    {
        return false;
    }
    _frame->format = _codec_ctx->pix_fmt;
    _frame->width = _codec_ctx->width;
    _frame->height = _codec_ctx->height;
    if (av_frame_get_buffer(_frame, 0) < 0)
    {
        return false;
    }
    _pkt = av_packet_alloc();
    if (!_pkt)
    {
        return false;
    }
    return true;
}

inline void Encoder_420P_H264::UpdateFrameYuv(const void* src)
{
    const u8* pSrc = (const u8*)src;
    /* Y */
    for (s32 y = 0; y < _codec_ctx->height; y++)
    {
        memcpy(&_frame->data[0][y * _frame->linesize[0]], pSrc, _codec_ctx->width);
        pSrc += _codec_ctx->width;
    }
    /* Cb and Cr */
    for (s32 y = 0; y < _codec_ctx->height / 2; y++)
    {
        memcpy(&_frame->data[1][y * _frame->linesize[1]], pSrc, _codec_ctx->width / 2);
        pSrc += _codec_ctx->width / 2;
    }
    for (s32 y = 0; y < _codec_ctx->height / 2; y++)
    {
        memcpy(&_frame->data[2][y * _frame->linesize[2]], pSrc, _codec_ctx->width / 2);
        pSrc += _codec_ctx->width / 2;
    }
    _frame->pts = _pts++;
}

inline void Encoder_NV12_H264::UpdateFrameYuv(const void* src)
{
    const u8* pSrc = (const u8*)src;
    /* Y */
    for (s32 y = 0; y < _codec_ctx->height; y++)
    {
        memcpy(&_frame->data[0][y * _frame->linesize[0]], pSrc, _codec_ctx->width);
        pSrc += _codec_ctx->width;
    }
    /* Cb and Cr */
    for (s32 y = 0; y < _codec_ctx->height / 2; y++)
    {
        for (s32 x = 0; x < _codec_ctx->width / 2; x++)
        {
            _frame->data[1][y * _frame->linesize[1] + x] = *pSrc++;
            _frame->data[2][y * _frame->linesize[2] + x] = *pSrc++;
        }
    }
    _frame->pts = _pts++;
}

}
