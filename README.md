# 1103demo# STM32 Pomodoro Punish Timer
基於 Nucleo-G474RE 開發的嚴格番茄鐘，具備呼吸燈、計時中斷、蜂鳴器與按鈕解鎖機制。
- 核心功能：專注(25分)、休息(5分)、超時懲罰爆閃狂叫
- 硬體配置：HSI 16MHz 時脈
- 通訊：LPUART1 (115200 8N1)
- 計時器：TIM2 (100Hz PWM 呼吸燈)、TIM3 (1Hz 精準倒數)、TIM4 (1kHz PWM 蜂鳴器)
- 中斷：PC13 外部按鈕 (含軟體防彈跳)
