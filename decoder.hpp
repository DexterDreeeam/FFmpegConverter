#pragma once

#include "common.hpp"

namespace ffc
{

enum _DecoderType
{
    H264_to_YUV_420P,
    H264_to_YUV_420P_NV12,
    H264_to_YUV_NV12,
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

    // input src must has at least 64 bytes padding after src[len]
    virtual int Input(void* src, int len) = 0;

    virtual bool Output(void* mem) = 0;

    virtual StreamInfo GetInfo() const = 0;

protected:
    Decoder() = default;

    virtual bool Init(const DecoderDesc& desc) = 0;
};

class Decoder_H264_YUV : public Decoder
{
public:
    virtual ~Decoder_H264_YUV() override;

    virtual int Input(void* src, int len) override;

    virtual bool Output(void* mem) override;

    virtual StreamInfo GetInfo() const override;

protected:
    Decoder_H264_YUV();

    virtual bool Init(const DecoderDesc& desc) override;

    virtual void ReadFrameYuv(void* mem) = 0;

protected:
    const AVCodec* _codec;
    AVCodecParserContext* _parser;
    AVCodecContext* _codec_ctx;
    AVPacket* _pkt;
    AVFrame* _frame;
    s64 _pts;
};

class Decoder_H264_420P : public Decoder_H264_YUV
{
public:
    friend class Decoder;

    virtual ~Decoder_H264_420P() override = default;

protected:
    Decoder_H264_420P() = default;

    virtual void ReadFrameYuv(void* mem) override;
};

class Decoder_H264_420P_NV12 : public Decoder_H264_YUV
{
public:
    friend class Decoder;

    virtual ~Decoder_H264_420P_NV12() override = default;

protected:
    Decoder_H264_420P_NV12() = default;

    virtual void ReadFrameYuv(void* mem) override;
};

class Decoder_H264_NV12 : public Decoder_H264_YUV
{
public:
    friend class Decoder;

    virtual ~Decoder_H264_NV12() override = default;

protected:
    Decoder_H264_NV12() = default;

    virtual void ReadFrameYuv(void* mem) override;
};

}
