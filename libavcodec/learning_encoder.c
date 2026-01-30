/*
 * Learning Encoder - A simple example encoder for educational purposes
 */

#include "config_components.h"

#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/internal.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/opt.h"
#include "libavutil/log.h"

#include "avcodec.h"
#include "codec_internal.h"
#include "encode.h"

typedef struct LearningEncoderContext {
    AVClass *class;          // 用于AVOption系统
    int frame_count;         // 已编码帧数
    int quality;             // 质量参数（示例参数）
    int compression_level;   // 压缩级别（示例参数）
    
    AVFrame *frame;          // 用于接收输入的内部帧缓存
} LearningEncoderContext;

static av_cold int learning_encode_init(AVCodecContext *avctx)
{
    LearningEncoderContext *s = avctx->priv_data;
    
    av_log(avctx, AV_LOG_INFO, "Learning Encoder Init: w=%d h=%d fmt=%d\n", avctx->width, avctx->height, avctx->pix_fmt);
    
    s->frame = av_frame_alloc();
    if (!s->frame)
        return AVERROR(ENOMEM);
        
    s->frame_count = 0;
    
    return 0;
}

static int learning_receive_packet(AVCodecContext *avctx, AVPacket *pkt)
{
    LearningEncoderContext *s = avctx->priv_data;
    int ret;
    uint8_t *buf;
    
    // 1. 获取输入帧
    ret = ff_encode_get_frame(avctx, s->frame);
    if (ret < 0 && ret != AVERROR_EOF)
        return ret;
        
    // 2. 处理 EOF
    if (ret == AVERROR_EOF) {
        return AVERROR_EOF;
    }

    // 3. 开始编码
    s->frame_count++;
    
    // 4. 分配 packet 空间
    int size_needed = 128; 
    
    ret = ff_get_encode_buffer(avctx, pkt, size_needed, 0);
    if (ret < 0) {
        av_log(avctx, AV_LOG_ERROR, "Failed to allocate packet buffer\n");
        av_frame_unref(s->frame);
        return ret;
    }
    
    buf = pkt->data;
    
    // 5. 写入数据 (HEADER + Metadata)
    AV_WB32(buf, MKTAG('L', 'E', 'R', 'N'));
    buf += 4;
    AV_WB32(buf, s->frame_count);
    buf += 4;
    AV_WB16(buf, avctx->width);
    buf += 2;
    AV_WB16(buf, avctx->height);
    buf += 2;
    
    // 6. 设置 packet 属性
    pkt->pts = s->frame->pts;
    pkt->dts = s->frame->pts;
    
    if (s->frame_count % 10 == 1) {
        pkt->flags |= AV_PKT_FLAG_KEY;
    }
    
    // 7. 调整 packet 大小
    av_shrink_packet(pkt, buf - pkt->data);
    
    // 8. 释放输入帧
    av_frame_unref(s->frame);
    
    return 0;
}

static av_cold int learning_encode_close(AVCodecContext *avctx)
{
    LearningEncoderContext *s = avctx->priv_data;
    if (s && s->frame)
        av_frame_free(&s->frame);
    av_log(avctx, AV_LOG_INFO, "Learning Encoder: Closed, total frames: %d\n", s->frame_count);
    return 0;
}

#define OFFSET(x) offsetof(LearningEncoderContext, x)
#define VE AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_ENCODING_PARAM

static const AVOption learning_encoder_options[] = {
    { "quality", "设置编码质量 (1-100)", OFFSET(quality), AV_OPT_TYPE_INT, 
      { .i64 = 50 }, 1, 100, VE },
    { "compression_level", "设置压缩级别 (0-9)", OFFSET(compression_level), AV_OPT_TYPE_INT, 
      { .i64 = 5 }, 0, 9, VE },
    { NULL }
};

static const AVClass learning_encoder_class = {
    .class_name = "learning_encoder",
    .item_name  = av_default_item_name,
    .option     = learning_encoder_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

static const enum AVPixelFormat pix_fmts[] = {
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_YUV422P,
    AV_PIX_FMT_YUV444P,
    AV_PIX_FMT_NONE
};

const FFCodec ff_learning_encoder = {
    .p.name         = "learning",
    .p.long_name    = NULL_IF_CONFIG_SMALL("Learning Encoder (Educational Example)"),
    .p.type         = AVMEDIA_TYPE_VIDEO,
    .p.id           = AV_CODEC_ID_LEARNING,
    .p.capabilities = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_ENCODER_REORDERED_OPAQUE,
    .priv_data_size = sizeof(LearningEncoderContext),
    .p.priv_class   = &learning_encoder_class,
    .p.pix_fmts     = pix_fmts,
    .init           = learning_encode_init,
    .close          = learning_encode_close,
    FF_CODEC_RECEIVE_PACKET_CB(learning_receive_packet),
};
