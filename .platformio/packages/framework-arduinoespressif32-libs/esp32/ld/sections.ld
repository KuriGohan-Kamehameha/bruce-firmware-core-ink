/* Automatically generated file; DO NOT EDIT */
/* Espressif IoT Development Framework Linker Script */
/* Generated from: /home/runner/work/esp32-arduino-lib-builder/esp32-arduino-lib-builder/build/esp-idf/esp_system/ld/sections.ld.in */

/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/*
 * Automatically generated file. DO NOT EDIT.
 * Espressif IoT Development Framework (ESP-IDF) 5.5.2 Configuration Header
 */
       
/* List of deprecated options */
/* CPU instruction prefetch padding size for flash mmap scenario */
/*
 * PMP region granularity size
 * Software may determine the PMP granularity by writing zero to pmp0cfg, then writing all ones
 * to pmpaddr0, then reading back pmpaddr0. If G is the index of the least-significant bit set,
 * the PMP granularity is 2^G+2 bytes.
 */
/* CPU instruction prefetch padding size for memory protection scenario */
/* Memory alignment size for PMS */
        /**
         * The ESP_BOOTLOADER_RESERVE_RTC size must have the same alignment of RTC_TIMER_RESERVE_RTC, else
         * the segment will overflow at link time because not enough bytes are allocated for the RTC segment.
         */
    /* rtc timer data (s_rtc_timer_retain_mem, see esp_clk.c files). For rtc_timer_data_in_rtc_mem section. */
