# AD7606 SPI接口200kSPS实时性分析

## 一、时序分析

### 1.1 AD7606转换时间

根据数据手册：
- **无过采样时**：转换时间 tCONV = **4 μs**（8通道）
- **采样周期**：200 kSPS = 5 μs/周期
- **转换完成后**：BUSY信号下降，数据可读

### 1.2 SPI读取时间计算

**读取8个通道需要：**
- 每个通道：16位 = 2字节
- 8个通道：16字节 = 128位
- 需要：**128个SCLK周期**

**SPI时钟限制（数据手册Table 3）：**
- VDRIVE = 3.3V：最大SCLK = **17 MHz**
- VDRIVE = 5V：最大SCLK = **23.5 MHz**

**SPI传输时间计算：**

| VDRIVE | 最大SCLK | 128周期时间 | 加上开销 |
|--------|----------|-------------|----------|
| 3.3V | 17 MHz | 7.5 μs | ~8-10 μs |
| 5V | 23.5 MHz | 5.4 μs | ~6-8 μs |

### 1.3 时间预算分析

**方案A：转换后读取（Reading After Conversion）**
```
总时间 = 转换时间 + 读取时间
      = 4 μs + 8 μs (3.3V) = 12 μs
      = 4 μs + 6 μs (5V) = 10 μs

对应采样频率：
- 3.3V: 1/12μs ≈ 83 kSPS ❌ 无法达到200kSPS
- 5V: 1/10μs = 100 kSPS ❌ 无法达到200kSPS
```

**方案B：转换中读取（Reading During Conversion）** ✅ **推荐**
```
根据数据手册：
- 可以在BUSY高电平期间读取上次转换的数据
- 这样可以实现流水线操作

时间分配：
- 转换时间：4 μs（当前转换）
- 读取时间：8 μs（读取上次数据，与转换并行）
- 总周期：max(4μs, 8μs) = 8 μs

对应采样频率：
- 3.3V: 1/8μs = 125 kSPS ⚠️ 仍无法达到200kSPS
- 5V: 1/8μs = 125 kSPS ⚠️ 仍无法达到200kSPS
```

### 1.4 使用双DOUT线（DOUTA + DOUTB）

**优势：**
- 可以并行读取两个通道的数据
- 读取时间减半

**时间计算：**
- 使用DOUTA和DOUTB同时读取
- 需要64个SCLK周期（而不是128个）
- 读取时间：64 / 17MHz = 3.8 μs (3.3V)

**总时间：**
- 转换时间：4 μs
- 读取时间：3.8 μs（并行读取）
- 总周期：max(4μs, 3.8μs) ≈ 4 μs

**对应采样频率：**
- 1/4μs = **250 kSPS** ✅ **可以满足200kSPS！**

---

## 二、结论

### ✅ 可以实现200kSPS的条件：

1. **必须使用双DOUT线**（DOUTA + DOUTB）
2. **必须使用"转换中读取"模式**（Reading During Conversion）
3. **VDRIVE建议使用5V**（更高的SCLK频率）
4. **SPI时钟频率**：建议设置为15-20 MHz（接近但不超过限制）

### ❌ 单DOUT线无法满足200kSPS

单DOUT线读取8通道需要128个SCLK周期，即使使用5V和最高频率，也无法在5μs周期内完成。

---

## 三、STM32CubeMX SPI配置（200kSPS优化）

### 3.1 基本配置

| 参数 | 推荐值 | 说明 |
|------|--------|------|
| Mode | Full-Duplex Master | 全双工主模式 |
| Hardware NSS Signal | Disable | 软件控制CS |
| Data Size | **16 Bits** | 必须16位 |
| First Bit | MSB First | |
| Clock Polarity (CPOL) | High | |
| Clock Phase (CPHA) | 2 Edge | **SPI Mode 3** |
| Prescaler | **2** (5V) 或 **4** (3.3V) | 高频率 |

### 3.2 时钟频率配置

**STM32H743 SPI时钟源：**
- APB时钟：120 MHz（如果系统时钟480MHz，APB=120MHz）

**Prescaler选择：**
- **VDRIVE = 5V时**：
  - Prescaler = 2: SPI时钟 = 120MHz / 2 = **60 MHz**（太高，超过23.5MHz限制）
  - Prescaler = 4: SPI时钟 = 120MHz / 4 = **30 MHz**（太高）
  - Prescaler = 8: SPI时钟 = 120MHz / 8 = **15 MHz** ✅ **推荐**
  - Prescaler = 16: SPI时钟 = 120MHz / 16 = **7.5 MHz**（保守）

- **VDRIVE = 3.3V时**：
  - Prescaler = 8: SPI时钟 = **15 MHz** ✅ **推荐**（接近17MHz限制）
  - Prescaler = 16: SPI时钟 = **7.5 MHz**（保守）

**推荐配置：**
- **Prescaler = 8**，SPI时钟 = **15 MHz**
- 这样既安全（不超过限制），又能提供足够的传输速度

### 3.3 使用双DOUT线的配置

**硬件连接：**
- DOUTA (DB7) → SPIx_MISO（主数据线）
- DOUTB (DB8) → 需要连接第二个SPI的MISO，或使用DMA双缓冲

