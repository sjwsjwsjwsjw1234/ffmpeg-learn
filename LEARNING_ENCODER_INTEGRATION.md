# FFmpeg 编码器集成学习指南

## 概述

本指南将带你了解如何在FFmpeg中集成一个新的编码器。我们创建了一个名为"Learning Encoder"的示例编码器，它展示了编码器集成的完整流程。

## 编码器架构

FFmpeg的编码器架构主要包含以下几个层次：

```
用户应用
    ↓
AVCodec API (公共接口)
    ↓
FFCodec (内部实现)
    ↓
编码器具体实现 (init/encode/close)
```

## 集成步骤详解

### 步骤 1: 创建编码器源文件

文件：`libavcodec/learning_encoder.c`

这个文件包含了编码器的核心实现，主要包括三个关键函数：

#### 1.1 初始化函数 (init)

```c
static av_cold int learning_encode_init(AVCodecContext *avctx)
{
    // 1. 获取私有数据结构
    LearningEncoderContext *s = avctx->priv_data;
    
    // 2. 打印配置信息（用于调试）
    av_log(avctx, AV_LOG_INFO, "Learning Encoder: 初始化开始\n");
    
    // 3. 初始化编码器状态
    s->frame_count = 0;
    
    // 4. 在实际编码器中，这里会：
    //    - 分配内存缓冲区
    //    - 初始化编码器内部状态
    //    - 检查参数合法性
    
    return 0;
}
```

**要点：**
- 使用 `av_cold` 标记，表示这个函数很少被调用（仅初始化时）
- 返回0表示成功，负值表示错误
- 可以通过 `avctx->priv_data` 访问自定义的私有数据结构

#### 1.2 编码函数 (encode)

```c
static int learning_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
                                  const AVFrame *frame, int *got_packet)
{
    // 1. 处理flush操作
    if (!frame) {
        *got_packet = 0;
        return 0;
    }
    
    // 2. 分配输出数据包
    ret = ff_alloc_packet(avctx, pkt, buffer_size);
    
    // 3. 执行实际的编码操作
    // 在真实编码器中：
    //    - 色彩空间转换
    //    - 压缩算法（DCT、量化、熵编码）
    //    - 生成比特流
    
    // 4. 设置数据包属性
    pkt->pts = frame->pts;
    pkt->dts = frame->pts;
    if (is_keyframe) {
        pkt->flags |= AV_PKT_FLAG_KEY;
    }
    
    *got_packet = 1;
    return 0;
}
```

**要点：**
- `frame` 为NULL表示flush操作（输出所有缓存帧）
- 使用 `ff_alloc_packet` 分配输出缓冲区
- 必须设置 `*got_packet` 来告知是否产生了输出
- 正确设置 pts/dts 和 keyframe 标志

#### 1.3 关闭函数 (close)

```c
static av_cold int learning_encode_close(AVCodecContext *avctx)
{
    // 1. 释放分配的资源
    // 2. 打印统计信息
    // 3. 清理编码器状态
    
    return 0;
}
```

**要点：**
- 必须释放所有在init中分配的资源
- 确保不会内存泄漏

#### 1.4 编码器定义结构

```c
const FFCodec ff_learning_encoder = {
    // 基本信息
    .p.name         = "learning",
    .p.long_name    = NULL_IF_CONFIG_SMALL("Learning Encoder (Educational Example)"),
    .p.type         = AVMEDIA_TYPE_VIDEO,
    .p.id           = AV_CODEC_ID_LEARNING,
    
    // 能力标志
    .p.capabilities = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_ENCODER_REORDERED_OPAQUE,
    
    // 私有数据
    .priv_data_size = sizeof(LearningEncoderContext),
    .p.priv_class   = &learning_encoder_class,
    
    // 支持的格式
    .p.pix_fmts     = pix_fmts,
    
    // 回调函数
    .init           = learning_encode_init,
    .close          = learning_encode_close,
    FF_CODEC_ENCODE_CB(learning_encode_frame),
};
```

**重要字段说明：**
- `name`: 编码器名称，用于命令行 `-c:v learning`
- `type`: 媒体类型（VIDEO/AUDIO/SUBTITLE）
- `id`: 编解码器ID，必须在 codec_id.h 中定义
- `capabilities`: 编码器能力标志
  - `AV_CODEC_CAP_DR1`: 支持Direct Rendering
  - `AV_CODEC_CAP_ENCODER_REORDERED_OPAQUE`: 支持重排序opaque字段
