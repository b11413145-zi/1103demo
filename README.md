# 🍅 STM32 懲罰番茄鐘 (Pomodoro Punish Timer)

基於 **STM32Nucleo-G474RE** 開發板與 STM32CubeIDE 開發的嚴格時間管理系統。本專案整合了多重硬體定時器、PWM 呼吸燈、蜂鳴器音頻輸出、序列埠命令列解析 (CLI) 以及外部中斷按鈕解鎖機制。

---

## 🛠️ 硬體架構與接腳配置 (Hardware Pinout)

| 周邊功能 | STM32 腳位 | 硬體對應 / 說明 |
| :--- | :--- | :--- |
| **綠色 LED 呼吸燈** | `PA5` | 內建 LD2 綠燈 (透過 TIM2 Channel 1 輸出 PWM)[cite: 4, 5] |
| **蜂鳴器警報器** | `PB6` | 外部連接蜂鳴器 (透過 TIM4 Channel 1 輸出 1kHz 音頻)[cite: 4, 5] |
| **序列埠通訊** | `PA2` / `PA3` | LPUART1 (透過 ST-Link 虛擬 COM Port 連接電腦 PuTTY)[cite: 4, 5] |
| **解除懲罰按鈕** | `PC13` | 內建藍色使用者按鈕 (具備 EXTI 外部中斷與防彈跳)[cite: 4, 5] |

---

## ⚙️ 核心技術規格 (Technical Specs)

- **系統時脈 (Clock)**：使用內部高速時鐘 **HSI 16MHz**，不使用 PLL，確保時間與鮑率計算絕對穩定[cite: 4, 5]。
- **TIM2 (PWM 呼吸燈)**：Prescaler = $16-1$，Period = $10000-1$，產生 **100Hz** 的 PWM 訊號，實現平滑呼吸與爆閃效果[cite: 5]。
- **TIM3 (系統心跳)**：Prescaler = $16000-1$，Period = $1000-1$，精準產生 **1Hz (1秒)** 的硬體定時中斷，掌管所有倒數計時[cite: 5]。
- **TIM4 (蜂鳴器 PWM)**：Prescaler = $16-1$，Period = $1000-1$，產生 **1kHz** 人耳最敏感的警報音頻[cite: 5]。
- **UART 通訊**：設定為 `115200 8N1`，搭配 `RxCpltCallback` 接收中斷與字串緩衝區[cite: 5]。

---

## 🔄 系統狀態機流程 (State Machine)

本專案採用嚴格的狀態機設計，支援以下 7 種運行模式：

1. **`STATE_IDLE` (待機模式)**：開機預設狀態，LED 熄滅，等待輸入。
2. **`STATE_FOCUS` (專注模式)**：輸入 `focus` 後啟動 25 分鐘倒數，LED 呈現 5~8 秒週期的平滑呼吸燈[cite: 4, 5]。
3. **`STATE_REST` (休息模式)**：專注結束後自動切換，5 分鐘倒數，LED 呈現輕快雙閃特效[cite: 4, 5]。
4. **`STATE_PAUSE` (暫停模式)**：隨時輸入 `pause` 凍結時間，LED 維持低亮度恆亮，再次輸入可恢復[cite: 4, 5]。
5. **`STATE_TEST` (測試模式)**：輸入 `test` 將倒數縮短為 10 秒，方便快速驗證流程[cite: 4, 5]。
6. **`STATE_PUNISH_WAIT` (等待懲罰)**：休息結束後的 5 秒寬限期，蜂鳴器每秒短鳴 1 次發出警告。
7. **`STATE_PUNISH` (懲罰模式)**：寬限期過後未點擊按鈕即觸發。LED 快速爆閃，蜂鳴器長鳴狂叫[cite: 4, 5]。
   - **救贖機制**：在懲罰或等待期間，按下板子上的 **藍色按鈕 (PC13)** 即可觸發 EXTI 外部中斷，強制解除懲罰並重回專注模式！

---

## 💻 PuTTY 指令操作手冊 (CLI Commands)

透過序列埠終端機（如 PuTTY，鮑率設為 `115200`）連線至開發板，可輸入以下指令：

- `focus`：啟動 25 分鐘專注模式。
- `rest`：啟動 5 分鐘休息模式。
- `test`：啟動 10 秒快速測試模式。
- `pause`：暫停 / 恢復計時器。
- `idle`：回到待機模式，停止計時。

---

## 🚀 專案建置與燒錄 (Getting Started)

1. 使用 **STM32CubeIDE** 開啟本專案。
2. 確認 `.ioc` 設定中 HSI 16MHz、TIM2/3/4 與 LPUART1 參數正確。
3. 點擊 **Build Project** 編譯專案（確保 0 Errors）。
4. 將 Nucleo-G474RE 開發板透過 USB 連接電腦，點擊 **Run -> Run As -> STM32 C/C++ Application** 燒錄至晶片。