/* Default entry point */
ENTRY(call_start_cpu0);
SECTIONS
{
  /**
   * RTC fast memory holds RTC wake stub code,
   * including from any source file named rtc_wake_stub*.c
   */
  .rtc.text :
  {
    
 . = ALIGN(4); 
 _rtc_text_start = ABSOLUTE(.);
    *(.rtc.literal .rtc.text .rtc.text.*)
    *rtc_wake_stub*.*(.literal .text .literal.* .text.*)
    _rtc_text_end = ABSOLUTE(.);
  } > rtc_iram_seg
  /**
   * This section is required to skip rtc.text area because rtc_iram_seg and
   * rtc_data_seg are reflect the same address space on different buses.
   */
  .rtc.dummy :
  {
    _rtc_dummy_start = ABSOLUTE(.);
    _rtc_fast_start = ABSOLUTE(.);
    . = SIZEOF(.rtc.text);
    _rtc_dummy_end = ABSOLUTE(.);
  } > rtc_data_seg
  /**
   * This section located in RTC FAST Memory area.
   * It holds data marked with RTC_FAST_ATTR attribute.
   * See the file "esp_attr.h" for more information.
   */
  .rtc.force_fast :
  {
    
 . = ALIGN(4); 
 _rtc_force_fast_start = ABSOLUTE(.);
    _coredump_rtc_fast_start = ABSOLUTE(.);
    *(.rtc.fast.coredump .rtc.fast.coredump.*)
    _coredump_rtc_fast_end = ABSOLUTE(.);
    *(.rtc.force_fast .rtc.force_fast.*)
    
 . = ALIGN(4); 
 _rtc_force_fast_end = ABSOLUTE(.);
  } > rtc_data_seg
  /**
   * RTC data section holds RTC wake stub
   * data/rodata, including from any source file
   * named rtc_wake_stub*.c and the data marked with
   * RTC_DATA_ATTR, RTC_RODATA_ATTR attributes.
   * The memory location of the data is dependent on
   * CONFIG_ESP32_RTCDATA_IN_FAST_MEM option.
   */
  .rtc.data :
  {
    _rtc_data_start = ABSOLUTE(.);
    _coredump_rtc_start = ABSOLUTE(.);
    *(.rtc.coredump .rtc.coredump.*)
    _coredump_rtc_end = ABSOLUTE(.);
    *(.rtc.data .rtc.data.*)
    *(.rtc.rodata .rtc.rodata.*)
    *rtc_wake_stub*.*(.data .rodata .data.* .rodata.*)
    _rtc_data_end = ABSOLUTE(.);
  } > rtc_data_location
  /* RTC bss, from any source file named rtc_wake_stub*.c */
  .rtc.bss (NOLOAD) :
  {
    _rtc_bss_start = ABSOLUTE(.);
    *rtc_wake_stub*.*(.bss .bss.*)
    *rtc_wake_stub*.*(COMMON)
    *(.rtc.bss)
    _rtc_bss_end = ABSOLUTE(.);
  } > rtc_data_location
  /**
   * This section holds data that should not be initialized at power up
   * and will be retained during deep sleep.
   * User data marked with RTC_NOINIT_ATTR will be placed
   * into this section. See the file "esp_attr.h" for more information.
   * The memory location of the data is dependent on
   * CONFIG_ESP32_RTCDATA_IN_FAST_MEM option.
   */
  .rtc_noinit (NOLOAD):
  {
    
 . = ALIGN(4); 
 _rtc_noinit_start = ABSOLUTE(.);
    *(.rtc_noinit .rtc_noinit.*)
    
 . = ALIGN(4); 
 _rtc_noinit_end = ABSOLUTE(.);
  } > rtc_data_location
  /**
   * This section located in RTC SLOW Memory area.
   * It holds data marked with RTC_SLOW_ATTR attribute.
   * See the file "esp_attr.h" for more information.
   */
  .rtc.force_slow :
  {
    
 . = ALIGN(4); 
 _rtc_force_slow_start = ABSOLUTE(.);
    *(.rtc.force_slow .rtc.force_slow.*)
    
 . = ALIGN(4); 
 _rtc_force_slow_end = ABSOLUTE(.);
  } > rtc_slow_seg
  /**
   * This section holds RTC FAST data that should have fixed addresses.
   * The data are not initialized at power-up and are retained during deep
   * sleep.
   */
  .rtc_fast_reserved (NOLOAD):
  {
    
 . = ALIGN(4); 
 _rtc_fast_reserved_start = ABSOLUTE(.);
    /**
     * New data can only be added here to ensure existing data are not moved.
     * Because data have adhered to the end of the segment and code is relied
     * on it.
     * >> put new data here <<
     */
    KEEP(*(.bootloader_data_rtc_mem .bootloader_data_rtc_mem.*))
    _rtc_fast_reserved_end = ABSOLUTE(.);
  } > rtc_fast_reserved_seg
  _rtc_fast_reserved_length = _rtc_fast_reserved_end - _rtc_fast_reserved_start;
  ASSERT((_rtc_fast_reserved_length <= LENGTH(rtc_fast_reserved_seg)),
          "RTC FAST reserved segment data does not fit.")
  /**
   * This section holds RTC SLOW data that should have fixed addresses.
   * The data are not initialized at power-up and are retained during deep
   * sleep.
   */
  .rtc_slow_reserved (NOLOAD):
  {
    
 . = ALIGN(4); 
 _rtc_slow_reserved_start = ABSOLUTE(.);
    /**
     * New data can only be added here to ensure existing data are not moved.
     * Because data have adhered to the end of the segment and code is relied
     * on it.
     * >> put new data here <<
     */
    *(.rtc_timer_data_in_rtc_mem .rtc_timer_data_in_rtc_mem.*)
    _rtc_slow_reserved_end = ABSOLUTE(.);
  } > rtc_slow_reserved_seg
  _rtc_slow_reserved_length = _rtc_slow_reserved_end - _rtc_slow_reserved_start;
  _rtc_reserved_length = _rtc_slow_reserved_length;
  ASSERT((_rtc_slow_reserved_length <= LENGTH(rtc_slow_reserved_seg)),
          "RTC SLOW reserved segment data does not fit.")
  /* Get size of rtc slow data based on rtc_data_location alias */
  _rtc_slow_length = (ORIGIN(rtc_slow_seg) == ORIGIN(rtc_data_location))
                        ? (_rtc_force_slow_end - _rtc_data_start)
                        : (_rtc_force_slow_end - _rtc_force_slow_start);
  _rtc_fast_length = (ORIGIN(rtc_slow_seg) == ORIGIN(rtc_data_location))
                        ? (_rtc_force_fast_end - _rtc_fast_start)
                        : (_rtc_noinit_end - _rtc_fast_start);
  ASSERT((_rtc_slow_length <= LENGTH(rtc_slow_seg)),
          "RTC_SLOW segment data does not fit.")
  ASSERT((_rtc_fast_length <= LENGTH(rtc_data_seg)),
          "RTC_FAST segment data does not fit.")
  /* Send .iram0 code to iram */
  .iram0.vectors :
  {
    _iram_start = ABSOLUTE(.);
    /* Vectors go to IRAM */
    _vector_table = ABSOLUTE(.);
    . = 0x0;
    KEEP(*(.WindowVectors.text));
    . = 0x180;
    KEEP(*(.Level2InterruptVector.text));
    . = 0x1c0;
    KEEP(*(.Level3InterruptVector.text));
    . = 0x200;
    KEEP(*(.Level4InterruptVector.text));
    . = 0x240;
    KEEP(*(.Level5InterruptVector.text));
    . = 0x280;
    KEEP(*(.DebugExceptionVector.text));
    . = 0x2c0;
    KEEP(*(.NMIExceptionVector.text));
    . = 0x300;
    KEEP(*(.KernelExceptionVector.text));
    . = 0x340;
    KEEP(*(.UserExceptionVector.text));
    . = 0x3C0;
    KEEP(*(.DoubleExceptionVector.text));
    . = 0x400;
    KEEP(*(._invalid_pc_placeholder.text));
    *(.*Vector.literal)
  } > iram0_0_seg
  .iram0.text :
  {
    /* Code marked as running out of IRAM */
    _iram_text_start = ABSOLUTE(.);
    *(.iram1 .iram1.*)
    *libapp_trace.a:app_trace.*(.literal .literal.* .text .text.*)
    *libapp_trace.a:app_trace_util.*(.literal .literal.* .text .text.*)
    *libapp_trace.a:port_uart.*(.literal .literal.* .text .text.*)
    *libesp_driver_gptimer.a:gptimer.*(.literal.gptimer_default_isr .text.gptimer_default_isr)
    *libesp_driver_i2c.a:i2c_master.*(.literal.i2c_master_isr_handler_default .text.i2c_master_isr_handler_default)
    *libesp_driver_mcpwm.a:mcpwm_cap.*(.literal.mcpwm_capture_default_isr .text.mcpwm_capture_default_isr)
    *libesp_driver_mcpwm.a:mcpwm_cmpr.*(.literal.mcpwm_comparator_default_isr .text.mcpwm_comparator_default_isr)
    *libesp_driver_mcpwm.a:mcpwm_fault.*(.literal.mcpwm_gpio_fault_default_isr .text.mcpwm_gpio_fault_default_isr)
    *libesp_driver_mcpwm.a:mcpwm_oper.*(.literal.mcpwm_operator_default_isr .text.mcpwm_operator_default_isr)
    *libesp_driver_mcpwm.a:mcpwm_timer.*(.literal.mcpwm_timer_default_isr .text.mcpwm_timer_default_isr)
    *libesp_driver_rmt.a:rmt_encoder.*(.literal.rmt_encoder_reset .text.rmt_encoder_reset)
    *libesp_driver_rmt.a:rmt_rx.*(.literal.rmt_isr_handle_rx_done .text.rmt_isr_handle_rx_done)
    *libesp_driver_rmt.a:rmt_rx.*(.literal.rmt_rx_default_isr .text.rmt_rx_default_isr)
    *libesp_driver_rmt.a:rmt_tx.*(.literal.rmt_encode_check_result .text.rmt_encode_check_result)
    *libesp_driver_rmt.a:rmt_tx.*(.literal.rmt_isr_handle_tx_done .text.rmt_isr_handle_tx_done)
    *libesp_driver_rmt.a:rmt_tx.*(.literal.rmt_isr_handle_tx_threshold .text.rmt_isr_handle_tx_threshold)
    *libesp_driver_rmt.a:rmt_tx.*(.literal.rmt_tx_default_isr .text.rmt_tx_default_isr)
    *libesp_driver_rmt.a:rmt_tx.*(.literal.rmt_tx_do_transaction .text.rmt_tx_do_transaction)
    *libesp_driver_rmt.a:rmt_tx.*(.literal.rmt_tx_mark_eof .text.rmt_tx_mark_eof)
    *libesp_event.a:default_event_loop.*(.literal.esp_event_isr_post .text.esp_event_isr_post)
    *libesp_event.a:esp_event.*(.literal.esp_event_isr_post_to .text.esp_event_isr_post_to)
    *libesp_hw_support.a:adc_share_hw_ctrl.*(.literal.adc_apb_periph_claim .text.adc_apb_periph_claim)
    *libesp_hw_support.a:adc_share_hw_ctrl.*(.literal.adc_apb_periph_free .text.adc_apb_periph_free)
    *libesp_hw_support.a:clk_utils.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:cpu.*(.literal.esp_cpu_compare_and_set .text.esp_cpu_compare_and_set)
    *libesp_hw_support.a:cpu.*(.literal.esp_cpu_reset .text.esp_cpu_reset)
    *libesp_hw_support.a:cpu.*(.literal.esp_cpu_stall .text.esp_cpu_stall)
    *libesp_hw_support.a:cpu.*(.literal.esp_cpu_unstall .text.esp_cpu_unstall)
    *libesp_hw_support.a:cpu.*(.literal.esp_cpu_wait_for_intr .text.esp_cpu_wait_for_intr)
    *libesp_hw_support.a:esp_clk.*(.literal.esp_clk_private_lock .text.esp_clk_private_lock)
    *libesp_hw_support.a:esp_clk.*(.literal.esp_clk_private_unlock .text.esp_clk_private_unlock)
    *libesp_hw_support.a:esp_clk.*(.literal.esp_clk_slowclk_cal_get .text.esp_clk_slowclk_cal_get)
    *libesp_hw_support.a:esp_clk.*(.literal.esp_clk_slowclk_cal_set .text.esp_clk_slowclk_cal_set)
    *libesp_hw_support.a:esp_clk.*(.literal.esp_rtc_get_time_us .text.esp_rtc_get_time_us)
    *libesp_hw_support.a:esp_clk_tree.*(.literal.esp_clk_tree_enable_src .text.esp_clk_tree_enable_src)
    *libesp_hw_support.a:esp_memory_utils.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:mspi_timing_tuning.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_module_reset .text.periph_module_reset)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_rcc_acquire_enter .text.periph_rcc_acquire_enter)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_rcc_acquire_exit .text.periph_rcc_acquire_exit)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_rcc_enter .text.periph_rcc_enter)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_rcc_exit .text.periph_rcc_exit)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_rcc_release_enter .text.periph_rcc_release_enter)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_rcc_release_exit .text.periph_rcc_release_exit)
    *libesp_hw_support.a:periph_ctrl.*(.literal.wifi_module_disable .text.wifi_module_disable)
    *libesp_hw_support.a:periph_ctrl.*(.literal.wifi_module_enable .text.wifi_module_enable)
    *libesp_hw_support.a:regi2c_ctrl.*(.literal.regi2c_ctrl_read_reg .text.regi2c_ctrl_read_reg)
    *libesp_hw_support.a:regi2c_ctrl.*(.literal.regi2c_ctrl_read_reg_mask .text.regi2c_ctrl_read_reg_mask)
    *libesp_hw_support.a:regi2c_ctrl.*(.literal.regi2c_ctrl_write_reg .text.regi2c_ctrl_write_reg)
    *libesp_hw_support.a:regi2c_ctrl.*(.literal.regi2c_ctrl_write_reg_mask .text.regi2c_ctrl_write_reg_mask)
    *libesp_hw_support.a:regi2c_ctrl.*(.literal.regi2c_enter_critical .text.regi2c_enter_critical)
    *libesp_hw_support.a:regi2c_ctrl.*(.literal.regi2c_exit_critical .text.regi2c_exit_critical)
    *libesp_hw_support.a:regi2c_ctrl.*(.literal.regi2c_saradc_disable .text.regi2c_saradc_disable)
    *libesp_hw_support.a:regi2c_ctrl.*(.literal.regi2c_saradc_enable .text.regi2c_saradc_enable)
    *libesp_hw_support.a:rtc_clk.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:rtc_init.*(.literal.rtc_vddsdio_get_config .text.rtc_vddsdio_get_config)
    *libesp_hw_support.a:rtc_init.*(.literal.rtc_vddsdio_set_config .text.rtc_vddsdio_set_config)
    *libesp_hw_support.a:rtc_sleep.*(.literal.rtc_sleep_get_default_config .text.rtc_sleep_get_default_config)
    *libesp_hw_support.a:rtc_sleep.*(.literal.rtc_sleep_init .text.rtc_sleep_init)
    *libesp_hw_support.a:rtc_sleep.*(.literal.rtc_sleep_low_init .text.rtc_sleep_low_init)
    *libesp_hw_support.a:rtc_sleep.*(.literal.rtc_sleep_pd .text.rtc_sleep_pd)
    *libesp_hw_support.a:rtc_sleep.*(.literal.rtc_sleep_start .text.rtc_sleep_start)
    *libesp_hw_support.a:rtc_time.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:rtc_wdt.*(.literal .literal.* .text .text.*)
    *libesp_hw_support.a:sar_periph_ctrl.*(.literal.sar_periph_ctrl_power_disable .text.sar_periph_ctrl_power_disable)
    *libesp_hw_support.a:sar_periph_ctrl.*(.literal.sar_periph_ctrl_power_enable .text.sar_periph_ctrl_power_enable)
    *libesp_hw_support.a:sleep_modem.*(.literal.modem_domain_pd_allowed .text.modem_domain_pd_allowed)
    *libesp_hw_support.a:sleep_modem.*(.literal.periph_inform_out_light_sleep_overhead .text.periph_inform_out_light_sleep_overhead)
    *libesp_hw_support.a:sleep_modem.*(.literal.sleep_modem_reject_triggers .text.sleep_modem_reject_triggers)
    *libesp_hw_support.a:sleep_modes.*(.literal.esp_light_sleep_start .text.esp_light_sleep_start)
    *libesp_hw_support.a:sleep_modes.*(.literal.esp_sleep_enable_timer_wakeup .text.esp_sleep_enable_timer_wakeup)
    *libesp_mm.a:cache_esp32.*(.literal .literal.* .text .text.*)
    *libesp_mm.a:esp_cache_msync.*(.literal .literal.* .text .text.*)
    *libesp_mm.a:esp_cache_utils.*(.literal .literal.* .text .text.*)
    *libesp_phy.a:phy_override.*(.literal.phy_get_tsens_value .text.phy_get_tsens_value)
    *libesp_pm.a:pm_impl.*(.literal.esp_pm_impl_get_cpu_freq .text.esp_pm_impl_get_cpu_freq)
    *libesp_ringbuf.a:(EXCLUDE_FILE(*libesp_ringbuf.a:ringbuf.*) .literal EXCLUDE_FILE(*libesp_ringbuf.a:ringbuf.*) .literal.* EXCLUDE_FILE(*libesp_ringbuf.a:ringbuf.*) .text EXCLUDE_FILE(*libesp_ringbuf.a:ringbuf.*) .text.*)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvAcquireItemNoSplit .literal.prvCheckItemFitsByteBuffer .literal.prvCheckItemFitsDefault .literal.prvCopyItemAllowSplit .literal.prvCopyItemByteBuf .literal.prvCopyItemNoSplit .literal.prvGetItemByteBuf .literal.prvGetItemDefault .literal.prvReceiveGenericFromISR .literal.prvReturnItemByteBuf .literal.prvReturnItemDefault .literal.prvSendItemDoneNoSplit .literal.vRingbufferDeleteWithCaps .literal.vRingbufferReturnItemFromISR .literal.xRingbufferCreateWithCaps .literal.xRingbufferGetStaticBuffer .literal.xRingbufferReceiveFromISR .literal.xRingbufferReceiveSplitFromISR .literal.xRingbufferReceiveUpToFromISR .literal.xRingbufferSendFromISR .text .text.prvAcquireItemNoSplit .text.prvCheckItemAvail .text.prvCheckItemFitsByteBuffer .text.prvCheckItemFitsDefault .text.prvCopyItemAllowSplit .text.prvCopyItemByteBuf .text.prvCopyItemNoSplit .text.prvGetItemByteBuf .text.prvGetItemDefault .text.prvReceiveGenericFromISR .text.prvReturnItemByteBuf .text.prvReturnItemDefault .text.prvSendItemDoneNoSplit .text.vRingbufferDeleteWithCaps .text.vRingbufferReturnItemFromISR .text.xRingbufferCreateWithCaps .text.xRingbufferGetStaticBuffer .text.xRingbufferReceiveFromISR .text.xRingbufferReceiveSplitFromISR .text.xRingbufferReceiveUpToFromISR .text.xRingbufferSendFromISR)
    *libesp_rom.a:esp_rom_print.*(.literal .literal.* .text .text.*)
    *libesp_rom.a:esp_rom_spiflash.*(.literal .literal.* .text .text.*)
    *libesp_rom.a:esp_rom_sys.*(.literal .literal.* .text .text.*)
    *libesp_system.a:esp_err.*(.literal .literal.* .text .text.*)
    *libesp_system.a:esp_system_chip.*(.literal.esp_restart_noos_dig .text.esp_restart_noos_dig)
    *libesp_system.a:esp_system_chip.*(.literal.esp_system_abort .text.esp_system_abort)
    *libesp_system.a:freertos_hooks.*(.literal.esp_vApplicationTickHook .text.esp_vApplicationTickHook)
    *libesp_system.a:image_process.*(.literal .literal.* .text .text.*)
    *libesp_system.a:panic.*(.literal.panic_abort .text.panic_abort)
    *libesp_system.a:reset_reason.*(.literal.esp_reset_reason_set_hint .text.esp_reset_reason_set_hint)
    *libesp_system.a:stack_check.*(.literal.__stack_chk_fail .text.__stack_chk_fail)
    *libesp_system.a:system_internal.*(.literal.esp_restart_noos .text.esp_restart_noos)
    *libesp_system.a:system_internal.*(.literal.esp_system_reset_modules_on_exit .text.esp_system_reset_modules_on_exit)
    *libesp_system.a:system_time.*(.literal.esp_system_get_time .text.esp_system_get_time)
    *libesp_system.a:system_time.*(.literal.esp_system_get_time_resolution .text.esp_system_get_time_resolution)
    *libesp_system.a:ubsan.*(.literal .literal.* .text .text.*)
    *libesp_timer.a:esp_timer_impl_common.*(.literal.esp_timer_impl_lock .text.esp_timer_impl_lock)
    *libesp_timer.a:esp_timer_impl_common.*(.literal.esp_timer_impl_unlock .text.esp_timer_impl_unlock)
    *libesp_timer.a:esp_timer_impl_lac.*(.literal.esp_timer_impl_advance .text.esp_timer_impl_advance)
    *libesp_timer.a:esp_timer_impl_lac.*(.literal.esp_timer_impl_set .text.esp_timer_impl_set)
    *libfreertos.a:(EXCLUDE_FILE(*libfreertos.a:app_startup.* *libfreertos.a:event_groups.* *libfreertos.a:freertos_compatibility.* *libfreertos.a:heap_idf.* *libfreertos.a:idf_additions.* *libfreertos.a:idf_additions_event_groups.* *libfreertos.a:list.* *libfreertos.a:port.* *libfreertos.a:port_common.* *libfreertos.a:port_systick.* *libfreertos.a:queue.* *libfreertos.a:stream_buffer.* *libfreertos.a:tasks.* *libfreertos.a:timers.*) .literal EXCLUDE_FILE(*libfreertos.a:app_startup.* *libfreertos.a:event_groups.* *libfreertos.a:freertos_compatibility.* *libfreertos.a:heap_idf.* *libfreertos.a:idf_additions.* *libfreertos.a:idf_additions_event_groups.* *libfreertos.a:list.* *libfreertos.a:port.* *libfreertos.a:port_common.* *libfreertos.a:port_systick.* *libfreertos.a:queue.* *libfreertos.a:stream_buffer.* *libfreertos.a:tasks.* *libfreertos.a:timers.*) .literal.* EXCLUDE_FILE(*libfreertos.a:app_startup.* *libfreertos.a:event_groups.* *libfreertos.a:freertos_compatibility.* *libfreertos.a:heap_idf.* *libfreertos.a:idf_additions.* *libfreertos.a:idf_additions_event_groups.* *libfreertos.a:list.* *libfreertos.a:port.* *libfreertos.a:port_common.* *libfreertos.a:port_systick.* *libfreertos.a:queue.* *libfreertos.a:stream_buffer.* *libfreertos.a:tasks.* *libfreertos.a:timers.*) .text EXCLUDE_FILE(*libfreertos.a:app_startup.* *libfreertos.a:event_groups.* *libfreertos.a:freertos_compatibility.* *libfreertos.a:heap_idf.* *libfreertos.a:idf_additions.* *libfreertos.a:idf_additions_event_groups.* *libfreertos.a:list.* *libfreertos.a:port.* *libfreertos.a:port_common.* *libfreertos.a:port_systick.* *libfreertos.a:queue.* *libfreertos.a:stream_buffer.* *libfreertos.a:tasks.* *libfreertos.a:timers.*) .text.*)
    *libfreertos.a:event_groups.*(.literal.xEventGroupClearBitsFromISR .literal.xEventGroupGetBitsFromISR .literal.xEventGroupSetBitsFromISR .text .text.xEventGroupClearBitsFromISR .text.xEventGroupGetBitsFromISR .text.xEventGroupSetBitsFromISR)
    *libfreertos.a:heap_idf.*(.text)
    *libfreertos.a:list.*(.text .text.uxListRemove .text.vListInsertEnd)
    *libfreertos.a:port.*(.literal.unlikely.vPortEndScheduler .literal.vApplicationStackOverflowHook .literal.vPortAssertIfInISR .literal.vPortExitCritical .literal.vPortExitCriticalCompliance .literal.vPortSetStackWatchpoint .literal.vPortYieldOtherCore .literal.xPortEnterCriticalTimeout .literal.xPortEnterCriticalTimeoutCompliance .literal.xPortInIsrContext .text .text.unlikely.vPortEndScheduler .text.vApplicationStackOverflowHook .text.vPortAssertIfInISR .text.vPortExitCritical .text.vPortExitCriticalCompliance .text.vPortSetStackWatchpoint .text.vPortYieldOtherCore .text.xPortEnterCriticalTimeout .text.xPortEnterCriticalTimeoutCompliance .text.xPortInIsrContext)
    *libfreertos.a:port_common.*(.text)
    *libfreertos.a:port_systick.*(.literal.xPortSysTickHandler .text .text.xPortSysTickHandler)
    *libfreertos.a:queue.*(.literal.prvCopyDataFromQueue .literal.prvCopyDataToQueue .literal.prvNotifyQueueSetContainer .literal.uxQueueMessagesWaitingFromISR .literal.xQueueGenericSendFromISR .literal.xQueueGetMutexHolderFromISR .literal.xQueueGiveFromISR .literal.xQueueIsQueueEmptyFromISR .literal.xQueueIsQueueFullFromISR .literal.xQueuePeekFromISR .literal.xQueueReceiveFromISR .literal.xQueueSelectFromSetFromISR .text .text.prvCopyDataFromQueue .text.prvCopyDataToQueue .text.prvNotifyQueueSetContainer .text.uxQueueMessagesWaitingFromISR .text.xQueueGenericSendFromISR .text.xQueueGetMutexHolderFromISR .text.xQueueGiveFromISR .text.xQueueIsQueueEmptyFromISR .text.xQueueIsQueueFullFromISR .text.xQueuePeekFromISR .text.xQueueReceiveFromISR .text.xQueueSelectFromSetFromISR)
    *libfreertos.a:stream_buffer.*(.literal.prvInitialiseNewStreamBuffer$isra$0 .literal.prvReadMessageFromBuffer .literal.prvWriteMessageToBuffer .literal.xStreamBufferReceiveCompletedFromISR .literal.xStreamBufferReceiveFromISR .literal.xStreamBufferSendCompletedFromISR .literal.xStreamBufferSendFromISR .text .text.prvBytesInBuffer .text.prvInitialiseNewStreamBuffer$isra$0 .text.prvReadMessageFromBuffer .text.prvWriteMessageToBuffer .text.xStreamBufferReceiveCompletedFromISR .text.xStreamBufferReceiveFromISR .text.xStreamBufferSendCompletedFromISR .text.xStreamBufferSendFromISR)
    *libfreertos.a:tasks.*(.literal.prvCheckTaskCanBeScheduledSMP .literal.prvInitialiseNewTask$constprop$0 .literal.prvIsYieldRequiredSMP .literal.prvResetNextTaskUnblockTime .literal.uxTaskPriorityGetFromISR .literal.vTaskGenericNotifyGiveFromISR .literal.vTaskGetSnapshot .literal.vTaskSwitchContext .literal.xTaskGenericNotifyFromISR .literal.xTaskGetSchedulerState .literal.xTaskGetTickCount .literal.xTaskGetTickCountFromISR .literal.xTaskIncrementTick .literal.xTaskIncrementTickOtherCores .literal.xTaskRemoveFromEventList .literal.xTaskResumeFromISR .text .text.prvCheckTaskCanBeScheduledSMP .text.prvInitialiseNewTask$constprop$0 .text.prvIsYieldRequiredSMP .text.prvResetNextTaskUnblockTime .text.uxTaskPriorityGetFromISR .text.vTaskGenericNotifyGiveFromISR .text.vTaskGetSnapshot .text.vTaskSwitchContext .text.xTaskGenericNotifyFromISR .text.xTaskGetSchedulerState .text.xTaskGetTickCount .text.xTaskGetTickCountFromISR .text.xTaskIncrementTick .text.xTaskIncrementTickOtherCores .text.xTaskRemoveFromEventList .text.xTaskResumeFromISR .text.xTimerCreateTimerTask)
    *libfreertos.a:timers.*(.literal.prvInitialiseNewTimer$part$0 .literal.xTimerGenericCommand .literal.xTimerPendFunctionCallFromISR .text .text.prvInitialiseNewTimer$part$0 .text.xTimerGenericCommand .text.xTimerPendFunctionCallFromISR)
    *libgcc.a:lib2funcs.*(.literal .literal.* .text .text.*)
    *libgcov.a:(.literal .literal.* .text .text.*)
    *libhal.a:cache_hal_esp32.*(.literal .literal.* .text .text.*)
    *libhal.a:gpio_hal.*(.literal.gpio_hal_isolate_in_sleep .text.gpio_hal_isolate_in_sleep)
    *libhal.a:i2c_hal_iram.*(.literal .literal.* .text .text.*)
    *libhal.a:ledc_hal_iram.*(.literal .literal.* .text .text.*)
    *libhal.a:mmu_hal.*(.literal .literal.* .text .text.*)
    *libhal.a:spi_flash_encrypt_hal_iram.*(.literal .literal.* .text .text.*)
    *libhal.a:spi_flash_hal_iram.*(.literal .literal.* .text .text.*)
    *libhal.a:timer_hal.*(.literal.timer_hal_capture_and_get_counter_value .text.timer_hal_capture_and_get_counter_value)
    *libhal.a:wdt_hal_iram.*(.literal .literal.* .text .text.*)
    *libheap.a:multi_heap.*(.literal.assert_valid_block .text.assert_valid_block)
    *libheap.a:multi_heap.*(.literal.multi_heap_aligned_alloc_impl .text.multi_heap_aligned_alloc_impl)
    *libheap.a:multi_heap.*(.literal.multi_heap_aligned_alloc_impl_offs .text.multi_heap_aligned_alloc_impl_offs)
    *libheap.a:multi_heap.*(.literal.multi_heap_free_impl .text.multi_heap_free_impl)
    *libheap.a:multi_heap.*(.literal.multi_heap_get_allocated_size_impl .text.multi_heap_get_allocated_size_impl)
    *libheap.a:multi_heap.*(.literal.multi_heap_get_block_address_impl .text.multi_heap_get_block_address_impl)
    *libheap.a:multi_heap.*(.literal.multi_heap_get_first_block .text.multi_heap_get_first_block)
    *libheap.a:multi_heap.*(.literal.multi_heap_get_next_block .text.multi_heap_get_next_block)
    *libheap.a:multi_heap.*(.literal.multi_heap_internal_lock .text.multi_heap_internal_lock)
    *libheap.a:multi_heap.*(.literal.multi_heap_internal_unlock .text.multi_heap_internal_unlock)
    *libheap.a:multi_heap.*(.literal.multi_heap_is_free .text.multi_heap_is_free)
    *libheap.a:multi_heap.*(.literal.multi_heap_malloc_impl .text.multi_heap_malloc_impl)
    *libheap.a:multi_heap.*(.literal.multi_heap_realloc_impl .text.multi_heap_realloc_impl)
    *libheap.a:multi_heap.*(.literal.multi_heap_set_lock .text.multi_heap_set_lock)
    *libheap.a:multi_heap_poisoning.*(.literal.multi_heap_aligned_alloc .text.multi_heap_aligned_alloc)
    *libheap.a:multi_heap_poisoning.*(.literal.multi_heap_aligned_alloc_offs .text.multi_heap_aligned_alloc_offs)
    *libheap.a:multi_heap_poisoning.*(.literal.multi_heap_aligned_free .text.multi_heap_aligned_free)
    *libheap.a:multi_heap_poisoning.*(.literal.multi_heap_free .text.multi_heap_free)
    *libheap.a:multi_heap_poisoning.*(.literal.multi_heap_get_allocated_size .text.multi_heap_get_allocated_size)
    *libheap.a:multi_heap_poisoning.*(.literal.multi_heap_get_block_address .text.multi_heap_get_block_address)
    *libheap.a:multi_heap_poisoning.*(.literal.multi_heap_get_full_block_size .text.multi_heap_get_full_block_size)
    *libheap.a:multi_heap_poisoning.*(.literal.multi_heap_internal_check_block_poisoning .text.multi_heap_internal_check_block_poisoning)
    *libheap.a:multi_heap_poisoning.*(.literal.multi_heap_internal_poison_fill_region .text.multi_heap_internal_poison_fill_region)
    *libheap.a:multi_heap_poisoning.*(.literal.multi_heap_malloc .text.multi_heap_malloc)
    *libheap.a:multi_heap_poisoning.*(.literal.multi_heap_realloc .text.multi_heap_realloc)
    *libheap.a:multi_heap_poisoning.*(.literal.poison_allocated_region .text.poison_allocated_region)
    *libheap.a:multi_heap_poisoning.*(.literal.verify_allocated_region .text.verify_allocated_region)
    *libheap.a:tlsf.*(.literal.tlsf_alloc_overhead .text.tlsf_alloc_overhead)
    *libheap.a:tlsf.*(.literal.tlsf_block_size .text.tlsf_block_size)
    *libheap.a:tlsf.*(.literal.tlsf_free .text.tlsf_free)
    *libheap.a:tlsf.*(.literal.tlsf_get_pool .text.tlsf_get_pool)
    *libheap.a:tlsf.*(.literal.tlsf_malloc .text.tlsf_malloc)
    *libheap.a:tlsf.*(.literal.tlsf_memalign .text.tlsf_memalign)
    *libheap.a:tlsf.*(.literal.tlsf_memalign_offs .text.tlsf_memalign_offs)
    *libheap.a:tlsf.*(.literal.tlsf_realloc .text.tlsf_realloc)
    *libheap.a:tlsf.*(.literal.tlsf_size .text.tlsf_size)
    *liblog.a:log.*(.literal .literal.* .text .text.*)
    *liblog.a:log_format_text.*(.literal .literal.* .text .text.*)
    *liblog.a:log_lock.*(.literal .literal.* .text .text.*)
    *liblog.a:log_print.*(.literal .literal.* .text .text.*)
    *liblog.a:log_timestamp.*(.literal.esp_log_early_timestamp .text.esp_log_early_timestamp)
    *liblog.a:log_timestamp.*(.literal.esp_log_timestamp .text.esp_log_timestamp)
    *liblog.a:log_timestamp_common.*(.literal .literal.* .text .text.*)
    *liblog.a:log_write.*(.literal.esp_log_write .text.esp_log_write)
    *liblog.a:log_write.*(.literal.esp_log_writev .text.esp_log_writev)
    *liblog.a:tag_log_level.*(.literal.esp_log_level_get_timeout .text.esp_log_level_get_timeout)
    *liblog.a:util.*(.literal .literal.* .text .text.*)
    *libnewlib.a:abort.*(.literal .literal.* .text .text.*)
    *libnewlib.a:assert.*(.literal .literal.* .text .text.*)
    *libnewlib.a:esp_time_impl.*(.literal.esp_set_time_from_rtc .text.esp_set_time_from_rtc)
    *libnewlib.a:esp_time_impl.*(.literal.esp_time_impl_get_boot_time .text.esp_time_impl_get_boot_time)
    *libnewlib.a:esp_time_impl.*(.literal.esp_time_impl_set_boot_time .text.esp_time_impl_set_boot_time)
    *libnewlib.a:heap.*(.literal .literal.* .text .text.*)
    *libnewlib.a:stdatomic.*(.literal .literal.* .text .text.*)
    *libnewlib.a:stdatomic_s32c1i.*(.literal .literal.* .text .text.*)
    *librtc.a:(.literal .literal.* .text .text.*)
    *libsoc.a:lldesc.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:esp_flash_api.*(.literal.check_chip_pointer_default .text.check_chip_pointer_default)
    *libspi_flash.a:esp_flash_api.*(.literal.detect_spi_flash_chip .text.detect_spi_flash_chip)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_chip_driver_initialized .text.esp_flash_chip_driver_initialized)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_erase_chip .text.esp_flash_erase_chip)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_erase_region .text.esp_flash_erase_region)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_get_chip_write_protect .text.esp_flash_get_chip_write_protect)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_get_io_mode .text.esp_flash_get_io_mode)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_get_physical_size .text.esp_flash_get_physical_size)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_get_protected_region .text.esp_flash_get_protected_region)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_get_size .text.esp_flash_get_size)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_init .text.esp_flash_init)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_init_main .text.esp_flash_init_main)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_read .text.esp_flash_read)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_read_encrypted .text.esp_flash_read_encrypted)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_set_chip_write_protect .text.esp_flash_set_chip_write_protect)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_set_io_mode .text.esp_flash_set_io_mode)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_set_protected_region .text.esp_flash_set_protected_region)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_write .text.esp_flash_write)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_write_encrypted .text.esp_flash_write_encrypted)
    *libspi_flash.a:esp_flash_api.*(.literal.flash_end_flush_cache .text.flash_end_flush_cache)
    *libspi_flash.a:esp_flash_api.*(.literal.read_unique_id .text.read_unique_id)
    *libspi_flash.a:esp_flash_api.*(.literal.spiflash_end_default .text.spiflash_end_default)
    *libspi_flash.a:esp_flash_api.*(.literal.spiflash_start_core .text.spiflash_start_core)
    *libspi_flash.a:esp_flash_api.*(.literal.spiflash_start_default .text.spiflash_start_default)
    *libspi_flash.a:flash_brownout_hook.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:memspi_host_driver.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_boya.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_gd.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_generic.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_issi.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_mxic.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_th.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_chip_winbond.*(.literal .literal.* .text .text.*)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.delay_us .text.delay_us)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.get_buffer_malloc .text.get_buffer_malloc)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.main_flash_op_status .text.main_flash_op_status)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.main_flash_region_protected .text.main_flash_region_protected)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.release_buffer_malloc .text.release_buffer_malloc)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.spi23_end .text.spi23_end)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.spi23_start .text.spi23_start)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.spi_flash_os_check_yield .text.spi_flash_os_check_yield)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.spi_flash_os_yield .text.spi_flash_os_yield)
    *libspi_flash.a:spi_flash_os_func_noos.*(.literal.delay_us .text.delay_us)
    *libspi_flash.a:spi_flash_os_func_noos.*(.literal.esp_flash_app_disable_os_functions .text.esp_flash_app_disable_os_functions)
    *libspi_flash.a:spi_flash_os_func_noos.*(.literal.get_temp_buffer_not_supported .text.get_temp_buffer_not_supported)
    *libspi_flash.a:spi_flash_wrap.*(.literal .literal.* .text .text.*)
    *libxt_hal.a:(.literal .literal.* .text .text.*)
    *libxtensa.a:(EXCLUDE_FILE(*libxtensa.a:xt_trax.* *libxtensa.a:xtensa_intr.*) .literal EXCLUDE_FILE(*libxtensa.a:xt_trax.* *libxtensa.a:xtensa_intr.*) .literal.* EXCLUDE_FILE(*libxtensa.a:xt_trax.* *libxtensa.a:xtensa_intr.*) .text EXCLUDE_FILE(*libxtensa.a:xt_trax.* *libxtensa.a:xtensa_intr.*) .text.*)
  } > iram0_0_seg
  .dram0.data :
  {
    _data_start = ABSOLUTE(.);
    *(.gnu.linkonce.d.*)
    *(.data1)
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
    *(.gnu.linkonce.s2.*)
    *(.jcr)
    *(EXCLUDE_FILE(*libbt.a *libbtdm_app.a) .data EXCLUDE_FILE(*libbt.a *libbtdm_app.a) .data.*)
    *(.dram1 .dram1.*)
    _coredump_dram_start = ABSOLUTE(.);
    *(.dram2.coredump .dram2.coredump.*)
    _coredump_dram_end = ABSOLUTE(.);
    *libapp_trace.a:app_trace.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libapp_trace.a:app_trace_util.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libapp_trace.a:port_uart.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    . = ALIGN(4);
    _bt_data_start = ABSOLUTE(.);
    *libbt.a:(.data .data.*)
    . = ALIGN(4);
    _bt_data_end = ABSOLUTE(.);
    . = ALIGN(4);
    _bt_controller_data_start = ABSOLUTE(.);
    *libbtdm_app.a:(.data .data.*)
    . = ALIGN(4);
    _bt_controller_data_end = ABSOLUTE(.);
    *libesp_hw_support.a:clk_utils.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hw_support.a:esp_memory_utils.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hw_support.a:mspi_timing_tuning.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_hw_support.a:rtc_clk.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_mm.a:cache_esp32.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_mm.a:esp_cache_msync.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_mm.a:esp_cache_utils.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_rom.a:esp_rom_print.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_rom.a:esp_rom_spiflash.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_rom.a:esp_rom_sys.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_system.a:esp_err.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_system.a:image_process.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libesp_system.a:ubsan.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libgcov.a:(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:cache_hal_esp32.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:i2c_hal_iram.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:ledc_hal_iram.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:mmu_hal.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:spi_flash_encrypt_hal_iram.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:spi_flash_hal_iram.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libhal.a:wdt_hal_iram.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *liblog.a:log.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *liblog.a:log_format_text.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *liblog.a:log_lock.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *liblog.a:log_print.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *liblog.a:log_timestamp_common.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *liblog.a:util.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libnewlib.a:abort.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libnewlib.a:assert.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libnewlib.a:heap.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libnewlib.a:stdatomic.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libnewlib.a:stdatomic_s32c1i.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libphy.a:(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libsoc.a:lldesc.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:flash_brownout_hook.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:memspi_host_driver.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_boya.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_gd.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_generic.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_issi.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_mxic.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_th.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_chip_winbond.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    *libspi_flash.a:spi_flash_wrap.*(.rodata .rodata.* .sdata2 .sdata2.* .srodata .srodata.*)
    _data_end = ABSOLUTE(.);
  } > dram0_0_seg
  /**
   * This section holds data that won't be initialised when startup.
   * This section locates in External RAM region.
   */
  .ext_ram_noinit (NOLOAD) :
  {
    _ext_ram_noinit_start = ABSOLUTE(.);
    *(.ext_ram_noinit*)
    
 . = ALIGN(4); 
 _ext_ram_noinit_end = ABSOLUTE(.);
  } > extern_ram_seg
  /**
   * This section holds data that should not be initialized at power up.
   * The section located in Internal SRAM memory region. The macro _NOINIT
   * can be used as attribute to place data into this section.
   * See the "esp_attr.h" file for more information.
   */
  .noinit (NOLOAD):
  {
    
 . = ALIGN(4); 
 _noinit_start = ABSOLUTE(.);
    *(.noinit .noinit.*)
    
 . = ALIGN(4); 
 _noinit_end = ABSOLUTE(.);
  } > dram0_0_seg
  /* External Memory BSS. (Variables with EXT_RAM_BSS_ATTR attribute). */
  .ext_ram.bss (NOLOAD) :
  {
    
 . = ALIGN(4); 
 _ext_ram_bss_start = ABSOLUTE(.);
    
 . = ALIGN(4); 
 _ext_ram_bss_end = ABSOLUTE(.);
  } > extern_ram_seg
  /* Shared RAM */
  .dram0.bss (NOLOAD) :
  {
    
 . = ALIGN(8); 
 _bss_start = ABSOLUTE(.);
    /**
     * ldgen places all bss-related data to mapping[dram0_bss]
     * (See components/esp_system/app.lf).
     */
    *(EXCLUDE_FILE(*libbt.a *libbtdm_app.a) .bss EXCLUDE_FILE(*libbt.a *libbtdm_app.a) .bss.*)
    *(.dynbss .dynsbss .gnu.linkonce.b .gnu.linkonce.b.* .gnu.linkonce.sb .gnu.linkonce.sb.* .gnu.linkonce.sb2 .gnu.linkonce.sb2.* .sbss .sbss.* .sbss2 .sbss2.* .scommon .share.mem)
    *(.ext_ram.bss .ext_ram.bss.*)
    *(EXCLUDE_FILE(*libbt.a *libbtdm_app.a) COMMON)
    . = ALIGN(4);
    _bt_bss_start = ABSOLUTE(.);
    *libbt.a:(.bss .bss.*)
    . = ALIGN(4);
    _bt_bss_end = ABSOLUTE(.);
    . = ALIGN(4);
    _bt_common_start = ABSOLUTE(.);
    *libbt.a:(COMMON)
    . = ALIGN(4);
    _bt_common_end = ABSOLUTE(.);
    . = ALIGN(4);
    _bt_controller_bss_start = ABSOLUTE(.);
    *libbtdm_app.a:(.bss .bss.*)
    . = ALIGN(4);
    _bt_controller_bss_end = ABSOLUTE(.);
    . = ALIGN(4);
    _bt_controller_common_start = ABSOLUTE(.);
    *libbtdm_app.a:(COMMON)
    . = ALIGN(4);
    _bt_controller_common_end = ABSOLUTE(.);
    
 . = ALIGN(8); 
 _bss_end = ABSOLUTE(.);
  } > dram0_0_seg
  ASSERT(((_bss_end - ORIGIN(dram0_0_seg)) <= LENGTH(dram0_0_seg)),
          "DRAM segment data does not fit.")
  .flash.appdesc : ALIGN(0x10)
  {
    /**
     * Mark flash.rodata start.
     * This can be used for mmu driver to maintain virtual address
     */
    _rodata_reserved_start = ABSOLUTE(.);
    _rodata_start = ABSOLUTE(.);
    /* !DO NOT PUT ANYTHING BEFORE THIS! */
    /* Should be the first.  App version info. */
    *(.rodata_desc .rodata_desc.*)
    /* Should be the second. Custom app version info. */
    *(.rodata_custom_desc .rodata_custom_desc.*)
    /**
     * Create an empty gap within this section. Thanks to this, the end of this
     * section will match .flah.rodata's begin address. Thus, both sections
     * will be merged when creating the final bin image.
     */
    . = ALIGN(ALIGNOF(.flash.rodata));
  } > default_rodata_seg
  ASSERT((ADDR(.flash.rodata) == ADDR(.flash.appdesc) + SIZEOF(.flash.appdesc)), "The gap between .flash.appdesc and .flash.rodata must not exist to produce the final bin image.")
  .flash.rodata : ALIGN(0x10)
  {
    _flash_rodata_start = ABSOLUTE(.);
    *(EXCLUDE_FILE(*libgcov.a *libphy.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:rtc_clk.* *libesp_mm.a:cache_esp32.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:ubsan.* *libhal.a:cache_hal_esp32.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_iram.* *libhal.a:wdt_hal_iram.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp_common.* *liblog.a:util.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libnewlib.a:stdatomic_s32c1i.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_wrap.*) .rodata EXCLUDE_FILE(*libgcov.a *libphy.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:rtc_clk.* *libesp_mm.a:cache_esp32.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:ubsan.* *libhal.a:cache_hal_esp32.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_iram.* *libhal.a:wdt_hal_iram.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp_common.* *liblog.a:util.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libnewlib.a:stdatomic_s32c1i.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_wrap.*) .rodata.* EXCLUDE_FILE(*libgcov.a *libphy.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:rtc_clk.* *libesp_mm.a:cache_esp32.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:ubsan.* *libhal.a:cache_hal_esp32.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_iram.* *libhal.a:wdt_hal_iram.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp_common.* *liblog.a:util.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libnewlib.a:stdatomic_s32c1i.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_wrap.*) .sdata2 EXCLUDE_FILE(*libgcov.a *libphy.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:rtc_clk.* *libesp_mm.a:cache_esp32.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:ubsan.* *libhal.a:cache_hal_esp32.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_iram.* *libhal.a:wdt_hal_iram.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp_common.* *liblog.a:util.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libnewlib.a:stdatomic_s32c1i.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_wrap.*) .sdata2.* EXCLUDE_FILE(*libgcov.a *libphy.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:rtc_clk.* *libesp_mm.a:cache_esp32.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:ubsan.* *libhal.a:cache_hal_esp32.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_iram.* *libhal.a:wdt_hal_iram.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp_common.* *liblog.a:util.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libnewlib.a:stdatomic_s32c1i.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_wrap.*) .srodata EXCLUDE_FILE(*libgcov.a *libphy.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:rtc_clk.* *libesp_mm.a:cache_esp32.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_system.a:esp_err.* *libesp_system.a:image_process.* *libesp_system.a:ubsan.* *libhal.a:cache_hal_esp32.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_iram.* *libhal.a:wdt_hal_iram.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp_common.* *liblog.a:util.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libnewlib.a:stdatomic_s32c1i.* *libsoc.a:lldesc.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_wrap.*) .srodata.*)
    *(.rodata_wlog_error .rodata_wlog_error.*)
    *(.irom1.text) /* catch stray ICACHE_RODATA_ATTR */
    *(.gnu.linkonce.r.*)
    *(.rodata1)
    /* C++ exception handlers table.  */
    
 . = ALIGN(4); 
 __XT_EXCEPTION_TABLE_ = ABSOLUTE(.);
    *(.xt_except_table)
    *(.gcc_except_table .gcc_except_table.*)
    *(.gnu.linkonce.e.*)
    
 . = ALIGN(4); 
 __XT_EXCEPTION_DESCS_ = ABSOLUTE(.);
    *(.xt_except_desc)
    *(.gnu.linkonce.h.*)
    __XT_EXCEPTION_DESCS_END__ = ABSOLUTE(.);
    *(.xt_except_desc_end)
    
 . = ALIGN(4); 
 __eh_frame = ABSOLUTE(.);
    KEEP(*(.eh_frame))
    /**
     * As we are not linking with crtend.o, which includes the CIE terminator
     * (see __FRAME_END__ in libgcc sources), it is manually provided here.
     */
    LONG(0);
    /**
     * C++ constructor tables.
     *
     * Excluding crtbegin.o/crtend.o since IDF doesn't use the toolchain crt.
     */
    
 . = ALIGN(4); 
 __init_array_start = ABSOLUTE(.);
    KEEP (*(EXCLUDE_FILE (*crtend.* *crtbegin.*) .ctors SORT(.ctors.*)))
    __init_array_end = ABSOLUTE(.);
    /* Addresses of memory regions reserved via SOC_RESERVE_MEMORY_REGION() */
    
 . = ALIGN(4); 
 soc_reserved_memory_region_start = ABSOLUTE(.);
    KEEP (*(.reserved_memory_address))
    soc_reserved_memory_region_end = ABSOLUTE(.);
    /* System init functions registered via ESP_SYSTEM_INIT_FN */
    
 . = ALIGN(4); 
 _esp_system_init_fn_array_start = ABSOLUTE(.);
    KEEP (*(SORT_BY_INIT_PRIORITY(.esp_system_init_fn.*)))
    _esp_system_init_fn_array_end = ABSOLUTE(.);
    _rodata_end = ABSOLUTE(.);
    /* Literals are also RO data. */
    _lit4_start = ABSOLUTE(.);
    *(*.lit4)
    *(.lit4.*)
    *(.gnu.linkonce.lit4.*)
    _lit4_end = ABSOLUTE(.);
    /* TLS data. */
    
 . = ALIGN(4); 
 _thread_local_start = ABSOLUTE(.);
    *(.tdata)
    *(.tdata.*)
    *(.tbss)
    *(.tbss.*)
    _thread_local_end = ABSOLUTE(.);
  } > default_rodata_seg
  _flash_rodata_align = ALIGNOF(.flash.rodata);
  /**
   * This section contains all the rodata that is not used
   * at runtime, helping to avoid an increase in binary size.
   */
  .flash.rodata_noload (NOLOAD) :
  {
    /**
     * This symbol marks the end of flash.rodata. It can be utilized by the MMU
     * driver to maintain the virtual address.
     * NOLOAD rodata may not be included in this section.
     */
    _rodata_reserved_end = ABSOLUTE(.);
    *(.rodata_wlog_debug .rodata_wlog_debug.*)
    *(.rodata_wlog_info .rodata_wlog_info.*)
    *(.rodata_wlog_verbose .rodata_wlog_verbose.*)
    *(.rodata_wlog_warning .rodata_wlog_warning.*)
  } > default_rodata_seg
  .flash.text :
  {
    _stext = .;
    /**
     * Mark the start of flash.text.
     * This can be used by the MMU driver to maintain the virtual address.
     */
    _instruction_reserved_start = ABSOLUTE(.);
    _text_start = ABSOLUTE(.);
    *(EXCLUDE_FILE(*libesp_ringbuf.a *libfreertos.a *libgcov.a *librtc.a *libxt_hal.a *libxtensa.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libesp_driver_gptimer.a:gptimer.* *libesp_driver_i2c.a:i2c_master.* *libesp_driver_mcpwm.a:mcpwm_cap.* *libesp_driver_mcpwm.a:mcpwm_cmpr.* *libesp_driver_mcpwm.a:mcpwm_fault.* *libesp_driver_mcpwm.a:mcpwm_oper.* *libesp_driver_mcpwm.a:mcpwm_timer.* *libesp_driver_rmt.a:rmt_encoder.* *libesp_driver_rmt.a:rmt_rx.* *libesp_driver_rmt.a:rmt_tx.* *libesp_event.a:default_event_loop.* *libesp_event.a:esp_event.* *libesp_hw_support.a:adc_share_hw_ctrl.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_clk.* *libesp_hw_support.a:esp_clk_tree.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:periph_ctrl.* *libesp_hw_support.a:regi2c_ctrl.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:rtc_init.* *libesp_hw_support.a:rtc_sleep.* *libesp_hw_support.a:rtc_time.* *libesp_hw_support.a:rtc_wdt.* *libesp_hw_support.a:sar_periph_ctrl.* *libesp_hw_support.a:sleep_modem.* *libesp_hw_support.a:sleep_modes.* *libesp_mm.a:cache_esp32.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_phy.a:phy_override.* *libesp_pm.a:pm_impl.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_system.a:esp_err.* *libesp_system.a:esp_system_chip.* *libesp_system.a:freertos_hooks.* *libesp_system.a:image_process.* *libesp_system.a:panic.* *libesp_system.a:reset_reason.* *libesp_system.a:stack_check.* *libesp_system.a:system_internal.* *libesp_system.a:system_time.* *libesp_system.a:ubsan.* *libesp_timer.a:esp_timer_impl_common.* *libesp_timer.a:esp_timer_impl_lac.* *libgcc.a:lib2funcs.* *libhal.a:cache_hal_esp32.* *libhal.a:gpio_hal.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_iram.* *libhal.a:timer_hal.* *libhal.a:wdt_hal_iram.* *libheap.a:multi_heap.* *libheap.a:multi_heap_poisoning.* *libheap.a:tlsf.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp.* *liblog.a:log_timestamp_common.* *liblog.a:log_write.* *liblog.a:tag_log_level.* *liblog.a:util.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:esp_time_impl.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libnewlib.a:stdatomic_s32c1i.* *libsoc.a:lldesc.* *libspi_flash.a:esp_flash_api.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_os_func_app.* *libspi_flash.a:spi_flash_os_func_noos.* *libspi_flash.a:spi_flash_wrap.*) .literal EXCLUDE_FILE(*libesp_ringbuf.a *libfreertos.a *libgcov.a *librtc.a *libxt_hal.a *libxtensa.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libesp_driver_gptimer.a:gptimer.* *libesp_driver_i2c.a:i2c_master.* *libesp_driver_mcpwm.a:mcpwm_cap.* *libesp_driver_mcpwm.a:mcpwm_cmpr.* *libesp_driver_mcpwm.a:mcpwm_fault.* *libesp_driver_mcpwm.a:mcpwm_oper.* *libesp_driver_mcpwm.a:mcpwm_timer.* *libesp_driver_rmt.a:rmt_encoder.* *libesp_driver_rmt.a:rmt_rx.* *libesp_driver_rmt.a:rmt_tx.* *libesp_event.a:default_event_loop.* *libesp_event.a:esp_event.* *libesp_hw_support.a:adc_share_hw_ctrl.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_clk.* *libesp_hw_support.a:esp_clk_tree.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:periph_ctrl.* *libesp_hw_support.a:regi2c_ctrl.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:rtc_init.* *libesp_hw_support.a:rtc_sleep.* *libesp_hw_support.a:rtc_time.* *libesp_hw_support.a:rtc_wdt.* *libesp_hw_support.a:sar_periph_ctrl.* *libesp_hw_support.a:sleep_modem.* *libesp_hw_support.a:sleep_modes.* *libesp_mm.a:cache_esp32.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_phy.a:phy_override.* *libesp_pm.a:pm_impl.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_system.a:esp_err.* *libesp_system.a:esp_system_chip.* *libesp_system.a:freertos_hooks.* *libesp_system.a:image_process.* *libesp_system.a:panic.* *libesp_system.a:reset_reason.* *libesp_system.a:stack_check.* *libesp_system.a:system_internal.* *libesp_system.a:system_time.* *libesp_system.a:ubsan.* *libesp_timer.a:esp_timer_impl_common.* *libesp_timer.a:esp_timer_impl_lac.* *libgcc.a:lib2funcs.* *libhal.a:cache_hal_esp32.* *libhal.a:gpio_hal.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_iram.* *libhal.a:timer_hal.* *libhal.a:wdt_hal_iram.* *libheap.a:multi_heap.* *libheap.a:multi_heap_poisoning.* *libheap.a:tlsf.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp.* *liblog.a:log_timestamp_common.* *liblog.a:log_write.* *liblog.a:tag_log_level.* *liblog.a:util.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:esp_time_impl.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libnewlib.a:stdatomic_s32c1i.* *libsoc.a:lldesc.* *libspi_flash.a:esp_flash_api.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_os_func_app.* *libspi_flash.a:spi_flash_os_func_noos.* *libspi_flash.a:spi_flash_wrap.*) .literal.* EXCLUDE_FILE(*libesp_ringbuf.a *libfreertos.a *libgcov.a *librtc.a *libxt_hal.a *libxtensa.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libesp_driver_gptimer.a:gptimer.* *libesp_driver_i2c.a:i2c_master.* *libesp_driver_mcpwm.a:mcpwm_cap.* *libesp_driver_mcpwm.a:mcpwm_cmpr.* *libesp_driver_mcpwm.a:mcpwm_fault.* *libesp_driver_mcpwm.a:mcpwm_oper.* *libesp_driver_mcpwm.a:mcpwm_timer.* *libesp_driver_rmt.a:rmt_encoder.* *libesp_driver_rmt.a:rmt_rx.* *libesp_driver_rmt.a:rmt_tx.* *libesp_event.a:default_event_loop.* *libesp_event.a:esp_event.* *libesp_hw_support.a:adc_share_hw_ctrl.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_clk.* *libesp_hw_support.a:esp_clk_tree.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:periph_ctrl.* *libesp_hw_support.a:regi2c_ctrl.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:rtc_init.* *libesp_hw_support.a:rtc_sleep.* *libesp_hw_support.a:rtc_time.* *libesp_hw_support.a:rtc_wdt.* *libesp_hw_support.a:sar_periph_ctrl.* *libesp_hw_support.a:sleep_modem.* *libesp_hw_support.a:sleep_modes.* *libesp_mm.a:cache_esp32.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_phy.a:phy_override.* *libesp_pm.a:pm_impl.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_system.a:esp_err.* *libesp_system.a:esp_system_chip.* *libesp_system.a:freertos_hooks.* *libesp_system.a:image_process.* *libesp_system.a:panic.* *libesp_system.a:reset_reason.* *libesp_system.a:stack_check.* *libesp_system.a:system_internal.* *libesp_system.a:system_time.* *libesp_system.a:ubsan.* *libesp_timer.a:esp_timer_impl_common.* *libesp_timer.a:esp_timer_impl_lac.* *libgcc.a:lib2funcs.* *libhal.a:cache_hal_esp32.* *libhal.a:gpio_hal.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_iram.* *libhal.a:timer_hal.* *libhal.a:wdt_hal_iram.* *libheap.a:multi_heap.* *libheap.a:multi_heap_poisoning.* *libheap.a:tlsf.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp.* *liblog.a:log_timestamp_common.* *liblog.a:log_write.* *liblog.a:tag_log_level.* *liblog.a:util.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:esp_time_impl.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libnewlib.a:stdatomic_s32c1i.* *libsoc.a:lldesc.* *libspi_flash.a:esp_flash_api.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_os_func_app.* *libspi_flash.a:spi_flash_os_func_noos.* *libspi_flash.a:spi_flash_wrap.*) .text EXCLUDE_FILE(*libesp_ringbuf.a *libfreertos.a *libgcov.a *librtc.a *libxt_hal.a *libxtensa.a *libapp_trace.a:app_trace.* *libapp_trace.a:app_trace_util.* *libapp_trace.a:port_uart.* *libesp_driver_gptimer.a:gptimer.* *libesp_driver_i2c.a:i2c_master.* *libesp_driver_mcpwm.a:mcpwm_cap.* *libesp_driver_mcpwm.a:mcpwm_cmpr.* *libesp_driver_mcpwm.a:mcpwm_fault.* *libesp_driver_mcpwm.a:mcpwm_oper.* *libesp_driver_mcpwm.a:mcpwm_timer.* *libesp_driver_rmt.a:rmt_encoder.* *libesp_driver_rmt.a:rmt_rx.* *libesp_driver_rmt.a:rmt_tx.* *libesp_event.a:default_event_loop.* *libesp_event.a:esp_event.* *libesp_hw_support.a:adc_share_hw_ctrl.* *libesp_hw_support.a:clk_utils.* *libesp_hw_support.a:cpu.* *libesp_hw_support.a:esp_clk.* *libesp_hw_support.a:esp_clk_tree.* *libesp_hw_support.a:esp_memory_utils.* *libesp_hw_support.a:mspi_timing_tuning.* *libesp_hw_support.a:periph_ctrl.* *libesp_hw_support.a:regi2c_ctrl.* *libesp_hw_support.a:rtc_clk.* *libesp_hw_support.a:rtc_init.* *libesp_hw_support.a:rtc_sleep.* *libesp_hw_support.a:rtc_time.* *libesp_hw_support.a:rtc_wdt.* *libesp_hw_support.a:sar_periph_ctrl.* *libesp_hw_support.a:sleep_modem.* *libesp_hw_support.a:sleep_modes.* *libesp_mm.a:cache_esp32.* *libesp_mm.a:esp_cache_msync.* *libesp_mm.a:esp_cache_utils.* *libesp_phy.a:phy_override.* *libesp_pm.a:pm_impl.* *libesp_rom.a:esp_rom_print.* *libesp_rom.a:esp_rom_spiflash.* *libesp_rom.a:esp_rom_sys.* *libesp_system.a:esp_err.* *libesp_system.a:esp_system_chip.* *libesp_system.a:freertos_hooks.* *libesp_system.a:image_process.* *libesp_system.a:panic.* *libesp_system.a:reset_reason.* *libesp_system.a:stack_check.* *libesp_system.a:system_internal.* *libesp_system.a:system_time.* *libesp_system.a:ubsan.* *libesp_timer.a:esp_timer_impl_common.* *libesp_timer.a:esp_timer_impl_lac.* *libgcc.a:lib2funcs.* *libhal.a:cache_hal_esp32.* *libhal.a:gpio_hal.* *libhal.a:i2c_hal_iram.* *libhal.a:ledc_hal_iram.* *libhal.a:mmu_hal.* *libhal.a:spi_flash_encrypt_hal_iram.* *libhal.a:spi_flash_hal_iram.* *libhal.a:timer_hal.* *libhal.a:wdt_hal_iram.* *libheap.a:multi_heap.* *libheap.a:multi_heap_poisoning.* *libheap.a:tlsf.* *liblog.a:log.* *liblog.a:log_format_text.* *liblog.a:log_lock.* *liblog.a:log_print.* *liblog.a:log_timestamp.* *liblog.a:log_timestamp_common.* *liblog.a:log_write.* *liblog.a:tag_log_level.* *liblog.a:util.* *libnewlib.a:abort.* *libnewlib.a:assert.* *libnewlib.a:esp_time_impl.* *libnewlib.a:heap.* *libnewlib.a:stdatomic.* *libnewlib.a:stdatomic_s32c1i.* *libsoc.a:lldesc.* *libspi_flash.a:esp_flash_api.* *libspi_flash.a:flash_brownout_hook.* *libspi_flash.a:memspi_host_driver.* *libspi_flash.a:spi_flash_chip_boya.* *libspi_flash.a:spi_flash_chip_gd.* *libspi_flash.a:spi_flash_chip_generic.* *libspi_flash.a:spi_flash_chip_issi.* *libspi_flash.a:spi_flash_chip_mxic.* *libspi_flash.a:spi_flash_chip_th.* *libspi_flash.a:spi_flash_chip_winbond.* *libspi_flash.a:spi_flash_os_func_app.* *libspi_flash.a:spi_flash_os_func_noos.* *libspi_flash.a:spi_flash_wrap.*) .text.*)
    *(.wifi0iram .wifi0iram.*)
    *(.wifiextrairam .wifiextrairam.*)
    *(.wifiorslpiram .wifiorslpiram.*)
    *(.wifirxiram .wifirxiram.*)
    *(.wifislpiram .wifislpiram.*)
    *(.wifislprxiram .wifislprxiram.*)
    *libcoexist.a:(.coexiram .coexiram.*)
    *libcoexist.a:(.coexsleepiram .coexsleepiram.*)
    *libesp_driver_gptimer.a:gptimer.*(.literal.gptimer_del_timer .literal.gptimer_destroy .literal.gptimer_disable .literal.gptimer_enable .literal.gptimer_get_captured_count .literal.gptimer_get_raw_count .literal.gptimer_get_resolution .literal.gptimer_new_timer .literal.gptimer_register_event_callbacks .literal.gptimer_set_alarm_action .literal.gptimer_set_raw_count .literal.gptimer_start .literal.gptimer_stop .text .text.gptimer_del_timer .text.gptimer_destroy .text.gptimer_disable .text.gptimer_enable .text.gptimer_get_captured_count .text.gptimer_get_raw_count .text.gptimer_get_resolution .text.gptimer_new_timer .text.gptimer_register_event_callbacks .text.gptimer_set_alarm_action .text.gptimer_set_raw_count .text.gptimer_start .text.gptimer_stop)
    *libesp_driver_i2c.a:i2c_master.*(.literal.i2c_del_master_bus .literal.i2c_master_bus_add_device .literal.i2c_master_bus_destroy .literal.i2c_master_bus_reset .literal.i2c_master_bus_rm_device .literal.i2c_master_bus_wait_all_done .literal.i2c_master_device_change_address .literal.i2c_master_execute_defined_operations .literal.i2c_master_get_bus_handle .literal.i2c_master_multi_buffer_transmit .literal.i2c_master_probe .literal.i2c_master_receive .literal.i2c_master_register_event_callbacks .literal.i2c_master_transmit .literal.i2c_master_transmit_receive .literal.i2c_new_master_bus .literal.s_i2c_asynchronous_transaction .literal.s_i2c_hw_fsm_reset$isra$0 .literal.s_i2c_read_command$isra$0 .literal.s_i2c_send_command_async .literal.s_i2c_send_commands .literal.s_i2c_start_end_command .literal.s_i2c_synchronous_transaction .literal.s_i2c_transaction_start .literal.s_i2c_write_command$isra$0 .text .text.i2c_del_master_bus .text.i2c_master_bus_add_device .text.i2c_master_bus_destroy .text.i2c_master_bus_reset .text.i2c_master_bus_rm_device .text.i2c_master_bus_wait_all_done .text.i2c_master_device_change_address .text.i2c_master_execute_defined_operations .text.i2c_master_get_bus_handle .text.i2c_master_multi_buffer_transmit .text.i2c_master_probe .text.i2c_master_receive .text.i2c_master_register_event_callbacks .text.i2c_master_transmit .text.i2c_master_transmit_receive .text.i2c_new_master_bus .text.s_i2c_asynchronous_transaction .text.s_i2c_hw_fsm_reset$isra$0 .text.s_i2c_read_command$isra$0 .text.s_i2c_send_command_async .text.s_i2c_send_commands .text.s_i2c_start_end_command .text.s_i2c_synchronous_transaction .text.s_i2c_transaction_start .text.s_i2c_write_command$isra$0)
    *libesp_driver_mcpwm.a:mcpwm_cap.*(.literal.mcpwm_cap_timer_destroy .literal.mcpwm_capture_channel_destroy .literal.mcpwm_capture_channel_disable .literal.mcpwm_capture_channel_enable .literal.mcpwm_capture_channel_register_event_callbacks .literal.mcpwm_capture_channel_trigger_soft_catch .literal.mcpwm_capture_timer_disable .literal.mcpwm_capture_timer_enable .literal.mcpwm_capture_timer_get_resolution .literal.mcpwm_capture_timer_set_phase_on_sync .literal.mcpwm_capture_timer_start .literal.mcpwm_capture_timer_stop .literal.mcpwm_del_capture_channel .literal.mcpwm_del_capture_timer .literal.mcpwm_new_capture_channel .literal.mcpwm_new_capture_timer .text .text.mcpwm_cap_timer_destroy .text.mcpwm_capture_channel_destroy .text.mcpwm_capture_channel_disable .text.mcpwm_capture_channel_enable .text.mcpwm_capture_channel_register_event_callbacks .text.mcpwm_capture_channel_trigger_soft_catch .text.mcpwm_capture_timer_disable .text.mcpwm_capture_timer_enable .text.mcpwm_capture_timer_get_resolution .text.mcpwm_capture_timer_set_phase_on_sync .text.mcpwm_capture_timer_start .text.mcpwm_capture_timer_stop .text.mcpwm_del_capture_channel .text.mcpwm_del_capture_timer .text.mcpwm_new_capture_channel .text.mcpwm_new_capture_timer)
    *libesp_driver_mcpwm.a:mcpwm_cmpr.*(.literal.mcpwm_comparator_destroy .literal.mcpwm_comparator_register_event_callbacks .literal.mcpwm_comparator_set_compare_value .literal.mcpwm_del_comparator .literal.mcpwm_new_comparator .text .text.mcpwm_comparator_destroy .text.mcpwm_comparator_register_event_callbacks .text.mcpwm_comparator_set_compare_value .text.mcpwm_del_comparator .text.mcpwm_new_comparator)
    *libesp_driver_mcpwm.a:mcpwm_fault.*(.literal.mcpwm_del_fault .literal.mcpwm_del_gpio_fault .literal.mcpwm_del_soft_fault .literal.mcpwm_fault_register_event_callbacks .literal.mcpwm_gpio_fault_destroy .literal.mcpwm_new_gpio_fault .literal.mcpwm_new_soft_fault .literal.mcpwm_soft_fault_activate .text .text.mcpwm_del_fault .text.mcpwm_del_gpio_fault .text.mcpwm_del_soft_fault .text.mcpwm_fault_register_event_callbacks .text.mcpwm_gpio_fault_destroy .text.mcpwm_new_gpio_fault .text.mcpwm_new_soft_fault .text.mcpwm_soft_fault_activate)
    *libesp_driver_mcpwm.a:mcpwm_oper.*(.literal.mcpwm_del_operator .literal.mcpwm_new_operator .literal.mcpwm_operator_apply_carrier .literal.mcpwm_operator_connect_timer .literal.mcpwm_operator_destroy .literal.mcpwm_operator_recover_from_fault .literal.mcpwm_operator_register_event_callbacks .literal.mcpwm_operator_set_brake_on_fault .text .text.mcpwm_del_operator .text.mcpwm_new_operator .text.mcpwm_operator_apply_carrier .text.mcpwm_operator_connect_timer .text.mcpwm_operator_destroy .text.mcpwm_operator_recover_from_fault .text.mcpwm_operator_register_event_callbacks .text.mcpwm_operator_set_brake_on_fault)
    *libesp_driver_mcpwm.a:mcpwm_timer.*(.literal.mcpwm_del_timer .literal.mcpwm_new_timer .literal.mcpwm_timer_destroy .literal.mcpwm_timer_disable .literal.mcpwm_timer_enable .literal.mcpwm_timer_get_phase .literal.mcpwm_timer_register_event_callbacks .literal.mcpwm_timer_set_period .literal.mcpwm_timer_set_phase_on_sync .literal.mcpwm_timer_start_stop .text .text.mcpwm_del_timer .text.mcpwm_new_timer .text.mcpwm_timer_destroy .text.mcpwm_timer_disable .text.mcpwm_timer_enable .text.mcpwm_timer_get_phase .text.mcpwm_timer_register_event_callbacks .text.mcpwm_timer_set_period .text.mcpwm_timer_set_phase_on_sync .text.mcpwm_timer_start_stop)
    *libesp_driver_rmt.a:rmt_encoder.*(.literal.rmt_alloc_encoder_mem .literal.rmt_del_encoder .text .text.rmt_alloc_encoder_mem .text.rmt_del_encoder)
    *libesp_driver_rmt.a:rmt_rx.*(.literal.rmt_del_rx_channel .literal.rmt_new_rx_channel .literal.rmt_receive .literal.rmt_rx_demodulate_carrier .literal.rmt_rx_destroy .literal.rmt_rx_disable .literal.rmt_rx_enable .literal.rmt_rx_register_event_callbacks .text .text.rmt_del_rx_channel .text.rmt_new_rx_channel .text.rmt_receive .text.rmt_rx_demodulate_carrier .text.rmt_rx_destroy .text.rmt_rx_disable .text.rmt_rx_enable .text.rmt_rx_register_event_callbacks)
    *libesp_driver_rmt.a:rmt_tx.*(.literal.rmt_del_sync_manager .literal.rmt_del_tx_channel .literal.rmt_new_sync_manager .literal.rmt_new_tx_channel .literal.rmt_sync_reset .literal.rmt_transmit .literal.rmt_tx_destroy .literal.rmt_tx_disable .literal.rmt_tx_enable .literal.rmt_tx_modulate_carrier .literal.rmt_tx_register_event_callbacks .literal.rmt_tx_switch_gpio .literal.rmt_tx_wait_all_done .text .text.rmt_del_sync_manager .text.rmt_del_tx_channel .text.rmt_new_sync_manager .text.rmt_new_tx_channel .text.rmt_sync_reset .text.rmt_transmit .text.rmt_tx_destroy .text.rmt_tx_disable .text.rmt_tx_enable .text.rmt_tx_modulate_carrier .text.rmt_tx_register_event_callbacks .text.rmt_tx_switch_gpio .text.rmt_tx_wait_all_done)
    *libesp_event.a:default_event_loop.*(.literal.esp_event_handler_instance_register .literal.esp_event_handler_instance_unregister .literal.esp_event_handler_register .literal.esp_event_handler_unregister .literal.esp_event_loop_create_default .literal.esp_event_loop_delete_default .literal.esp_event_post .text .text.esp_event_handler_instance_register .text.esp_event_handler_instance_unregister .text.esp_event_handler_register .text.esp_event_handler_unregister .text.esp_event_loop_create_default .text.esp_event_loop_delete_default .text.esp_event_post)
    *libesp_event.a:esp_event.*(.literal.base_node_add_handler .literal.esp_event_handler_instance_register_with .literal.esp_event_handler_instance_unregister_with .literal.esp_event_handler_register_with .literal.esp_event_handler_register_with_internal .literal.esp_event_handler_unregister_with .literal.esp_event_handler_unregister_with_internal .literal.esp_event_loop_create .literal.esp_event_loop_delete .literal.esp_event_loop_run .literal.esp_event_loop_run_task .literal.esp_event_post_to .literal.handler_instances_add .literal.handler_instances_remove .literal.handler_instances_remove_all .literal.loop_node_add_handler .literal.loop_remove_handler .text .text.base_node_add_handler .text.esp_event_dump .text.esp_event_handler_instance_register_with .text.esp_event_handler_instance_unregister_with .text.esp_event_handler_register_with .text.esp_event_handler_register_with_internal .text.esp_event_handler_unregister_with .text.esp_event_handler_unregister_with_internal .text.esp_event_loop_create .text.esp_event_loop_delete .text.esp_event_loop_run .text.esp_event_loop_run_task .text.esp_event_post_to .text.handler_instances_add .text.handler_instances_remove .text.handler_instances_remove_all .text.loop_node_add_handler .text.loop_remove_handler)
    *libesp_hw_support.a:adc_share_hw_ctrl.*(.literal.adc2_wifi_acquire .literal.adc2_wifi_release .literal.adc_lock_acquire .literal.adc_lock_release .literal.adc_lock_try_acquire .text .text.adc2_wifi_acquire .text.adc2_wifi_release .text.adc_lock_acquire .text.adc_lock_release .text.adc_lock_try_acquire)
    *libesp_hw_support.a:cpu.*(.text .text.esp_cpu_clear_breakpoint .text.esp_cpu_clear_watchpoint .text.esp_cpu_set_breakpoint .text.esp_cpu_set_watchpoint)
    *libesp_hw_support.a:esp_clk.*(.literal.esp_clk_rtc_time .text .text.esp_clk_rtc_time)
    *libesp_hw_support.a:esp_clk_tree.*(.literal.esp_clk_tree_src_get_freq_hz .text .text.esp_clk_tree_src_get_freq_hz)
    *libesp_hw_support.a:periph_ctrl.*(.literal.periph_ll_get_clk_en_mask .literal.periph_ll_get_rst_en_reg .literal.periph_module_disable .literal.periph_module_enable .text .text.periph_ll_get_clk_en_mask .text.periph_ll_get_rst_en_mask .text.periph_ll_get_rst_en_reg .text.periph_module_disable .text.periph_module_enable)
    *libesp_hw_support.a:regi2c_ctrl.*(.text)
    *libesp_hw_support.a:rtc_init.*(.literal.rtc_init .text .text.rtc_init)
    *libesp_hw_support.a:rtc_sleep.*(.literal.rtc_deep_sleep_start .text .text.rtc_deep_sleep_start)
    *libesp_hw_support.a:sar_periph_ctrl.*(.literal.s_sar_power_acquire .literal.s_sar_power_release .literal.sar_periph_ctrl_adc_continuous_power_acquire .literal.sar_periph_ctrl_adc_continuous_power_release .literal.sar_periph_ctrl_adc_oneshot_power_acquire .literal.sar_periph_ctrl_adc_oneshot_power_release .literal.sar_periph_ctrl_init .literal.sar_periph_ctrl_pwdet_power_acquire .literal.sar_periph_ctrl_pwdet_power_release .text .text.s_sar_power_acquire .text.s_sar_power_release .text.sar_periph_ctrl_adc_continuous_power_acquire .text.sar_periph_ctrl_adc_continuous_power_release .text.sar_periph_ctrl_adc_oneshot_power_acquire .text.sar_periph_ctrl_adc_oneshot_power_release .text.sar_periph_ctrl_init .text.sar_periph_ctrl_pwdet_power_acquire .text.sar_periph_ctrl_pwdet_power_release)
    *libesp_hw_support.a:sleep_modem.*(.literal.esp_pm_register_inform_out_light_sleep_overhead_callback .literal.esp_pm_register_light_sleep_default_params_config_callback .literal.esp_pm_unregister_inform_out_light_sleep_overhead_callback .literal.esp_pm_unregister_light_sleep_default_params_config_callback .text .text.esp_pm_register_inform_out_light_sleep_overhead_callback .text.esp_pm_register_light_sleep_default_params_config_callback .text.esp_pm_unregister_inform_out_light_sleep_overhead_callback .text.esp_pm_unregister_light_sleep_default_params_config_callback .text.sleep_modem_configure)
    *libesp_hw_support.a:sleep_modes.*(.literal.esp_deep_sleep .literal.esp_deep_sleep_deregister_hook .literal.esp_deep_sleep_deregister_phy_hook .literal.esp_deep_sleep_register_hook .literal.esp_deep_sleep_register_phy_hook .literal.esp_deep_sleep_try .literal.esp_get_deep_sleep_wake_stub .literal.esp_sleep_disable_ext1_wakeup_io .literal.esp_sleep_disable_wakeup_source .literal.esp_sleep_enable_adc_tsens_monitor .literal.esp_sleep_enable_ext0_wakeup .literal.esp_sleep_enable_ext1_wakeup .literal.esp_sleep_enable_ext1_wakeup_io .literal.esp_sleep_enable_gpio_wakeup .literal.esp_sleep_enable_touchpad_wakeup .literal.esp_sleep_enable_uart_wakeup .literal.esp_sleep_enable_ulp_wakeup .literal.esp_sleep_get_ext1_wakeup_status .literal.esp_sleep_get_touchpad_wakeup_status .literal.esp_sleep_get_wakeup_cause .literal.esp_sleep_get_wakeup_causes .literal.esp_sleep_is_valid_wakeup_gpio .literal.esp_sleep_overhead_out_time_refresh .literal.esp_sleep_pd_config .literal.esp_sleep_periph_use_8m .literal.esp_sleep_sub_mode_config .literal.esp_sleep_sub_mode_dump_config .literal.esp_sleep_sub_mode_force_disable .literal.rtc_sleep_enable_ultra_low .literal.rtcio_ll_function_select$constprop$0 .literal.s_sleep_hook_deregister .literal.s_sleep_hook_register .text .text.esp_deep_sleep .text.esp_deep_sleep_deregister_hook .text.esp_deep_sleep_deregister_phy_hook .text.esp_deep_sleep_register_hook .text.esp_deep_sleep_register_phy_hook .text.esp_deep_sleep_try .text.esp_get_deep_sleep_wake_stub .text.esp_sleep_disable_bt_wakeup .text.esp_sleep_disable_ext1_wakeup_io .text.esp_sleep_disable_wakeup_source .text.esp_sleep_disable_wifi_beacon_wakeup .text.esp_sleep_disable_wifi_wakeup .text.esp_sleep_enable_adc_tsens_monitor .text.esp_sleep_enable_bt_wakeup .text.esp_sleep_enable_ext0_wakeup .text.esp_sleep_enable_ext1_wakeup .text.esp_sleep_enable_ext1_wakeup_io .text.esp_sleep_enable_gpio_wakeup .text.esp_sleep_enable_touchpad_wakeup .text.esp_sleep_enable_uart_wakeup .text.esp_sleep_enable_ulp_wakeup .text.esp_sleep_enable_wifi_beacon_wakeup .text.esp_sleep_enable_wifi_wakeup .text.esp_sleep_get_ext1_wakeup_status .text.esp_sleep_get_touchpad_wakeup_status .text.esp_sleep_get_wakeup_cause .text.esp_sleep_get_wakeup_causes .text.esp_sleep_is_valid_wakeup_gpio .text.esp_sleep_overhead_out_time_refresh .text.esp_sleep_pd_config .text.esp_sleep_periph_use_8m .text.esp_sleep_sub_mode_config .text.esp_sleep_sub_mode_dump_config .text.esp_sleep_sub_mode_force_disable .text.rtc_sleep_enable_ultra_low .text.rtcio_ll_function_select$constprop$0 .text.s_sleep_hook_deregister .text.s_sleep_hook_register)
    *libesp_phy.a:phy_override.*(.literal.esp_phy_efuse_get_chip_ver_pkg .literal.esp_phy_efuse_get_mac .literal.set_xpd_sar .text .text.esp_phy_efuse_get_chip_ver_pkg .text.esp_phy_efuse_get_mac .text.include_esp_phy_override .text.phy_set_pwdet_power .text.set_xpd_sar)
    *libesp_pm.a:pm_impl.*(.literal.esp_pm_get_configuration .literal.esp_pm_impl_get_mode .literal.esp_pm_impl_idle_hook .literal.esp_pm_impl_init .literal.esp_pm_impl_waiti .text .text.esp_pm_configure .text.esp_pm_get_configuration .text.esp_pm_impl_get_mode .text.esp_pm_impl_idle_hook .text.esp_pm_impl_init .text.esp_pm_impl_waiti)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvGetCurMaxSizeAllowSplit .text.prvGetCurMaxSizeAllowSplit)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvGetCurMaxSizeByteBuf .text.prvGetCurMaxSizeByteBuf)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvGetCurMaxSizeNoSplit .text.prvGetCurMaxSizeNoSplit)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvInitializeNewRingbuffer .text.prvInitializeNewRingbuffer)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvReceiveGeneric .text.prvReceiveGeneric)
    *libesp_ringbuf.a:ringbuf.*(.literal.prvSendAcquireGeneric .text.prvSendAcquireGeneric)
    *libesp_ringbuf.a:ringbuf.*(.literal.vRingbufferDelete .text.vRingbufferDelete)
    *libesp_ringbuf.a:ringbuf.*(.literal.vRingbufferGetInfo .text.vRingbufferGetInfo)
    *libesp_ringbuf.a:ringbuf.*(.literal.vRingbufferReturnItem .text.vRingbufferReturnItem)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferAddToQueueSetRead .text.xRingbufferAddToQueueSetRead)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferCreate .text.xRingbufferCreate)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferCreateNoSplit .text.xRingbufferCreateNoSplit)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferCreateStatic .text.xRingbufferCreateStatic)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferGetCurFreeSize .text.xRingbufferGetCurFreeSize)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferGetMaxItemSize .text.xRingbufferGetMaxItemSize)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferPrintInfo .text.xRingbufferPrintInfo)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferReceive .text.xRingbufferReceive)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferReceiveSplit .text.xRingbufferReceiveSplit)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferReceiveUpTo .text.xRingbufferReceiveUpTo)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferRemoveFromQueueSetRead .text.xRingbufferRemoveFromQueueSetRead)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferSend .text.xRingbufferSend)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferSendAcquire .text.xRingbufferSendAcquire)
    *libesp_ringbuf.a:ringbuf.*(.literal.xRingbufferSendComplete .text.xRingbufferSendComplete)
    *libesp_system.a:esp_system_chip.*(.literal.esp_get_free_heap_size .literal.esp_get_free_internal_heap_size .literal.esp_get_idf_version .literal.esp_get_minimum_free_heap_size .text .text.esp_get_free_heap_size .text.esp_get_free_internal_heap_size .text.esp_get_idf_version .text.esp_get_minimum_free_heap_size)
    *libesp_system.a:freertos_hooks.*(.literal.esp_deregister_freertos_idle_hook .literal.esp_deregister_freertos_idle_hook_for_cpu .literal.esp_deregister_freertos_tick_hook .literal.esp_deregister_freertos_tick_hook_for_cpu .literal.esp_register_freertos_idle_hook .literal.esp_register_freertos_idle_hook_for_cpu .literal.esp_register_freertos_tick_hook .literal.esp_register_freertos_tick_hook_for_cpu .literal.esp_vApplicationIdleHook .text .text.esp_deregister_freertos_idle_hook .text.esp_deregister_freertos_idle_hook_for_cpu .text.esp_deregister_freertos_tick_hook .text.esp_deregister_freertos_tick_hook_for_cpu .text.esp_register_freertos_idle_hook .text.esp_register_freertos_idle_hook_for_cpu .text.esp_register_freertos_tick_hook .text.esp_register_freertos_tick_hook_for_cpu .text.esp_vApplicationIdleHook)
    *libesp_system.a:panic.*(.literal.esp_panic_handler .literal.esp_panic_handler_disable_timg_wdts .literal.esp_panic_handler_enable_rtc_wdt .literal.esp_panic_handler_feed_wdts .literal.esp_panic_handler_increment_entry_count .literal.panic_print_char .literal.panic_print_char_uart .literal.panic_print_dec .literal.panic_print_hex .literal.panic_print_str .literal.print_abort_details .text .text.esp_panic_handler .text.esp_panic_handler_disable_timg_wdts .text.esp_panic_handler_enable_rtc_wdt .text.esp_panic_handler_feed_wdts .text.esp_panic_handler_increment_entry_count .text.esp_reset_reason_get_hint .text.esp_reset_reason_set_hint .text.panic_print_char .text.panic_print_char_uart .text.panic_print_dec .text.panic_print_hex .text.panic_print_str .text.print_abort_details)
    *libesp_system.a:reset_reason.*(.literal.esp_reset_reason .literal.esp_reset_reason_get_hint .literal.startup.esp_reset_reason_init .text .text.esp_reset_reason .text.esp_reset_reason_get_hint .text.startup.esp_reset_reason_init)
    *libesp_system.a:stack_check.*(.literal.startup.__esp_stack_guard_setup .text .text.startup.__esp_stack_guard_setup)
    *libesp_system.a:system_internal.*(.text)
    *libesp_system.a:system_time.*(.text)
    *libesp_timer.a:esp_timer_impl_common.*(.text)
    *libesp_timer.a:esp_timer_impl_lac.*(.literal.esp_timer_impl_deinit .literal.esp_timer_impl_early_init .literal.esp_timer_impl_get_alarm_reg .literal.esp_timer_impl_init .text .text.esp_timer_impl_deinit .text.esp_timer_impl_early_init .text.esp_timer_impl_get_alarm_reg .text.esp_timer_impl_init)
    *libfreertos.a:app_startup.*(.literal .literal.* .text .text.*)
    *libfreertos.a:event_groups.*(.literal.uxEventGroupGetNumber .text.uxEventGroupGetNumber)
    *libfreertos.a:event_groups.*(.literal.vEventGroupClearBitsCallback .text.vEventGroupClearBitsCallback)
    *libfreertos.a:event_groups.*(.literal.vEventGroupDelete .text.vEventGroupDelete)
    *libfreertos.a:event_groups.*(.literal.vEventGroupSetBitsCallback .text.vEventGroupSetBitsCallback)
    *libfreertos.a:event_groups.*(.literal.vEventGroupSetNumber .text.vEventGroupSetNumber)
    *libfreertos.a:event_groups.*(.literal.xEventGroupClearBits .text.xEventGroupClearBits)
    *libfreertos.a:event_groups.*(.literal.xEventGroupCreate .text.xEventGroupCreate)
    *libfreertos.a:event_groups.*(.literal.xEventGroupCreateStatic .text.xEventGroupCreateStatic)
    *libfreertos.a:event_groups.*(.literal.xEventGroupGetStaticBuffer .text.xEventGroupGetStaticBuffer)
    *libfreertos.a:event_groups.*(.literal.xEventGroupSetBits .text.xEventGroupSetBits)
    *libfreertos.a:event_groups.*(.literal.xEventGroupSync .text.xEventGroupSync)
    *libfreertos.a:event_groups.*(.literal.xEventGroupWaitBits .text.xEventGroupWaitBits)
    *libfreertos.a:freertos_compatibility.*(.literal .literal.* .text .text.*)
    *libfreertos.a:heap_idf.*(.literal.pvPortMalloc .text.pvPortMalloc)
    *libfreertos.a:heap_idf.*(.literal.vPortFree .text.vPortFree)
    *libfreertos.a:heap_idf.*(.literal.xPortCheckValidListMem .text.xPortCheckValidListMem)
    *libfreertos.a:heap_idf.*(.literal.xPortCheckValidTCBMem .text.xPortCheckValidTCBMem)
    *libfreertos.a:heap_idf.*(.literal.xPortGetFreeHeapSize .text.xPortGetFreeHeapSize)
    *libfreertos.a:heap_idf.*(.literal.xPortGetMinimumEverFreeHeapSize .text.xPortGetMinimumEverFreeHeapSize)
    *libfreertos.a:heap_idf.*(.literal.xPortcheckValidStackMem .text.xPortcheckValidStackMem)
    *libfreertos.a:idf_additions.*(.literal .literal.* .text .text.*)
    *libfreertos.a:idf_additions_event_groups.*(.literal .literal.* .text .text.*)
    *libfreertos.a:list.*(.literal.vListInitialise .text.vListInitialise)
    *libfreertos.a:list.*(.literal.vListInitialiseItem .text.vListInitialiseItem)
    *libfreertos.a:list.*(.literal.vListInsert .text.vListInsert)
    *libfreertos.a:port.*(.literal.pxPortInitialiseStack .text.pxPortInitialiseStack)
    *libfreertos.a:port.*(.literal.vPortTCBPreDeleteHook .text.vPortTCBPreDeleteHook)
    *libfreertos.a:port.*(.literal.vPortTaskWrapper .text.vPortTaskWrapper)
    *libfreertos.a:port.*(.literal.xPortGetTickRateHz .text.xPortGetTickRateHz)
    *libfreertos.a:port.*(.literal.xPortStartScheduler .text.xPortStartScheduler)
    *libfreertos.a:port_common.*(.literal.vApplicationGetIdleTaskMemory .text.vApplicationGetIdleTaskMemory)
    *libfreertos.a:port_common.*(.literal.vApplicationGetTimerTaskMemory .text.vApplicationGetTimerTaskMemory)
    *libfreertos.a:port_systick.*(.literal.vPortSetupTimer .text.vPortSetupTimer)
    *libfreertos.a:queue.*(.literal.ucQueueGetQueueType .text.ucQueueGetQueueType)
    *libfreertos.a:queue.*(.literal.uxQueueGetQueueNumber .text.uxQueueGetQueueNumber)
    *libfreertos.a:queue.*(.literal.uxQueueMessagesWaiting .text.uxQueueMessagesWaiting)
    *libfreertos.a:queue.*(.literal.uxQueueSpacesAvailable .text.uxQueueSpacesAvailable)
    *libfreertos.a:queue.*(.literal.vQueueDelete .text.vQueueDelete)
    *libfreertos.a:queue.*(.literal.vQueueSetQueueNumber .text.vQueueSetQueueNumber)
    *libfreertos.a:queue.*(.literal.vQueueWaitForMessageRestricted .text.vQueueWaitForMessageRestricted)
    *libfreertos.a:queue.*(.literal.xQueueAddToSet .text.xQueueAddToSet)
    *libfreertos.a:queue.*(.literal.xQueueCreateCountingSemaphore .text.xQueueCreateCountingSemaphore)
    *libfreertos.a:queue.*(.literal.xQueueCreateCountingSemaphoreStatic .text.xQueueCreateCountingSemaphoreStatic)
    *libfreertos.a:queue.*(.literal.xQueueCreateMutex .text.xQueueCreateMutex)
    *libfreertos.a:queue.*(.literal.xQueueCreateMutexStatic .text.xQueueCreateMutexStatic)
    *libfreertos.a:queue.*(.literal.xQueueCreateSet .text.xQueueCreateSet)
    *libfreertos.a:queue.*(.literal.xQueueGenericCreate .text.xQueueGenericCreate)
    *libfreertos.a:queue.*(.literal.xQueueGenericCreateStatic .text.xQueueGenericCreateStatic)
    *libfreertos.a:queue.*(.literal.xQueueGenericGetStaticBuffers .text.xQueueGenericGetStaticBuffers)
    *libfreertos.a:queue.*(.literal.xQueueGenericReset .text.xQueueGenericReset)
    *libfreertos.a:queue.*(.literal.xQueueGenericSend .text.xQueueGenericSend)
    *libfreertos.a:queue.*(.literal.xQueueGetMutexHolder .text.xQueueGetMutexHolder)
    *libfreertos.a:queue.*(.literal.xQueueGiveMutexRecursive .text.xQueueGiveMutexRecursive)
    *libfreertos.a:queue.*(.literal.xQueuePeek .text.xQueuePeek)
    *libfreertos.a:queue.*(.literal.xQueueReceive .text.xQueueReceive)
    *libfreertos.a:queue.*(.literal.xQueueRemoveFromSet .text.xQueueRemoveFromSet)
    *libfreertos.a:queue.*(.literal.xQueueSelectFromSet .text.xQueueSelectFromSet)
    *libfreertos.a:queue.*(.literal.xQueueSemaphoreTake .text.xQueueSemaphoreTake)
    *libfreertos.a:queue.*(.literal.xQueueTakeMutexRecursive .text.xQueueTakeMutexRecursive)
    *libfreertos.a:stream_buffer.*(.literal.prvReadBytesFromBuffer .text.prvReadBytesFromBuffer)
    *libfreertos.a:stream_buffer.*(.literal.prvWriteBytesToBuffer .text.prvWriteBytesToBuffer)
    *libfreertos.a:stream_buffer.*(.literal.ucStreamBufferGetStreamBufferType .text.ucStreamBufferGetStreamBufferType)
    *libfreertos.a:stream_buffer.*(.literal.uxStreamBufferGetStreamBufferNumber .text.uxStreamBufferGetStreamBufferNumber)
    *libfreertos.a:stream_buffer.*(.literal.vStreamBufferDelete .text.vStreamBufferDelete)
    *libfreertos.a:stream_buffer.*(.literal.vStreamBufferSetStreamBufferNumber .text.vStreamBufferSetStreamBufferNumber)
    *libfreertos.a:stream_buffer.*(.literal.xStreamBufferBytesAvailable .text.xStreamBufferBytesAvailable)
    *libfreertos.a:stream_buffer.*(.literal.xStreamBufferGenericCreate .text.xStreamBufferGenericCreate)
    *libfreertos.a:stream_buffer.*(.literal.xStreamBufferGenericCreateStatic .text.xStreamBufferGenericCreateStatic)
    *libfreertos.a:stream_buffer.*(.literal.xStreamBufferGetStaticBuffers .text.xStreamBufferGetStaticBuffers)
    *libfreertos.a:stream_buffer.*(.literal.xStreamBufferIsEmpty .text.xStreamBufferIsEmpty)
    *libfreertos.a:stream_buffer.*(.literal.xStreamBufferIsFull .text.xStreamBufferIsFull)
    *libfreertos.a:stream_buffer.*(.literal.xStreamBufferNextMessageLengthBytes .text.xStreamBufferNextMessageLengthBytes)
    *libfreertos.a:stream_buffer.*(.literal.xStreamBufferReceive .text.xStreamBufferReceive)
    *libfreertos.a:stream_buffer.*(.literal.xStreamBufferReset .text.xStreamBufferReset)
    *libfreertos.a:stream_buffer.*(.literal.xStreamBufferSend .text.xStreamBufferSend)
    *libfreertos.a:stream_buffer.*(.literal.xStreamBufferSetTriggerLevel .text.xStreamBufferSetTriggerLevel)
    *libfreertos.a:stream_buffer.*(.literal.xStreamBufferSpacesAvailable .text.xStreamBufferSpacesAvailable)
    *libfreertos.a:tasks.*(.literal.__getreent .text.__getreent)
    *libfreertos.a:tasks.*(.literal.eTaskGetState .text.eTaskGetState)
    *libfreertos.a:tasks.*(.literal.pcTaskGetName .text.pcTaskGetName)
    *libfreertos.a:tasks.*(.literal.prvAddCurrentTaskToDelayedList .text.prvAddCurrentTaskToDelayedList)
    *libfreertos.a:tasks.*(.literal.prvAddNewTaskToReadyList .text.prvAddNewTaskToReadyList)
    *libfreertos.a:tasks.*(.literal.prvDeleteTCB .text.prvDeleteTCB)
    *libfreertos.a:tasks.*(.literal.prvIdleTask .text.prvIdleTask)
    *libfreertos.a:tasks.*(.literal.prvListTasksWithinSingleList .text.prvListTasksWithinSingleList)
    *libfreertos.a:tasks.*(.literal.prvReleaseKernelLock .text.prvReleaseKernelLock)
    *libfreertos.a:tasks.*(.literal.prvSearchForNameWithinSingleList .text.prvSearchForNameWithinSingleList)
    *libfreertos.a:tasks.*(.literal.prvTakeKernelLock .text.prvTakeKernelLock)
    *libfreertos.a:tasks.*(.literal.prvTaskCreateDynamicPinnedToCoreWithCaps .text.prvTaskCreateDynamicPinnedToCoreWithCaps)
    *libfreertos.a:tasks.*(.literal.prvTaskPriorityRaise .text.prvTaskPriorityRaise)
    *libfreertos.a:tasks.*(.literal.prvTaskPriorityRestore .text.prvTaskPriorityRestore)
    *libfreertos.a:tasks.*(.literal.prvWriteNameToBuffer .text.prvWriteNameToBuffer)
    *libfreertos.a:tasks.*(.literal.pvTaskGetCurrentTCBForCore .text.pvTaskGetCurrentTCBForCore)
    *libfreertos.a:tasks.*(.literal.pvTaskGetThreadLocalStoragePointer .text.pvTaskGetThreadLocalStoragePointer)
    *libfreertos.a:tasks.*(.literal.pvTaskIncrementMutexHeldCount .text.pvTaskIncrementMutexHeldCount)
    *libfreertos.a:tasks.*(.literal.pxTaskGetStackStart .text.pxTaskGetStackStart)
    *libfreertos.a:tasks.*(.literal.ulTaskGenericNotifyTake .text.ulTaskGenericNotifyTake)
    *libfreertos.a:tasks.*(.literal.ulTaskGenericNotifyValueClear .text.ulTaskGenericNotifyValueClear)
    *libfreertos.a:tasks.*(.literal.ulTaskGetIdleRunTimeCounter .text.ulTaskGetIdleRunTimeCounter)
    *libfreertos.a:tasks.*(.literal.ulTaskGetIdleRunTimeCounterForCore .text.ulTaskGetIdleRunTimeCounterForCore)
    *libfreertos.a:tasks.*(.literal.ulTaskGetIdleRunTimePercent .text.ulTaskGetIdleRunTimePercent)
    *libfreertos.a:tasks.*(.literal.ulTaskGetIdleRunTimePercentForCore .text.ulTaskGetIdleRunTimePercentForCore)
    *libfreertos.a:tasks.*(.literal.uxTaskGetNumberOfTasks .text.uxTaskGetNumberOfTasks)
    *libfreertos.a:tasks.*(.literal.uxTaskGetSnapshotAll .text.uxTaskGetSnapshotAll)
    *libfreertos.a:tasks.*(.literal.uxTaskGetStackHighWaterMark .text.uxTaskGetStackHighWaterMark)
    *libfreertos.a:tasks.*(.literal.uxTaskGetStackHighWaterMark2 .text.uxTaskGetStackHighWaterMark2)
    *libfreertos.a:tasks.*(.literal.uxTaskGetSystemState .text.uxTaskGetSystemState)
    *libfreertos.a:tasks.*(.literal.uxTaskGetTaskNumber .text.uxTaskGetTaskNumber)
    *libfreertos.a:tasks.*(.literal.uxTaskPriorityGet .text.uxTaskPriorityGet)
    *libfreertos.a:tasks.*(.literal.uxTaskResetEventItemValue .text.uxTaskResetEventItemValue)
    *libfreertos.a:tasks.*(.literal.vTaskDelay .text.vTaskDelay)
    *libfreertos.a:tasks.*(.literal.vTaskDelete .text.vTaskDelete)
    *libfreertos.a:tasks.*(.literal.vTaskEndScheduler .text.vTaskEndScheduler)
    *libfreertos.a:tasks.*(.literal.vTaskGetInfo .text.vTaskGetInfo)
    *libfreertos.a:tasks.*(.literal.vTaskGetRunTimeStats .text.vTaskGetRunTimeStats)
    *libfreertos.a:tasks.*(.literal.vTaskInternalSetTimeOutState .text.vTaskInternalSetTimeOutState)
    *libfreertos.a:tasks.*(.literal.vTaskList .text.vTaskList)
    *libfreertos.a:tasks.*(.literal.vTaskMissedYield .text.vTaskMissedYield)
    *libfreertos.a:tasks.*(.literal.vTaskPlaceOnEventList .text.vTaskPlaceOnEventList)
    *libfreertos.a:tasks.*(.literal.vTaskPlaceOnEventListRestricted .text.vTaskPlaceOnEventListRestricted)
    *libfreertos.a:tasks.*(.literal.vTaskPlaceOnUnorderedEventList .text.vTaskPlaceOnUnorderedEventList)
    *libfreertos.a:tasks.*(.literal.vTaskPriorityDisinheritAfterTimeout .text.vTaskPriorityDisinheritAfterTimeout)
    *libfreertos.a:tasks.*(.literal.vTaskPrioritySet .text.vTaskPrioritySet)
    *libfreertos.a:tasks.*(.literal.vTaskRemoveFromUnorderedEventList .text.vTaskRemoveFromUnorderedEventList)
    *libfreertos.a:tasks.*(.literal.vTaskResume .text.vTaskResume)
    *libfreertos.a:tasks.*(.literal.vTaskSetTaskNumber .text.vTaskSetTaskNumber)
    *libfreertos.a:tasks.*(.literal.vTaskSetThreadLocalStoragePointer .text.vTaskSetThreadLocalStoragePointer)
    *libfreertos.a:tasks.*(.literal.vTaskSetThreadLocalStoragePointerAndDelCallback .text.vTaskSetThreadLocalStoragePointerAndDelCallback)
    *libfreertos.a:tasks.*(.literal.vTaskSetTimeOutState .text.vTaskSetTimeOutState)
    *libfreertos.a:tasks.*(.literal.vTaskStartScheduler .text.vTaskStartScheduler)
    *libfreertos.a:tasks.*(.literal.vTaskSuspend .text.vTaskSuspend)
    *libfreertos.a:tasks.*(.literal.vTaskSuspendAll .text.vTaskSuspendAll)
    *libfreertos.a:tasks.*(.literal.xTaskAbortDelay .text.xTaskAbortDelay)
    *libfreertos.a:tasks.*(.literal.xTaskCatchUpTicks .text.xTaskCatchUpTicks)
    *libfreertos.a:tasks.*(.literal.xTaskCheckForTimeOut .text.xTaskCheckForTimeOut)
    *libfreertos.a:tasks.*(.literal.xTaskCreatePinnedToCore .text.xTaskCreatePinnedToCore)
    *libfreertos.a:tasks.*(.literal.xTaskCreateStaticPinnedToCore .text.xTaskCreateStaticPinnedToCore)
    *libfreertos.a:tasks.*(.literal.xTaskDelayUntil .text.xTaskDelayUntil)
    *libfreertos.a:tasks.*(.literal.xTaskGenericNotify .text.xTaskGenericNotify)
    *libfreertos.a:tasks.*(.literal.xTaskGenericNotifyStateClear .text.xTaskGenericNotifyStateClear)
    *libfreertos.a:tasks.*(.literal.xTaskGenericNotifyWait .text.xTaskGenericNotifyWait)
    *libfreertos.a:tasks.*(.literal.xTaskGetCoreID .text.xTaskGetCoreID)
    *libfreertos.a:tasks.*(.literal.xTaskGetCurrentTaskHandle .text.xTaskGetCurrentTaskHandle)
    *libfreertos.a:tasks.*(.literal.xTaskGetCurrentTaskHandleForCore .text.xTaskGetCurrentTaskHandleForCore)
    *libfreertos.a:tasks.*(.literal.xTaskGetHandle .text.xTaskGetHandle)
    *libfreertos.a:tasks.*(.literal.xTaskGetIdleTaskHandle .text.xTaskGetIdleTaskHandle)
    *libfreertos.a:tasks.*(.literal.xTaskGetIdleTaskHandleForCore .text.xTaskGetIdleTaskHandleForCore)
    *libfreertos.a:tasks.*(.literal.xTaskGetNext .text.xTaskGetNext)
    *libfreertos.a:tasks.*(.literal.xTaskGetStaticBuffers .text.xTaskGetStaticBuffers)
    *libfreertos.a:tasks.*(.literal.xTaskPriorityDisinherit .text.xTaskPriorityDisinherit)
    *libfreertos.a:tasks.*(.literal.xTaskPriorityInherit .text.xTaskPriorityInherit)
    *libfreertos.a:tasks.*(.literal.xTaskResumeAll .text.xTaskResumeAll)
    *libfreertos.a:timers.*(.literal.pcTimerGetName .text.pcTimerGetName)
    *libfreertos.a:timers.*(.literal.prvCheckForValidListAndQueue .text.prvCheckForValidListAndQueue)
    *libfreertos.a:timers.*(.literal.prvInsertTimerInActiveList .text.prvInsertTimerInActiveList)
    *libfreertos.a:timers.*(.literal.prvProcessExpiredTimer .text.prvProcessExpiredTimer)
    *libfreertos.a:timers.*(.literal.prvReloadTimer .text.prvReloadTimer)
    *libfreertos.a:timers.*(.literal.prvSwitchTimerLists .text.prvSwitchTimerLists)
    *libfreertos.a:timers.*(.literal.prvTimerTask .text.prvTimerTask)
    *libfreertos.a:timers.*(.literal.pvTimerGetTimerID .text.pvTimerGetTimerID)
    *libfreertos.a:timers.*(.literal.uxTimerGetReloadMode .text.uxTimerGetReloadMode)
    *libfreertos.a:timers.*(.literal.uxTimerGetTimerNumber .text.uxTimerGetTimerNumber)
    *libfreertos.a:timers.*(.literal.vTimerSetReloadMode .text.vTimerSetReloadMode)
    *libfreertos.a:timers.*(.literal.vTimerSetTimerID .text.vTimerSetTimerID)
    *libfreertos.a:timers.*(.literal.vTimerSetTimerNumber .text.vTimerSetTimerNumber)
    *libfreertos.a:timers.*(.literal.xTimerCreate .text.xTimerCreate)
    *libfreertos.a:timers.*(.literal.xTimerCreateStatic .text.xTimerCreateStatic)
    *libfreertos.a:timers.*(.literal.xTimerCreateTimerTask .text.xTimerCreateTimerTask)
    *libfreertos.a:timers.*(.literal.xTimerGetExpiryTime .text.xTimerGetExpiryTime)
    *libfreertos.a:timers.*(.literal.xTimerGetPeriod .text.xTimerGetPeriod)
    *libfreertos.a:timers.*(.literal.xTimerGetReloadMode .text.xTimerGetReloadMode)
    *libfreertos.a:timers.*(.literal.xTimerGetStaticBuffer .text.xTimerGetStaticBuffer)
    *libfreertos.a:timers.*(.literal.xTimerGetTimerDaemonTaskHandle .text.xTimerGetTimerDaemonTaskHandle)
    *libfreertos.a:timers.*(.literal.xTimerIsTimerActive .text.xTimerIsTimerActive)
    *libfreertos.a:timers.*(.literal.xTimerPendFunctionCall .text.xTimerPendFunctionCall)
    *libhal.a:gpio_hal.*(.literal.gpio_hal_intr_disable .literal.gpio_hal_intr_enable_on_core .literal.gpio_hal_iomux_in .literal.gpio_hal_iomux_out .text .text.gpio_hal_intr_disable .text.gpio_hal_intr_enable_on_core .text.gpio_hal_iomux_in .text.gpio_hal_iomux_out)
    *libhal.a:timer_hal.*(.literal.timer_hal_deinit .literal.timer_hal_init .text .text.timer_hal_deinit .text.timer_hal_init .text.timer_hal_set_counter_value)
    *libheap.a:multi_heap.*(.literal.multi_heap_check .literal.multi_heap_dump .literal.multi_heap_dump_tlsf .literal.multi_heap_find_containing_block_impl .literal.multi_heap_get_info_impl .literal.multi_heap_register_impl .literal.multi_heap_reset_minimum_free_bytes .literal.multi_heap_restore_minimum_free_bytes .literal.multi_heap_walk .literal.tlsf_check_hook .text .text.multi_heap_check .text.multi_heap_dump .text.multi_heap_dump_tlsf .text.multi_heap_find_containing_block_impl .text.multi_heap_free_size_impl .text.multi_heap_get_info_impl .text.multi_heap_get_info_tlsf .text.multi_heap_minimum_free_size_impl .text.multi_heap_register_impl .text.multi_heap_reset_minimum_free_bytes .text.multi_heap_restore_minimum_free_bytes .text.multi_heap_walk .text.tlsf_check_hook)
    *libheap.a:multi_heap_poisoning.*(.literal.multi_heap_find_containing_block .literal.multi_heap_free_size .literal.multi_heap_get_info .literal.multi_heap_minimum_free_size .literal.multi_heap_register .text .text.multi_heap_find_containing_block .text.multi_heap_free_size .text.multi_heap_get_info .text.multi_heap_minimum_free_size .text.multi_heap_register)
    *libheap.a:tlsf.*(.literal.default_walker .literal.integrity_walker .literal.tlsf_add_pool .literal.tlsf_check .literal.tlsf_check_pool .literal.tlsf_create .literal.tlsf_create_with_pool .literal.tlsf_malloc_addr .literal.tlsf_remove_pool .literal.tlsf_walk_pool .text .text.default_walker .text.integrity_walker .text.tlsf_add_pool .text.tlsf_check .text.tlsf_check_pool .text.tlsf_create .text.tlsf_create_with_pool .text.tlsf_destroy .text.tlsf_find_containing_block .text.tlsf_fit_size .text.tlsf_malloc_addr .text.tlsf_pool_overhead .text.tlsf_remove_pool .text.tlsf_walk_pool)
    *liblog.a:log_timestamp.*(.text)
    *liblog.a:log_write.*(.literal.esp_log_set_vprintf .text .text.esp_log_set_vprintf)
    *liblog.a:tag_log_level.*(.literal.esp_log_level_get .literal.esp_log_level_set .literal.log_level_get .text .text.esp_log_level_get .text.esp_log_level_set .text.log_level_get)
    *libnewlib.a:esp_time_impl.*(.literal.esp_sync_timekeeping_timers .literal.esp_time_impl_get_time .literal.esp_time_impl_get_time_since_boot .text .text.esp_sync_timekeeping_timers .text.esp_time_impl_get_time .text.esp_time_impl_get_time_since_boot)
    *libphy.a:(.phyiram .phyiram.*)
    *libspi_flash.a:esp_flash_api.*(.literal.esp_flash_app_disable_protect .literal.esp_flash_get_protectable_regions .literal.esp_flash_read_chip_id .literal.esp_flash_read_id .literal.esp_flash_read_unique_chip_id .literal.esp_flash_suspend_cmd_init .literal.find_region .text .text.esp_flash_app_disable_protect .text.esp_flash_get_protectable_regions .text.esp_flash_read_chip_id .text.esp_flash_read_id .text.esp_flash_read_unique_chip_id .text.esp_flash_suspend_cmd_init .text.find_region)
    *libspi_flash.a:spi_flash_os_func_app.*(.literal.esp_flash_app_enable_os_functions .literal.esp_flash_deinit_os_functions .literal.esp_flash_init_os_functions .text .text.esp_flash_app_enable_os_functions .text.esp_flash_deinit_os_functions .text.esp_flash_init_main_bus_lock .text.esp_flash_init_os_functions .text.esp_flash_set_dangerous_write_protection .text.use_bus_lock)
    *libspi_flash.a:spi_flash_os_func_noos.*(.text)
    *libxtensa.a:xt_trax.*(.literal .literal.* .text .text.*)
    *libxtensa.a:xtensa_intr.*(.literal .literal.* .text .text.*)
    *(.stub)
    *(.gnu.warning)
    *(.gnu.linkonce.literal.* .gnu.linkonce.t.*.literal .gnu.linkonce.t.*)
    *(.irom0.text) /* catch stray ICACHE_RODATA_ATTR */
    /**
     * CPU will try to prefetch up to 16 bytes of of instructions.
     * This means that any configuration (e.g. MMU, PMS) must allow
     * safe access to up to 16 bytes after the last real instruction,
     * add dummy bytes to ensure this.
     */
    . += 16;
    _text_end = ABSOLUTE(.);
    /**
     * Mark the flash.text end.
     * This can be used for MMU driver to maintain virtual address.
     */
    _instruction_reserved_end = ABSOLUTE(.);
    _etext = .;
    /**
     * Similar to _iram_start, this symbol goes here so it is
     * resolved by addr2line in preference to the first symbol in
     * the flash.text segment.
     */
    _flash_cache_start = ABSOLUTE(0);
  } > default_code_seg
  /* Marks the end of IRAM code segment */
  .iram0.text_end (NOLOAD) :
  {
    
 . = ALIGN(4); 
 _iram_text_end = ABSOLUTE(.);
  } > iram0_0_seg
  .iram0.data :
  {
    
 . = ALIGN(4); 
 _iram_data_start = ABSOLUTE(.);
    *(.iram.data .iram.data.*)
    _coredump_iram_start = ABSOLUTE(.);
    *(.iram2.coredump .iram2.coredump.*)
    _coredump_iram_end = ABSOLUTE(.);
    _iram_data_end = ABSOLUTE(.);
  } > iram0_0_seg
  .iram0.bss (NOLOAD) :
  {
    
 . = ALIGN(4); 
 _iram_bss_start = ABSOLUTE(.);
    *(.iram.bss .iram.bss.*)
    _iram_bss_end = ABSOLUTE(.);
    
 . = ALIGN(4); 
 _iram_end = ABSOLUTE(.);
  } > iram0_0_seg
  /* Marks the end of data, bss and possibly rodata  */
  .dram0.heap_start (NOLOAD) :
  {
    /* Lowest possible start address for the heap */
    
 . = ALIGN(8); 
 _heap_low_start = ABSOLUTE(.);
  } > dram0_0_seg
  /**
   * This section is not included in the binary image; it is only present in the ELF file.
   * It is used to keep certain symbols in the ELF file.
   */
  .noload 0 (INFO) :
  {
    _noload_keep_in_elf_start = ABSOLUTE(.);
    KEEP(*(.noload_keep_in_elf .noload_keep_in_elf.*))
    _noload_keep_in_elf_end = ABSOLUTE(.);
  }
  /* DWARF 1 */
  .debug 0 : { *(.debug) }
  .line 0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo 0 : { *(.debug_srcinfo) }
  .debug_sfnames 0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges 0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info 0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev 0 : { *(.debug_abbrev) }
  .debug_line 0 : { *(.debug_line) }
  .debug_frame 0 : { *(.debug_frame) }
  .debug_str 0 : { *(.debug_str) }
  .debug_loc 0 : { *(.debug_loc) }
  .debug_macinfo 0 : { *(.debug_macinfo) }
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  /* DWARF 3 */
  .debug_ranges 0 : { *(.debug_ranges) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames 0 : { *(.debug_varnames) }
  /* GNU DWARF 2 extensions */
  .debug_gnu_pubnames 0 : { *(.debug_gnu_pubnames) }
  .debug_gnu_pubtypes 0 : { *(.debug_gnu_pubtypes) }
  /* DWARF 4 */
  .debug_types 0 : { *(.debug_types) }
  /* DWARF 5 */
  .debug_addr 0 : { *(.debug_addr) }
  .debug_line_str 0 : { *(.debug_line_str) }
  .debug_loclists 0 : { *(.debug_loclists) }
  .debug_macro 0 : { *(.debug_macro) }
  .debug_names 0 : { *(.debug_names) }
  .debug_rnglists 0 : { *(.debug_rnglists) }
  .debug_str_offsets 0 : { *(.debug_str_offsets) }
  .comment 0 : { *(.comment) }
  .note.GNU-stack 0: { *(.note.GNU-stack) }
/**
 * .xt.prop and .xt.lit sections will be used by the debugger and disassembler
 * to get more information about raw data present in the code.
 * Indeed, it may be required to add some padding at some points in the code
 * in order to align a branch/jump destination on a particular bound.
 * Padding these instructions will generate null bytes that shall be
 * interpreted as data, and not code by the debugger or disassembler.
 * This section will only be present in the ELF file, not in the final binary
 * For more details, check GCC-212
 */
  .xtensa.info 0: { *(.xtensa.info) }
  .xt.prop 0 : { *(.xt.prop .xt.prop.* .gnu.linkonce.prop.*) }
  .xt.lit 0 : { *(.xt.lit .xt.lit.* .gnu.linkonce.p.*) }
  /DISCARD/ :
  {
   *(.fini)
   *(.eh_frame_hdr)
  }
}
ASSERT(((_iram_end - ORIGIN(iram0_0_seg)) <= LENGTH(iram0_0_seg)),
          "IRAM0 segment data does not fit.")
ASSERT(((_heap_low_start - ORIGIN(dram0_0_seg)) <= LENGTH(dram0_0_seg)),
          "DRAM segment data does not fit.")