- `pix_fmts`: 支持的像素格式列表

### 步骤 2: 添加编解码器ID

文件：`libavcodec/codec_id.h`

```c
enum AVCodecID {
    // ... 其他编解码器ID ...
    AV_CODEC_ID_JPEGXL_ANIM,
    AV_CODEC_ID_LEARNING,        ///< Learning encoder for educational purposes
    
    /* various PCM "codecs" */
    // ...
};
```

**要点：**
- 在适当的位置添加新的ID
- 通常添加在相似类型编解码器附近
- 不要修改现有ID的值（会破坏ABI兼容性）
- 添加注释说明用途

### 步骤 3: 注册编码器

文件：`libavcodec/allcodecs.c`

```c
// 在文件开头声明
extern const FFCodec ff_learning_encoder;

// 编码器会通过 codec_list 自动注册
// 这个列表在编译时通过 configure 脚本生成
```

**要点：**
- 必须使用 `extern` 声明
- 遵循命名规范：`ff_<name>_encoder`
- 实际的注册通过编译系统完成

### 步骤 4: 更新编译配置

文件：`libavcodec/Makefile`

```makefile
# Learning encoder for educational purposes
OBJS-$(CONFIG_LEARNING_ENCODER)        += learning_encoder.o
```

**要点：**
- 使用条件编译 `CONFIG_LEARNING_ENCODER`
- 这个宏在运行 configure 时会被定义
- 允许用户选择是否编译这个编码器

### 步骤 5: 配置编译选项

在项目根目录运行：

```bash
# 查看可用的编码器选项
./configure --help | grep learning

# 启用 learning 编码器
./configure --enable-encoder=learning

# 或者启用所有编码器
./configure --enable-encoders
```

### 步骤 6: 编译

```bash
# 编译整个项目
make -j$(nproc)

# 或者只编译 libavcodec
make libavcodec -j$(nproc)
```

## 编码器私有数据结构

```c
typedef struct LearningEncoderContext {
    AVClass *class;          // 用于AVOption系统（必需）
    int frame_count;         // 编码器状态
    int quality;             // 用户可配置参数
    int compression_level;   // 用户可配置参数
} LearningEncoderContext;
```

**要点：**
- 第一个字段必须是 `AVClass *class`
- 用于存储编码器的状态和配置
- 通过 `AVOption` 系统暴露给用户

## AVOption 系统

允许用户通过命令行或API设置参数：

```c
static const AVOption learning_encoder_options[] = {
    { "quality", "设置编码质量 (1-100)", OFFSET(quality), AV_OPT_TYPE_INT, 
      { .i64 = 50 }, 1, 100, VE },
    { "compression_level", "设置压缩级别 (0-9)", OFFSET(compression_level), 
      AV_OPT_TYPE_INT, { .i64 = 5 }, 0, 9, VE },
    { NULL }
};
```

使用示例：
```bash
ffmpeg -i input.mp4 -c:v learning -quality 80 -compression_level 7 output.mkv
```

## 支持的像素格式

```c
static const enum AVPixelFormat pix_fmts[] = {
    AV_PIX_FMT_YUV420P,    // 最常用的YUV格式
    AV_PIX_FMT_YUV422P,
    AV_PIX_FMT_YUV444P,
    AV_PIX_FMT_NONE        // 列表结束标志
};
```

**常见像素格式：**
- `YUV420P`: 4:2:0采样，最常用
- `YUV422P`: 4:2:2采样，更好的色度
- `YUV444P`: 4:4:4采样，无色度损失
- `RGB24`: RGB格式

## 编码器能力标志

```c
.p.capabilities = AV_CODEC_CAP_DR1 | 
                  AV_CODEC_CAP_ENCODER_REORDERED_OPAQUE |
                  AV_CODEC_CAP_DELAY |
                  AV_CODEC_CAP_FRAME_THREADS;
```

**常用标志：**
- `AV_CODEC_CAP_DR1`: Direct Rendering，允许编码器直接写入用户提供的缓冲区
- `AV_CODEC_CAP_DELAY`: 编码器有延迟（需要缓存帧）
- `AV_CODEC_CAP_FRAME_THREADS`: 支持帧级别的多线程
- `AV_CODEC_CAP_SLICE_THREADS`: 支持切片级别的多线程
- `AV_CODEC_CAP_ENCODER_REORDERED_OPAQUE`: 支持重排序的opaque字段

## 内部能力标志

在 `FFCodec` 结构中定义：

