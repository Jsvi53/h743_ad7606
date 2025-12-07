# STM32H743 AD7606 SPI串口配置指南

## 一、AD7606 SPI接口硬件连接

### 1.1 AD7606接口模式选择

**重要：** 使用SPI接口时，需要设置AD7606的接口选择引脚：

- **PAR/SER/BYTE SEL**: 接**高电平**（选择串行/字节模式）
- **DB15/BYTE SEL**: 接**低电平**（选择串行接口，不是并行字节模式）

### 1.2 SPI信号连接

| AD7606引脚 | STM32H743引脚 | 说明 |
|-----------|--------------|------|
| CS | 任意GPIO（软件控制） | 片选信号 |
| RD/SCLK | SPIx_SCK | 串行时钟 |
| DB7/DOUTA | SPIx_MISO | 串行数据输出A（主通道） |
| DB8/DOUTB | 可选连接 | 串行数据输出B（可选，用于双通道读取） |

### 1.3 控制信号连接（GPIO）

| AD7606信号 | STM32H743引脚 | 说明 |
|-----------|--------------|------|
| CONVST | 任意GPIO输出 | 转换启动信号（推荐支持PWM的引脚） |
| RESET | 任意GPIO输出 | 复位信号 |
| RANGE | 任意GPIO输出 | 量程选择（0=±5V, 1=±10V） |
| OS0 | 任意GPIO输出 | 过采样选择 |
| OS1 | 任意GPIO输出 | 过采样选择 |
| OS2 | 任意GPIO输出 | 过采样选择 |
| BUSY | 任意GPIO输入（可选） | 忙信号（推荐支持外部中断的引脚） |

**注意：**
- DB15必须接低电平（GND）以选择串行接口模式
- DB0-DB6, DB9-DB14在串行模式下应接地或不连接
- 如果只使用DOUTA，DOUTB可以不连接

---

## 二、STM32CubeMX配置步骤

### 2.1 配置SPI外设

#### 步骤1：选择SPI外设
1. 左侧面板：`Connectivity` → 选择 `SPI1`、`SPI2`、`SPI3` 等（根据硬件连接选择）
2. 中间面板会显示SPI配置选项

#### 步骤2：Mode配置
在"SPI1 Mode and Configuration"中：

| 参数 | 推荐值 | 说明 |
|------|--------|------|
| **Mode** | `Full-Duplex Master` | 全双工主模式 |
| **Hardware NSS Signal** | `Disable` | 使用软件控制CS |

#### 步骤3：Parameter Settings配置
点击"Configuration"标签页 → "Parameter Settings"：

| 参数 | 推荐值 | 说明 |
|------|--------|------|
| **Frame Format** | `Motorola` | 标准SPI格式 |
| **Data Size** | `16 Bits` | AD7606输出16位数据 |
| **First Bit** | `MSB First` | 最高位先传输 |
| **Prescaler (for Baud Rate)** | `2` 或 `4` | 根据系统时钟调整，建议先选2 |
| **Baud Rate** | 根据Prescaler计算 | 建议10-20 MHz（VDRIVE=3.3V时最大17MHz） |
| **Clock Polarity (CPOL)** | `Low` 或 `High` | 根据AD7606时序要求 |
| **Clock Phase (CPHA)** | `1 Edge` 或 `2 Edge` | 根据AD7606时序要求 |

**重要时序参数说明：**

根据AD7606数据手册（Table 3）：
- **VDRIVE = 3.3V时**：最大SCLK频率 = 17 MHz
- **VDRIVE = 5V时**：最大SCLK频率 = 23.5 MHz

**推荐配置（参考现有代码）：**
- CPOL: `High` (CPOL=1)
- CPHA: `2 Edge` (CPHA=1)
- 即：**SPI Mode 3**

#### 步骤4：Advanced Parameters（可选）
- **CRC Calculation**: `Disabled`
- **NSSP Mode**: `Enabled`
- **NSS Signal Type**: `Software`
- **Fifo Threshold**: `Fifo Threshold 01 Data`
- 其他保持默认

### 2.2 配置GPIO引脚

#### 2.2.1 SPI引脚（自动配置）
配置SPI后，CubeMX会自动配置以下引脚：
- **SCK**: 根据选择的SPI外设自动配置
- **MISO**: 根据选择的SPI外设自动配置

#### 2.2.2 CS片选信号（手动配置）
CS信号需要手动配置为GPIO输出：

1. 在"Pinout & Configuration"中找到对应的GPIO引脚
2. 配置为：
   - Mode: `GPIO_Output`
   - GPIO output level: `High`（默认高电平，CS无效）
   - GPIO mode: `Output Push Pull`
   - GPIO Pull-up/Pull-down: `No pull-up and no pull-down`
   - Maximum output speed: `Very High`
   - User Label: `AD7606_CS`