**软件实现：**
- 需要同时读取DOUTA和DOUTB
- 可以使用两个SPI外设，或使用DMA配置

---

## 四、实际配置建议

### 方案1：单SPI + 单DOUTA（简单但速度受限）

**最大采样频率：约100-125 kSPS**

**配置：**
- SPI时钟：15 MHz
- 使用DOUTA读取所有8个通道
- 在转换中读取上次数据

**代码示例：**
```c
// 读取8个通道（单DOUTA，需要128个SCLK）
void AD7606_ReadChannels_Single(uint16_t *channels)
{
    AD7606_CS_LOW();
    for (int i = 0; i < 8; i++) {
        HAL_SPI_Receive(&hspi1, (uint8_t*)&channels[i], 1, HAL_MAX_DELAY);
    }
    AD7606_CS_HIGH();
}
```

### 方案2：双SPI + 双DOUT（推荐，可达200kSPS）

**最大采样频率：可达200kSPS以上**

**配置：**
- SPI1：连接DOUTA，读取通道1-4
- SPI2：连接DOUTB，读取通道5-8
- 两个SPI同时工作，并行读取

**代码示例：**
```c
// 使用两个SPI同时读取
void AD7606_ReadChannels_Dual(uint16_t *channels)
{
    AD7606_CS_LOW();
    
    // 同时启动两个SPI传输
    HAL_SPI_Receive_DMA(&hspi1, (uint8_t*)&channels[0], 8);  // 通道1-4
    HAL_SPI_Receive_DMA(&hspi2, (uint8_t*)&channels[4], 8);  // 通道5-8
    
    // 等待传输完成
    while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY ||
           HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY);
    
    AD7606_CS_HIGH();
}
```

### 方案3：单SPI + DMA（折中方案）

**最大采样频率：约150-180 kSPS**

**配置：**
- 使用DMA加速SPI传输
- 单DOUTA，但通过DMA减少CPU开销

---

## 五、CubeMX配置步骤（200kSPS优化）

### 5.1 SPI基本配置

1. **Mode**: Full-Duplex Master
2. **Hardware NSS Signal**: Disable
3. **Data Size**: 16 Bits
4. **First Bit**: MSB First
5. **Clock Polarity**: High
6. **Clock Phase**: 2 Edge
7. **Prescaler**: **8**（SPI时钟 = 15 MHz）

### 5.2 如果使用双DOUT方案

需要配置**两个SPI外设**：
- **SPI1**: 连接DOUTA
- **SPI2**: 连接DOUTB

两个SPI使用相同的配置参数。

### 5.3 DMA配置（可选但推荐）

**SPI1 DMA配置：**
- DMA Request: SPI1_RX
- Stream/Channel: 根据芯片选择
- Direction: Peripheral to Memory
- Priority: High
- Data Width: Half Word (16 bits)

**SPI2 DMA配置：**
- 类似SPI1的配置

---

## 六、实际性能估算

### 单DOUTA方案：
- SPI时钟：15 MHz
- 读取时间：128 / 15MHz = 8.5 μs
- 转换时间：4 μs
- 总周期：8.5 μs（转换中读取）
- **最大采样频率：约117 kSPS**

### 双DOUT方案：
- SPI时钟：15 MHz
- 读取时间：64 / 15MHz = 4.3 μs
- 转换时间：4 μs
- 总周期：max(4μs, 4.3μs) = 4.3 μs
- **最大采样频率：约232 kSPS** ✅ **满足200kSPS**

---

## 七、最终建议

### ✅ 要实现200kSPS，必须：

1. **使用双DOUT线**（DOUTA + DOUTB）
2. **配置两个SPI外设**（SPI1和SPI2）
3. **使用DMA传输**（减少CPU开销）
4. **在转换中读取**（流水线操作）
5. **SPI时钟设置为15 MHz**（Prescaler = 8）
6. **VDRIVE使用5V**（更安全，有更大余量）

### ⚠️ 如果只能使用单DOUT：

- **最大采样频率约100-125 kSPS**
- 无法达到200kSPS的实时要求
- 可以考虑降低采样频率，或使用FMC并口方案

---

## 八、代码实现要点

### 8.1 转换中读取模式

```c
void AD7606_StartAndRead(uint16_t *channels)
{
    // 1. 启动新转换（同时读取上次数据）
    AD7606_StartConvst();
    
    // 2. 立即读取上次转换的数据（在BUSY高电平期间）
    // 注意：这里读取的是上一次转换的结果
    AD7606_ReadChannels_Dual(channels);
    
    // 3. 等待当前转换完成（可选，如果使用BUSY信号）
    // while (HAL_GPIO_ReadPin(AD7606_BUSY_GPIO_Port, AD7606_BUSY_Pin) == GPIO_PIN_SET);
}
```

### 8.2 定时器中断方式

```c
void TIMx_IRQHandler(void)
{
    static uint16_t channels[8];
    
    // 读取上次转换的结果
    AD7606_ReadChannels_Dual(channels);
    
    // 启动下次转换
    AD7606_StartConvst();
    
    // 保存数据到FIFO...
}
```

---

**总结：SPI接口可以实现200kSPS，但必须使用双DOUT线方案！**