```c
.caps_internal = FF_CODEC_CAP_INIT_CLEANUP |
                 FF_CODEC_CAP_AUTO_THREADS;
```

**常用内部标志：**
- `FF_CODEC_CAP_INIT_CLEANUP`: init失败时会调用close清理
- `FF_CODEC_CAP_AUTO_THREADS`: 自动处理线程数量
- `FF_CODEC_CAP_SETS_PKT_DTS`: 编码器会设置DTS

## 关键帧和帧类型

```c
// 设置关键帧
if (is_keyframe) {
    pkt->flags |= AV_PKT_FLAG_KEY;
}

// 读取输入帧类型
switch (frame->pict_type) {
    case AV_PICTURE_TYPE_I:  // I帧（关键帧）
    case AV_PICTURE_TYPE_P:  // P帧（预测帧）
    case AV_PICTURE_TYPE_B:  // B帧（双向预测帧）
    // ...
}
```

## 时间戳处理

```c
// 设置输出时间戳
pkt->pts = frame->pts;           // 显示时间戳
pkt->dts = frame->pts;           // 解码时间戳

// 对于有B帧的编码器，DTS可能不等于PTS
// DTS必须单调递增，PTS可以乱序
```

## 错误处理

```c
// 返回FFmpeg错误码
return AVERROR(ENOMEM);      // 内存不足
return AVERROR(EINVAL);      // 参数无效
return AVERROR_EXTERNAL;     // 外部库错误
return AVERROR_PATCHWELCOME; // 功能未实现

// 使用av_log记录错误
av_log(avctx, AV_LOG_ERROR, "编码失败: %s\n", av_err2str(ret));
```

## 日志级别

```c
av_log(avctx, AV_LOG_ERROR,   "错误信息");
av_log(avctx, AV_LOG_WARNING, "警告信息");
av_log(avctx, AV_LOG_INFO,    "一般信息");
av_log(avctx, AV_LOG_DEBUG,   "调试信息");
av_log(avctx, AV_LOG_TRACE,   "追踪信息");
```

## 测试编码器

### 方法 1: 使用 ffmpeg 命令行

```bash
# 查看编码器是否可用
./ffmpeg -encoders | grep learning

# 使用编码器 (注意：learning编码器没有注册到容器格式，所以需要指定 rawvideo 格式或 tag)
./ffmpeg -y -f lavfi -i testsrc=size=128x128:rate=25:duration=2 -pix_fmt yuv420p -c:v learning -f rawvideo output.bin

# 查看详细信息
./ffmpeg -i input.mp4 -c:v learning -quality 80 -loglevel debug -f rawvideo output.bin
```

### 方法 2: 使用 ffprobe

```bash
# 检查输出文件
ffprobe output.mkv
```

### 方法 3: 使用 C API

```c
#include <libavcodec/avcodec.h>

int main() {
    const AVCodec *codec = avcodec_find_encoder_by_name("learning");
    if (!codec) {
        printf("编码器未找到\n");
        return -1;
    }
    
    AVCodecContext *ctx = avcodec_alloc_context3(codec);
    // 设置参数
    ctx->width = 1920;
    ctx->height = 1080;
    ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    
    // 打开编码器
    if (avcodec_open2(ctx, codec, NULL) < 0) {
        printf("打开编码器失败\n");
        return -1;
    }
    
    // 使用编码器...
    
    avcodec_free_context(&ctx);
    return 0;
}
```

## 高级主题

### 1. 多线程支持

如果编码器支持多线程：

```c
// 在初始化中
if (avctx->thread_count > 1) {
    // 设置多线程编码
}

// 帧级线程需要实现 update_thread_context
.update_thread_context = learning_update_thread_context,
```

### 2. 硬件加速

集成硬件编码器（如NVENC、VAAPI）：

```c
// 定义硬件配置
const AVCodecHWConfigInternal *const learning_hw_configs[] = {
    HW_CONFIG_ENCODER_FRAMES(VAAPI, VAAPI),
    NULL,
};

.hw_configs = learning_hw_configs,
```

### 3. 码率控制

实现不同的码率控制模式：

```c
// CBR (Constant Bit Rate)
if (avctx->bit_rate > 0 && avctx->rc_max_rate == 0) {
    // 实现CBR
}

// VBR (Variable Bit Rate)
if (avctx->bit_rate > 0 && avctx->rc_max_rate > 0) {
    // 实现VBR
}

// CQP (Constant Quantizer Parameter)
if (avctx->global_quality > 0) {
    // 实现CQP
}
```