#### 2.2.3 AD7606控制信号GPIO配置

在"Pinout & Configuration"中，找到对应的GPIO并配置：

**CONVST（转换启动）**:
- Mode: `GPIO_Output`
- GPIO output level: `High`（默认高电平）
- GPIO mode: `Output Push Pull`
- GPIO Pull-up/Pull-down: `No pull-up and no pull-down`
- Maximum output speed: `Very High`
- User Label: `AD7606_CONVST`

**RESET（复位）**:
- Mode: `GPIO_Output`
- GPIO output level: `Low`
- GPIO mode: `Output Push Pull`
- GPIO Pull-up/Pull-down: `No pull-up and no pull-down`
- Maximum output speed: `Very High`
- User Label: `AD7606_RESET`

**RANGE（量程选择）**:
- Mode: `GPIO_Output`
- GPIO output level: `Low`（默认±5V量程）
- GPIO mode: `Output Push Pull`
- GPIO Pull-up/Pull-down: `No pull-up and no pull-down`
- Maximum output speed: `Very High`
- User Label: `AD7606_RANGE`

**OS0, OS1, OS2（过采样选择）**:
- Mode: `GPIO_Output`
- GPIO output level: `Low`
- GPIO mode: `Output Push Pull`
- GPIO Pull-up/Pull-down: `No pull-up and no pull-down`
- Maximum output speed: `Very High`
- User Label: `AD7606_OS0`, `AD7606_OS1`, `AD7606_OS2`

**BUSY（忙信号，可选）**:
- Mode: `GPIO_Input`
- GPIO Pull-up/Pull-down: `No pull-up and no pull-down`
- User Label: `AD7606_BUSY`

### 2.3 时钟配置（200kSPS优化）

**对于200kSPS应用，推荐配置：**

1. 在"Clock Configuration"中检查SPI时钟源频率
2. **Prescaler设置**：
   - **VDRIVE = 5V**: Prescaler = **8**（SPI时钟 ≈ 15 MHz）
   - **VDRIVE = 3.3V**: Prescaler = **8**（SPI时钟 ≈ 15 MHz）
3. 确保不超过AD7606的最大SCLK频率（17MHz @ 3.3V, 23.5MHz @ 5V）
4. **15 MHz是安全且高效的选择**（约为最大值的70-80%）

**计算示例（STM32H743，APB时钟=120MHz）：**
- Prescaler = 8: SPI时钟 = 120MHz / 8 = **15 MHz** ✅
- 读取8通道（单DOUT）：128 / 15MHz = 8.5 μs
- 读取8通道（双DOUT）：64 / 15MHz = 4.3 μs ✅ 满足200kSPS

### 2.4 生成代码

1. 点击"Project Manager"
2. 检查"Project Settings"
3. 点击"GENERATE CODE"生成代码

---

## 三、代码实现要点

### 3.1 SPI读取AD7606数据

#### 方案A：单DOUTA读取（适用于≤100kSPS）

```c
// CS片选控制
#define AD7606_CS_LOW()    HAL_GPIO_WritePin(AD7606_CS_GPIO_Port, AD7606_CS_Pin, GPIO_PIN_RESET)
#define AD7606_CS_HIGH()   HAL_GPIO_WritePin(AD7606_CS_GPIO_Port, AD7606_CS_Pin, GPIO_PIN_SET)

// 读取一个通道的数据（16位）
uint16_t AD7606_ReadChannel(void)
{
    uint16_t data = 0;
    HAL_SPI_Receive(&hspi1, (uint8_t*)&data, 1, HAL_MAX_DELAY);
    return data;
}

// 读取8个通道的数据（单DOUTA）
void AD7606_ReadChannels_Single(uint16_t *channels)
{
    AD7606_CS_LOW();  // 片选有效
    
    for (int i = 0; i < 8; i++) {
        channels[i] = AD7606_ReadChannel();
    }
    
    AD7606_CS_HIGH(); // 片选无效
}
```

#### 方案B：双DOUT并行读取（推荐，适用于200kSPS）✅

**硬件连接：**
- DOUTA (DB7) → SPI1_MISO
- DOUTB (DB8) → SPI2_MISO

**CubeMX配置：**
- 需要配置**两个SPI外设**（SPI1和SPI2）
- 两个SPI使用相同的配置参数

