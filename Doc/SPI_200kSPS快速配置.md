# AD7606 SPI接口200kSPS配置快速参考

## ⚠️ 关键结论

### ❌ 单DOUT线无法满足200kSPS
- 读取8通道需要128个SCLK周期
- 即使使用最高频率，也无法在5μs周期内完成
- **最大采样频率：约100-125 kSPS**

### ✅ 双DOUT线可以实现200kSPS
- 使用DOUTA和DOUTB并行读取
- 读取时间减半（64个SCLK周期）
- **最大采样频率：可达200-250 kSPS**

---

## 一、硬件连接（200kSPS必须）

### 必须连接的信号：

| AD7606引脚 | STM32H743引脚 | 说明 |
|-----------|--------------|------|
| **DOUTA (DB7)** | **SPI1_MISO** | 主数据线，读取通道1-4 |
| **DOUTB (DB8)** | **SPI2_MISO** | 辅助数据线，读取通道5-8 |
| RD/SCLK | SPI1_SCK 和 SPI2_SCK | 两个SPI共享时钟（或使用同一时钟源） |
| CS | 任意GPIO | 片选信号（两个SPI共享） |

**重要：**
- ✅ 必须同时连接DOUTA和DOUTB
- ✅ 需要配置**两个SPI外设**（SPI1和SPI2）
- ✅ 两个SPI使用相同的CS信号

---

## 二、STM32CubeMX配置

### 2.1 配置SPI1（连接DOUTA）

1. **Mode**: Full-Duplex Master
2. **Hardware NSS Signal**: Disable
3. **Data Size**: **16 Bits**
4. **First Bit**: MSB First
5. **Clock Polarity**: High
6. **Clock Phase**: 2 Edge（SPI Mode 3）
7. **Prescaler**: **8**（SPI时钟 = 15 MHz）

### 2.2 配置SPI2（连接DOUTB）

**配置与SPI1完全相同：**
1. **Mode**: Full-Duplex Master
2. **Hardware NSS Signal**: Disable
3. **Data Size**: **16 Bits**
4. **First Bit**: MSB First
5. **Clock Polarity**: High
6. **Clock Phase**: 2 Edge（SPI Mode 3）
7. **Prescaler**: **8**（SPI时钟 = 15 MHz）

### 2.3 配置DMA（推荐）

**SPI1 DMA配置：**
- DMA Request: SPI1_RX
- Direction: Peripheral to Memory
- Priority: High
- Data Width: Half Word (16 bits)

**SPI2 DMA配置：**
- DMA Request: SPI2_RX
- Direction: Peripheral to Memory
- Priority: High
- Data Width: Half Word (16 bits)

---

## 三、代码实现

### 3.1 双DOUT并行读取函数

```c
// 使用双SPI并行读取8个通道（200kSPS优化）
void AD7606_ReadChannels_Dual(uint16_t *channels)
{
    AD7606_CS_LOW();  // 片选有效
    
    // 同时启动两个SPI的DMA传输
    HAL_SPI_Receive_DMA(&hspi1, (uint8_t*)&channels[0], 4);  // 通道1-4 (DOUTA)
    HAL_SPI_Receive_DMA(&hspi2, (uint8_t*)&channels[4], 4);  // 通道5-8 (DOUTB)
    
    // 等待传输完成
    while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY ||
           HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY);
    
    AD7606_CS_HIGH(); // 片选无效
}
```

### 3.2 转换中读取模式（流水线）

```c
// 启动转换并读取上次数据（实现200kSPS的关键）
void AD7606_StartAndRead(uint16_t *channels)
{
    // 1. 启动新转换
    AD7606_StartConvst();
    
    // 2. 立即读取上次转换的数据（在BUSY高电平期间）
    // 注意：第一次调用时读取的是无效数据，需要先启动一次转换
    AD7606_ReadChannels_Dual(channels);
}
```

### 3.3 定时器中断方式（连续采集）

```c
// 定时器中断服务程序
void TIMx_IRQHandler(void)
{
    static uint16_t channels[8];
    
    TIM_ClearFlag(HTIMx, TIM_FLAG_Update);
    
    // 读取上次转换的结果
    AD7606_ReadChannels_Dual(channels);
    
    // 启动下次转换
    AD7606_StartConvst();
    
    // 保存数据到FIFO或处理...
    // 例如：保存到FIFO缓冲区
}
```

---

## 四、性能估算

### 使用双DOUT + 15MHz SPI时钟：

- **读取时间**：64个SCLK / 15MHz = **4.3 μs**
- **转换时间**：4 μs
- **总周期**：max(4μs, 4.3μs) = **4.3 μs**
- **最大采样频率**：1 / 4.3μs = **232 kSPS** ✅

**结论：可以满足200kSPS的实时要求！**

---

## 五、关键要点总结

### ✅ 实现200kSPS必须满足：

1. ✅ **使用双DOUT线**（DOUTA + DOUTB）
2. ✅ **配置两个SPI外设**（SPI1和SPI2）
3. ✅ **使用"转换中读取"模式**（流水线操作）
4. ✅ **SPI时钟设置为15 MHz**（Prescaler = 8）
5. ✅ **推荐使用DMA传输**（减少CPU开销）
6. ✅ **VDRIVE使用5V**（更安全，有更大余量）

### ❌ 如果只能使用单DOUT：

- **最大采样频率约100-125 kSPS**
- 无法达到200kSPS的实时要求
- 建议降低采样频率，或使用FMC并口方案

---

## 六、配置检查清单

- [ ] DOUTA连接到SPI1_MISO
- [ ] DOUTB连接到SPI2_MISO
- [ ] 两个SPI配置为16位模式
- [ ] SPI时钟设置为15 MHz（Prescaler = 8）
- [ ] 使用SPI Mode 3（CPOL=High, CPHA=2Edge）
- [ ] 配置DMA传输（推荐）
- [ ] 使用"转换中读取"模式
- [ ] VDRIVE连接到5V（推荐）

---

**详细分析请参考：`AD7606_SPI_200kSPS可行性分析.md`**