### 4. 延迟和缓冲

如果编码器需要缓冲多个帧（如B帧）：

```c
// 设置延迟标志
.p.capabilities = AV_CODEC_CAP_DELAY,

// 在encode中处理缓冲
static int learning_encode_frame(...) {
    if (frame) {
        // 添加到缓冲区
        add_to_buffer(frame);
    }
    
    // 尝试从缓冲区输出
    if (can_output()) {
        *got_packet = 1;
        // 生成数据包
    }
}
```

## 常见问题

### Q1: 编码器无法找到

**A:** 检查以下几点：
1. 是否在 `allcodecs.c` 中声明
2. 是否在 `Makefile` 中添加编译规则
3. 运行 `./configure --enable-encoder=learning`
4. 重新编译 `make clean && make`

### Q2: 编译错误

**A:** 常见原因：
1. 头文件包含不正确
2. 函数签名与定义不匹配
3. 缺少必需的字段（如 AVClass）

### Q3: 运行时崩溃

**A:** 检查：
1. 是否正确初始化所有结构体字段
2. 内存分配和释放是否匹配
3. 数组边界检查
4. 空指针检查

### Q4: 输出文件损坏

**A:** 可能的原因：
1. PTS/DTS 设置不正确
2. 关键帧标志未设置
3. 数据包大小计算错误
4. 字节序问题

## 性能优化建议

1. **使用SIMD指令**
   ```c
   #include "libavutil/x86/asm.h"
   // 使用SSE、AVX等优化
   ```

2. **减少内存拷贝**
   ```c
   // 尽可能使用指针操作
   // 使用 AV_CODEC_CAP_DR1 直接渲染
   ```

3. **并行处理**
   ```c
   // 实现帧级或切片级多线程
   ```

4. **缓存友好的数据结构**
   ```c
   // 对齐数据结构
   // 减少缓存未命中
   ```

## 调试技巧

1. **启用调试日志**
   ```bash
   ffmpeg -loglevel trace -i input.mp4 -c:v learning output.mkv
   ```

2. **使用 GDB 调试**
   ```bash
   gdb --args ffmpeg -i input.mp4 -c:v learning output.mkv
   ```

3. **内存检测**
   ```bash
   valgrind --leak-check=full ffmpeg -i input.mp4 -c:v learning output.mkv
   ```

4. **性能分析**
   ```bash
   perf record ffmpeg -i input.mp4 -c:v learning output.mkv
   perf report
   ```

## 参考资源

### FFmpeg官方文档
- [FFmpeg官方网站](https://ffmpeg.org/)
- [FFmpeg开发文档](https://ffmpeg.org/developer.html)
- [Doxygen API文档](https://ffmpeg.org/doxygen/trunk/)

### 参考实现
- `libavcodec/null.c` - 最简单的编解码器
- `libavcodec/rawenc.c` - RAW视频编码器
- `libavcodec/libx264.c` - 封装外部编码器库
- `libavcodec/mpeg12enc.c` - 完整的视频编码器
- `libavcodec/huffyuvenc.c` - 无损视频编码器

### 学习路径

1. **基础阶段**
   - 理解FFmpeg架构
   - 学习AVFrame、AVPacket结构
   - 实现简单的null编码器

2. **进阶阶段**
   - 实现简单的压缩算法
   - 添加参数配置
   - 优化性能

3. **高级阶段**
   - 实现多线程
   - 添加硬件加速
   - 实现完整的视频编码标准

## 总结

集成一个编码器到FFmpeg需要以下关键步骤：

1. ✅ 创建编码器源文件（实现 init/encode/close）
2. ✅ 在 codec_id.h 添加编解码器ID
3. ✅ 在 allcodecs.c 声明编码器
4. ✅ 在 Makefile 添加编译规则
5. ✅ 配置和编译
6. ✅ 测试和调试

通过本示例，你已经了解了：
- FFmpeg编码器的基本结构
- 如何实现关键的编码函数
- 如何集成到FFmpeg框架中
- 如何配置和编译
- 如何测试和调试

这个learning encoder虽然功能简单，但展示了一个完整的编码器集成流程。你可以在此基础上实现更复杂的编码算法。

## 下一步

1. **扩展功能**：添加实际的压缩算法
2. **性能优化**：使用SIMD指令优化
3. **添加解码器**：实现对应的解码器
4. **硬件加速**：集成GPU编码
5. **完善文档**：添加更详细的注释

祝学习愉快！🚀
