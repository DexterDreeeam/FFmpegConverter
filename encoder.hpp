#pragma once

#include "common.hpp"

namespace ffc
{

enum _EncoderType
{
    YUV_420P_to_H264,
    YUV_NV12_to_H264,
};

struct EncoderDesc
{
    _EncoderType type;
    u32          kbps;
    u32          width;
    u32          height;
    u32          fps;
};

class Encoder
{
public:
    static ref<Encoder> Build(const EncoderDesc& desc);

    virtual ~Encoder() = default;

    virtual bool Input(const void* src) = 0;

    virtual int Output(void* buf) = 0;

protected:
    Encoder() = default;

    virtual bool Init(const EncoderDesc& desc) = 0;
};

class Encoder_YUV_H264 : public Encoder
{
public:
    virtual ~Encoder_YUV_H264() override;

    virtual bool Input(const void* src) override;

    virtual int Output(void* buf) override;

protected:
    Encoder_YUV_H264();

    virtual bool Init(const EncoderDesc& desc) override;

    virtual void UpdateFrameYuv(const void* src) = 0;

protected:
    const AVCodec*   _codec;
    AVCodecContext*  _codec_ctx;
    AVPacket*        _pkt;
    AVFrame*         _frame;
    s64              _pts;
};

class Encoder_420P_H264 : public Encoder_YUV_H264
{
    friend class Encoder;

public:
    virtual ~Encoder_420P_H264() override = default;

protected:
    Encoder_420P_H264() = default;

    virtual void UpdateFrameYuv(const void* src) override;
};

class Encoder_NV12_H264 : public Encoder_YUV_H264
{
    friend class Encoder;

public:
    virtual ~Encoder_NV12_H264() override = default;

protected:
    Encoder_NV12_H264() = default;

    virtual void UpdateFrameYuv(const void* src) override;
};

}
