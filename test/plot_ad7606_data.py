#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
AD7606采集数据波形绘制工具
从串口输出文本中提取数据并绘制波形图
"""

import re
import sys
import os

# 尝试导入matplotlib，如果失败则提示安装
try:
    import numpy as np
    import matplotlib
    # 在非交互式环境中使用Agg后端
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    from matplotlib import font_manager
    MATPLOTLIB_AVAILABLE = True
except ImportError as e:
    print(f"错误：缺少必要的库。请运行：pip install numpy matplotlib")
    print(f"详细错误：{e}")
    sys.exit(1)

# 设置中文字体（Windows系统）
plt.rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei', 'Arial Unicode MS']
plt.rcParams['axes.unicode_minus'] = False  # 解决负号显示问题

def parse_serial_data(text):
    """
    从串口输出文本中解析数据
    格式：Sample #1000: V1=-2199 (0xF769) = -0.671 V
    或：Sample #2000: V1=-30 (0xFFE2) = -0.009 V [Rate: 11904 SPS]
    """
    # 正则表达式匹配：Sample #数字: V1=数字 (0x十六进制) = 电压值 V
    pattern = r'Sample #(\d+):\s+V1=(-?\d+)\s+\(0x[0-9A-Fa-f]+\)\s+=\s+(-?[\d.]+)\s+V'
    
    samples = []
    voltages = []
    
    for line in text.split('\n'):
        match = re.search(pattern, line)
        if match:
            sample_num = int(match.group(1))
            adc_value = int(match.group(2))
            voltage = float(match.group(3))
            
            samples.append(sample_num)
            voltages.append(voltage)
    
    return np.array(samples), np.array(voltages)

def plot_waveform(samples, voltages, sample_rate=None):
    """
    绘制波形图
    """
    if len(samples) == 0:
        print("错误：没有找到有效数据！")
        return
    
    # 计算时间轴（如果知道采样率）
    if sample_rate is None:
        # 从数据中估算采样率（使用最后两个样本的时间差）
        if len(samples) >= 2:
            # 假设每1000个样本打印一次，计算采样率
            # 从数据中提取采样率信息
            time_axis = samples / 11627.0  # 使用平均采样率11627 SPS
            time_label = "时间 (秒)"
        else:
            time_axis = samples
            time_label = "采样序号"
    else:
        time_axis = samples / sample_rate
        time_label = "时间 (秒)"
    
    # 创建图形
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8))
    
    # 子图1：完整波形
    ax1.plot(time_axis, voltages, 'b-', linewidth=1.5, label='采集数据')
    ax1.set_xlabel(time_label, fontsize=12)
    ax1.set_ylabel('电压 (V)', fontsize=12)
    ax1.set_title('AD7606采集波形 - 完整视图', fontsize=14, fontweight='bold')
    ax1.grid(True, alpha=0.3)
    ax1.legend()
    
    # 计算统计信息
    v_max = np.max(voltages)
    v_min = np.min(voltages)
    v_pp = v_max - v_min
    v_rms = np.sqrt(np.mean(voltages**2))
    v_mean = np.mean(voltages)
    
    # 添加统计信息文本框
    stats_text = f'最大值: {v_max:.3f} V\n'
    stats_text += f'最小值: {v_min:.3f} V\n'
    stats_text += f'峰峰值: {v_pp:.3f} V\n'
    stats_text += f'RMS值: {v_rms:.3f} V\n'
    stats_text += f'平均值: {v_mean:.3f} V\n'
    stats_text += f'采样点数: {len(samples)}'
    
    ax1.text(0.02, 0.98, stats_text, transform=ax1.transAxes,
             fontsize=10, verticalalignment='top',
             bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
    
    # 子图2：局部放大（显示几个周期）
    if len(time_axis) > 100:
        # 显示中间部分的几个周期
        start_idx = len(time_axis) // 3
        end_idx = start_idx + min(5000, len(time_axis) - start_idx)
        ax2.plot(time_axis[start_idx:end_idx], voltages[start_idx:end_idx], 
                'r-', linewidth=1.5, label='局部放大')
        ax2.set_xlabel(time_label, fontsize=12)
        ax2.set_ylabel('电压 (V)', fontsize=12)
        ax2.set_title('AD7606采集波形 - 局部放大', fontsize=14, fontweight='bold')
        ax2.grid(True, alpha=0.3)
        ax2.legend()
    else:
        ax2.plot(time_axis, voltages, 'r-', linewidth=1.5, label='采集数据')
        ax2.set_xlabel(time_label, fontsize=12)
        ax2.set_ylabel('电压 (V)', fontsize=12)
        ax2.set_title('AD7606采集波形', fontsize=14, fontweight='bold')
        ax2.grid(True, alpha=0.3)
        ax2.legend()
    
    plt.tight_layout()
    return fig

def main():
    """
    主函数
    """
    print("=" * 60)
    print("AD7606采集数据波形绘制工具")
    print("=" * 60)
    
    # 检查命令行参数
    if len(sys.argv) > 1:
        input_file = sys.argv[1]
        print(f"\n从文件读取数据：{input_file}")
        if os.path.exists(input_file):
            with open(input_file, 'r', encoding='utf-8', errors='ignore') as f:
                text = f.read()
            print(f"文件读取成功，数据长度：{len(text)} 字符")
        else:
            print(f"错误：文件不存在：{input_file}")
            return
    else:
        # 默认读取test目录下的数据文件
        input_file = os.path.join(os.path.dirname(__file__), 'ad7606_data.txt')
        if os.path.exists(input_file):
            print(f"\n从文件读取数据：{input_file}")
            with open(input_file, 'r', encoding='utf-8', errors='ignore') as f:
                text = f.read()
        else:
            print(f"\n使用方法：")
            print(f"  1. 将串口输出数据保存到文件：{input_file}")
            print(f"  2. 运行脚本：python {os.path.basename(__file__)} [数据文件路径]")
            print(f"\n或者直接粘贴数据到终端（按Ctrl+D结束输入）")
            print("=" * 60)
            
            # 从标准输入读取
            print("\n请粘贴串口输出数据（按Ctrl+D或Ctrl+Z结束）：")
            text = sys.stdin.read()
    
    if not text:
        print("错误：没有输入数据！")
        return
    
    # 解析数据
    print("\n正在解析数据...")
    samples, voltages = parse_serial_data(text)
    
    if len(samples) == 0:
        print("错误：未能从输入中解析出有效数据！")
        print("\n请确保数据格式为：")
        print("Sample #1000: V1=-2199 (0xF769) = -0.671 V")
        return
    
    print(f"成功解析 {len(samples)} 个数据点")
    print(f"采样范围：{samples[0]} - {samples[-1]}")
    print(f"电压范围：{voltages.min():.3f} V - {voltages.max():.3f} V")
    
    # 估算采样率（从数据中提取）
    sample_rate = None
    rate_pattern = r'\[Rate:\s*(\d+)\s*SPS\]'
    matches = re.findall(rate_pattern, text)
    if matches:
        sample_rate = int(matches[-1])  # 使用最后一个采样率
        print(f"检测到采样率：{sample_rate} SPS")
    
    # 绘制波形
    print("\n正在绘制波形图...")
    fig = plot_waveform(samples, voltages, sample_rate)
    
    # 保存图片
    output_file = os.path.join(os.path.dirname(__file__), 'ad7606_waveform.png')
    fig.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"\n波形图已保存到：{output_file}")
    
    # 关闭图形（使用Agg后端，不需要显示）
    plt.close(fig)
    
    print("\n完成！")

if __name__ == '__main__':
    main()