```c
// 使用双SPI并行读取（200kSPS优化）
void AD7606_ReadChannels_Dual(uint16_t *channels)
{
    AD7606_CS_LOW();  // 片选有效
    
    // 同时启动两个SPI传输
    // SPI1读取通道1-4（DOUTA）
    // SPI2读取通道5-8（DOUTB）
    
    // 方式1：使用DMA（推荐，最快）
    HAL_SPI_Receive_DMA(&hspi1, (uint8_t*)&channels[0], 4);  // 通道1-4
    HAL_SPI_Receive_DMA(&hspi2, (uint8_t*)&channels[4], 4);  // 通道5-8
    
    // 等待传输完成
    while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY ||
           HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY);
    
    // 方式2：使用阻塞方式（简单但稍慢）
    // for (int i = 0; i < 4; i++) {
    //     HAL_SPI_Receive(&hspi1, (uint8_t*)&channels[i], 1, HAL_MAX_DELAY);
    //     HAL_SPI_Receive(&hspi2, (uint8_t*)&channels[i+4], 1, HAL_MAX_DELAY);
    // }
    
    AD7606_CS_HIGH(); // 片选无效
}
```

### 3.2 转换时序

#### 方案1：转换后读取（简单但速度受限）

```c
// 启动转换
void AD7606_StartConvst(void)
{
    HAL_GPIO_WritePin(AD7606_CONVST_GPIO_Port, AD7606_CONVST_Pin, GPIO_PIN_RESET);
    for (volatile int i = 0; i < 10; i++);  // 短暂延时（约25ns）
    HAL_GPIO_WritePin(AD7606_CONVST_GPIO_Port, AD7606_CONVST_Pin, GPIO_PIN_SET);
}

// 转换后读取（适用于低速应用）
void AD7606_ReadAfterConversion(uint16_t *channels)
{
    // 1. 启动转换
    AD7606_StartConvst();
    
    // 2. 等待转换完成
    HAL_Delay(1);  // 无过采样时约4us，延时1ms足够
    
    // 3. 读取8个通道的数据
    AD7606_ReadChannels_Single(channels);  // 或 ReadChannels_Dual
}
```

#### 方案2：转换中读取（推荐，200kSPS必须）✅

```c
// 转换中读取模式（流水线操作，实现200kSPS）
void AD7606_StartAndRead(uint16_t *channels)
{
    // 1. 启动新转换（同时读取上次转换的数据）
    AD7606_StartConvst();
    
    // 2. 立即读取上次转换的数据（在BUSY高电平期间）
    // 注意：这里读取的是上一次转换的结果
    // 第一次调用时，读取的是无效数据，需要先启动一次转换
    AD7606_ReadChannels_Dual(channels);  // 使用双DOUT方案
    
    // 3. 可选：等待当前转换完成（如果使用BUSY信号）
    // while (HAL_GPIO_ReadPin(AD7606_BUSY_GPIO_Port, AD7606_BUSY_Pin) == GPIO_PIN_SET);
}
```

#### 方案3：定时器中断方式（推荐用于连续采集）

```c
// 在定时器中断中实现（参考bsp_spi_ad7606.c）
void TIMx_IRQHandler(void)
{
    static uint16_t channels[8];
    
    TIM_ClearFlag(HTIMx, TIM_FLAG_Update);
    
    // 读取上次转换的结果
    AD7606_ReadChannels_Dual(channels);
    
    // 启动下次转换
    AD7606_StartConvst();
    
    // 保存数据到FIFO或处理...
}
```

---

## 四、SPI时序参数详解

### 4.1 AD7606 SPI时序要求

根据数据手册Table 3：

| 参数 | VDRIVE=3.3V | VDRIVE=5V | 说明 |
|------|-------------|-----------|------|
| **fSCLK (最大)** | 17 MHz | 23.5 MHz | 串行时钟频率 |
| **t18 (CS到DOUT有效)** | 20 ns | 15 ns | CS下降沿到MSB有效 |
| **t19 (数据访问时间)** | 23 ns | 17 ns | SCLK上升沿后数据有效 |
| **t20 (SCLK低脉冲宽度)** | 0.4 × tSCLK | 0.4 × tSCLK | SCLK低电平宽度 |
| **t21 (SCLK高脉冲宽度)** | 0.4 × tSCLK | 0.4 × tSCLK | SCLK高电平宽度 |

### 4.2 200kSPS实时性分析

**重要结论：**

⚠️ **单DOUT线无法满足200kSPS实时要求**
- 读取8通道需要128个SCLK周期
- 即使使用最高频率，读取时间约6-8μs
- 加上转换时间4μs，总周期约10-12μs
- **最大采样频率：约80-100 kSPS**

