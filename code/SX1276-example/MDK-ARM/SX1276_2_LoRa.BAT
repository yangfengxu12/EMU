SET PATH=F:\Keil_v5\ARM\ARMCC\Bin;C:\Program Files (x86)\Common Files\Oracle\Java\javapath;F:\VMware Workstation\bin\;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Windows\System32\OpenSSH\;F:\platform-tools;C:\Program Files\Git\cmd;C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility;F:\Cadence\SPB_16.6\openaccess\bin\win32\opt;F:\Cadence\SPB_16.6\tools\capture;F:\Cadence\SPB_16.6\tools\pspice;F:\Cadence\SPB_16.6\tools\specctra\bin;F:\Cadence\SPB_16.6\tools\fet\bin;F:\Cadence\SPB_16.6\tools\libutil\bin;F:\Cadence\SPB_16.6\tools\bin;F:\Cadence\SPB_16.6\tools\pcb\bin;C:\Users\yfx\AppData\Local\Programs\Python\Python39\Scripts\;C:\Users\yfx\AppData\Local\Programs\Python\Python39\;C:\Users\yfx\AppData\Local\Microsoft\WindowsApps;C:\Program Files\Bandizip\;C:\Users\yfx\AppData\Local\GitHubDesktop\bin;F:\Microsoft VS Code\bin;F:\texlive\2020\bin\win32
SET CPU_TYPE=STM32L476RGTx
SET CPU_VENDOR=STMicroelectronics
SET UV2_TARGET=SX1276_2_LoRa
SET CPU_CLOCK=0x007A1200
"F:\Keil_v5\ARM\ARMCC\Bin\ArmAsm" --Via ".\sx1276_2_lora\startup_stm32l476xx._ia"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\sx1276_2_lora\main.__i"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\sx1276_2_lora\stm32l4xx_it.__i"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\sx1276_2_lora\stm32l4xx_hal_msp.__i"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\sx1276_2_lora\stm32l4xx_hal_spi.__i"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\sx1276_2_lora\stm32l4xx_hal_spi_ex.__i"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\sx1276_2_lora\stm32l4xx_hal_tim.__i"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\sx1276_2_lora\stm32l4xx_hal_tim_ex.__i"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\sx1276_2_lora\stm32l4xx_hal_uart.__i"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\sx1276_2_lora\stm32l4xx_hal_uart_ex.__i"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\sx1276_2_lora\stm32l4xx_hal.__i"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\sx1276_2_lora\stm32l4xx_hal_i2c.__i"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\sx1276_2_lora\stm32l4xx_hal_i2c_ex.__i"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\sx1276_2_lora\stm32l4xx_hal_rcc.__i"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\sx1276_2_lora\stm32l4xx_hal_rcc_ex.__i"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\sx1276_2_lora\stm32l4xx_hal_flash.__i"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\sx1276_2_lora\stm32l4xx_hal_flash_ex.__i"
"F:\Keil_v5\ARM\ARMCC\Bin\ArmLink" --Via ".\SX1276_2_LoRa\OOK2Chirp_Project.lnp"
"F:\Keil_v5\ARM\ARMCC\Bin\fromelf.exe" ".\SX1276_2_LoRa\OOK2Chirp_Project.axf" --i32combined --output ".\SX1276_2_LoRa\OOK2Chirp_Project.hex"
