# AD7606 SPI配置快速参考

## 一、硬件连接要点

### 接口模式选择（重要！）
- **PAR/SER/BYTE SEL**: 接**高电平**（VDRIVE）
- **DB15**: 接**低电平**（GND）- 选择串行接口模式

### SPI信号连接
```
AD7606          STM32H743
--------        ----------
CS       →      GPIO输出（软件控制）
RD/SCLK  →      SPIx_SCK
DB7/DOUTA →     SPIx_MISO
DB8/DOUTB →     可选（不接也可以）
```

---

## 二、CubeMX配置要点

### SPI基本配置

| 参数 | 推荐值 |
|------|--------|
| Mode | Full-Duplex Master |
| Hardware NSS Signal | Disable |
| Data Size | **16 Bits** |
| First Bit | MSB First |
| Clock Polarity (CPOL) | **High** |
| Clock Phase (CPHA) | **2 Edge** |
| Prescaler | 8 (3.3V) 或 4 (5V) |

**SPI Mode = Mode 3** (CPOL=High, CPHA=2Edge)

### GPIO配置

| 信号 | 模式 | 初始状态 |
|------|------|----------|
| SPIx_SCK | SPI | 自动配置 |
| SPIx_MISO | SPI | 自动配置 |
| CS | GPIO_Output | High |
| CONVST | GPIO_Output | High |
| RESET | GPIO_Output | Low |
| RANGE | GPIO_Output | Low |
| OS0-OS2 | GPIO_Output | Low |
| BUSY | GPIO_Input | - |

---

## 三、关键时序参数

### AD7606 SPI限制
- **VDRIVE = 3.3V**: 最大SCLK = **17 MHz**
- **VDRIVE = 5V**: 最大SCLK = **23.5 MHz**

### 时钟频率计算
```
SPI时钟 = APB时钟 / Prescaler

示例（APB=120MHz）:
Prescaler=8  →  15MHz  (适合3.3V)
Prescaler=4  →  30MHz  (太高，不推荐)
```

---

## 四、代码示例

### 基本读取函数
```c
// CS控制
#define AD7606_CS_LOW()    HAL_GPIO_WritePin(AD7606_CS_GPIO_Port, AD7606_CS_Pin, GPIO_PIN_RESET)
#define AD7606_CS_HIGH()   HAL_GPIO_WritePin(AD7606_CS_GPIO_Port, AD7606_CS_Pin, GPIO_PIN_SET)

// 读取一个通道
uint16_t AD7606_ReadChannel(void)
{
    uint16_t data = 0;
    AD7606_CS_LOW();
    HAL_SPI_Receive(&hspi1, (uint8_t*)&data, 1, HAL_MAX_DELAY);
    AD7606_CS_HIGH();
    return data;
}

// 读取8个通道
void AD7606_ReadChannels(uint16_t *channels)
{
    for (int i = 0; i < 8; i++) {
        channels[i] = AD7606_ReadChannel();
    }
}
```

---

## 五、配置检查清单

- [ ] PAR/SER/BYTE SEL = 高电平
- [ ] DB15 = 低电平（GND）
- [ ] SPI Data Size = 16 Bits
- [ ] CPOL = High, CPHA = 2 Edge (Mode 3)
- [ ] SPI时钟 ≤ 17MHz (3.3V) 或 ≤ 23.5MHz (5V)
- [ ] CS配置为GPIO输出（软件控制）
- [ ] 所有控制GPIO已配置

---

## 六、常见问题

**Q: 数据读取不正确？**
- 检查SPI模式（推荐Mode 3）
- 检查CS时序
- 降低SPI时钟频率测试

**Q: 转换速度慢？**
- 检查过采样设置
- 优化SPI时钟频率（在限制范围内）
- 考虑使用DOUTB并行读取

**Q: 如何选择SPI外设？**
- 根据硬件连接选择SPI1/SPI2/SPI3等
- 确保引脚不冲突
- 检查时钟源和频率

---

**详细配置请参考：AD7606_SPI配置指南.md**

