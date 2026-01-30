# FFmpeg Learning 编码器编译与验证指南

## 项目概述
这是一个FFmpeg源码项目，包含了一个自定义的learning编码器用于教学目的。

## 步骤 1：配置FFmpeg

### 快速配置

```bash
./configure --enable-encoder=learning
```

**说明**：配置FFmpeg并启用learning编码器，等待配置完成后显示配置摘要

```yaml
subProjectPath: .
command: ./configure --enable-encoder=learning
cwd: .
port: null
previewUrl: null
description: 配置FFmpeg并启用learning编码器
role: build
```

## 步骤 2：编译FFmpeg

### 执行编译

```bash
make -j$(nproc)
```

**说明**：编译整个FFmpeg项目，使用所有可用CPU核心进行并行编译，等待编译完成（可能需要几分钟）

```yaml
subProjectPath: .
command: make -j$(nproc)
cwd: .
port: null
previewUrl: null
description: 编译FFmpeg项目
role: build
```

## 步骤 3：验证编码器

### 检查编码器是否可用

```bash
./ffmpeg -encoders | grep learning
```

**说明**：列出所有编码器并查找learning编码器，应该显示learning编码器信息

```yaml
subProjectPath: .
command: ./ffmpeg -encoders | grep learning
cwd: .
port: null
previewUrl: null
description: 验证learning编码器是否编译成功
role: test
```

## 步骤 4：执行转码测试（可选）

### 准备测试

如果你有测试视频文件（如input.mp4），可以执行：

```bash
./ffmpeg -i input.mp4 -c:v learning -quality 80 output.mkv
```

**说明**：使用learning编码器进行转码，设置质量参数为80

```yaml
subProjectPath: .
command: ./ffmpeg -i input.mp4 -c:v learning -quality 80 output.mkv
cwd: .
port: null
previewUrl: null
description: 使用learning编码器执行转码测试
role: test
```

## 注意事项

- 配置、编译、验证必须按顺序执行
- 编译时间取决于CPU性能，首次编译可能需要5-15分钟
- 转码测试需要准备输入视频文件
- 查看详细文档：LEARNING_ENCODER_INTEGRATION.md