✅ **双DOUT线可以实现200kSPS**
- 使用DOUTA和DOUTB并行读取
- 读取时间减半（64个SCLK周期）
- 总周期约4-5μs
- **最大采样频率：可达200-250 kSPS**

**详细分析请参考：`AD7606_SPI_200kSPS可行性分析.md`**

### 4.2 SPI模式选择

AD7606 SPI时序特点：
- CS下降沿时，MSB出现在DOUTA/DOUTB
- 后续数据位在SCLK上升沿有效
- 数据在SCLK下降沿采样

**推荐配置：**
- **CPOL = High** (空闲时SCLK为高)
- **CPHA = 2 Edge** (第二个边沿采样，即下降沿采样)
- 即：**SPI Mode 3**

### 4.3 时钟频率设置

**计算示例（STM32H743，系统时钟480MHz）：**

如果SPI时钟源为APB时钟（120MHz）：
- Prescaler = 2: SPI时钟 = 120MHz / 2 = 60MHz（太高，超过AD7606限制）
- Prescaler = 4: SPI时钟 = 120MHz / 4 = 30MHz（太高）
- Prescaler = 8: SPI时钟 = 120MHz / 8 = 15MHz（适合3.3V）
- Prescaler = 16: SPI时钟 = 120MHz / 16 = 7.5MHz（保守选择）

**建议：**
- VDRIVE = 3.3V: Prescaler = 8 (15MHz) 或 16 (7.5MHz)
- VDRIVE = 5V: Prescaler = 4 (30MHz，但建议用8更安全)

---

## 五、配置检查清单

- [ ] SPI已配置为Full-Duplex Master模式
- [ ] Data Size设置为16 Bits
- [ ] CPOL和CPHA配置正确（推荐Mode 3）
- [ ] SPI时钟频率不超过AD7606限制
- [ ] CS引脚已配置为GPIO输出（软件控制）
- [ ] 所有AD7606控制GPIO已配置（CONVST, RESET, RANGE, OS0-OS2）
- [ ] BUSY引脚已配置（如果使用）
- [ ] 已生成代码并检查是否有错误

---

## 六、常见问题

### Q1: 如何选择SPI模式（CPOL/CPHA）？
**A**: 根据AD7606时序，推荐使用Mode 3（CPOL=High, CPHA=2Edge）。如果数据读取不正确，可以尝试Mode 0（CPOL=Low, CPHA=1Edge）。

### Q2: SPI时钟频率如何设置？
**A**: 
- 确保不超过AD7606的最大SCLK频率
- VDRIVE=3.3V时最大17MHz
- VDRIVE=5V时最大23.5MHz
- 建议设置为最大值的70-80%以确保稳定

### Q3: 为什么需要软件控制CS？
**A**: AD7606的CS信号需要精确控制，每个通道读取时需要单独的CS脉冲，硬件NSS无法满足要求。

### Q4: 可以使用DMA吗？
**A**: 可以，但需要注意：
- 每个通道需要16个SCLK周期
- 8个通道共需要128个SCLK周期
- 需要正确配置DMA传输长度和CS控制

### Q5: DOUTB需要连接吗？
**A**: 
- **低速应用（≤100kSPS）**：可以不连接，只使用DOUTA
- **高速应用（200kSPS）**：**必须连接DOUTB**，使用双DOUT并行读取才能满足实时性要求

### Q6: SPI能否实现200kSPS实时采集？
**A**: 
- **单DOUT线**：❌ 无法实现，最大约100kSPS
- **双DOUT线**：✅ 可以实现，可达200-250kSPS
- **关键配置**：
  - 必须使用DOUTA + DOUTB
  - 必须使用"转换中读取"模式
  - SPI时钟建议15 MHz
  - 推荐使用DMA传输

---

## 七、参考配置示例

### 引脚分配示例（SPI2）

| 功能 | STM32H743引脚 | CubeMX配置 |
|------|--------------|-----------|
| SPI2_SCK | PB13 | SPI2_SCK |
| SPI2_MISO | PB14 | SPI2_MISO |
| AD7606_CS | PE13 | GPIO_Output |
| AD7606_CONVST | PE11 | GPIO_Output |
| AD7606_RESET | PE12 | GPIO_Output |
| AD7606_RANGE | PE10 | GPIO_Output |
| AD7606_OS0 | PE7 | GPIO_Output |
| AD7606_OS1 | PE8 | GPIO_Output |
| AD7606_OS2 | PE9 | GPIO_Output |
| AD7606_BUSY | PE15 | GPIO_Input |

---

**提示：** 配置完成后，建议先用较低的SPI时钟频率测试，确认通信正常后再逐步提高频率。

