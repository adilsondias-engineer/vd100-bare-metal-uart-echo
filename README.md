# vd100-bare-metal-uart-echo

Bare-metal PS UART echo application for the VD100 board (Versal AI Edge XCVE2302).

Demonstrates PS UART0 receive and transmit on bare-metal — no Linux, no Yocto, no device
tree. Designed as the minimal foundation for HW engineers who need a fast debug output
path without building a full Linux platform.

---

## What It Does

```
PC terminal (minicom) → type text → Enter
    → CP2102 USB-UART → LPD_MIO16/17 → PS UART0
        → A72 bare-metal app receives → echoes back
    → PS UART0 → LPD_MIO16/17 → CP2102 → terminal
```

Output:
```
========================================
  VD100 Bare-Metal UART Echo
  XCVE2302 | PS UART0 | 115200 baud
========================================
Type anything, it will be echoed back...

[Type]  >> hello world
[VD100] >> hello world
[Type]  >> it works
[VD100] >> it works
```

The `[VD100] >>` prefix makes it unambiguous that the FPGA received and echoed the text —
not just local terminal echo.

---

## Hardware

| Item | Value |
|------|-------|
| Board | VD100 |
| Device | XCVE2302-SFVA784-1LP-E-S |
| UART | PS UART0 — LPD_MIO16 (RX) / LPD_MIO17 (TX) |
| USB-UART | CP2102 (Silicon Labs) — MINI USB connector |
| Baud rate | 115200 |
| Vitis | 2025.2 |

---

## Repository Structure

```
vd100-bare-metal-uart-echo/
├── src/
│   └── main.cpp          # UART echo application
└── README.md
```

Platform and system project are shared with `vd100-bare-metal-ma-aie-app` —
see `vd100-bare-metal-aie-platform` and `vd100-bare-metal-system-project`.

---

## Build

This component is an app component inside `vd100-bare-metal-system-project`.

```
1. vd100_bd_aie_pipeline (Vivado) → Export XSA
        │
        ▼
2. vd100-bare-metal-aie-platform (Vitis)
        │  Platform from XSA, BSP with aiebaremetal enabled
        ▼
3. vd100-bare-metal-system-project (Vitis)
        │  App component: vd100-bare-metal-uart-echo
        │  v++ package → BOOT.BIN + SD image
        ▼
4. Copy BOOT.BIN to SD card FAT partition → boot VD100
```

---

## Terminal Setup

```bash
# Install minicom
sudo apt install minicom

# Connect — /dev/ttyUSB0 may vary
minicom -D /dev/ttyUSB0 -b 115200

# Disable local echo in minicom for clean output:
# Ctrl+A → E (toggles local echo off)
```

---

## Critical Notes

### XUartPsv_IsReceiveData() is required before RecvByte()

Without polling for available data first, `XUartPsv_RecvByte()` reads from
an empty FIFO and echoes garbage characters continuously.

```cpp
// CORRECT
while (!XUartPsv_IsReceiveData(XPAR_XUARTPSV_0_BASEADDR));
u8 c = XUartPsv_RecvByte(XPAR_XUARTPSV_0_BASEADDR);

// WRONG — reads garbage from empty FIFO
u8 c = XUartPsv_RecvByte(XPAR_XUARTPSV_0_BASEADDR);
```

### UART hardware is PS MIO — not PL I/O

On VD100, UART is connected to PS LPD_MIO16/17 via CP2102. It cannot be assigned
in XDC constraints. Bare-metal access uses the `xuartpsv` BSP driver.

This is a fundamental difference from 7-series and UltraScale boards where UART
is typically routed through PL I/O and assigned in XDC.

### Why bare-metal for UART?

On Versal, getting UART output via Linux requires: CIPS configuration, Yocto build,
device tree, boot sequence — typically 2-3 hours minimum. Bare-metal gives immediate
UART output with a simple BSP. For HW engineers validating PL logic, bare-metal is
the correct starting point.

---

## Versal Bare-Metal Series

This is part of a series of bare-metal VD100 projects:

| Repo | Description |
|------|-------------|
| `vd100-bare-metal-uart-echo` | **This repo** — PS UART echo foundation |
| `vd100-bare-metal-ma-aie-app` | Auto-run AIE MA crossover, print results |
| `vd100-bare-metal-ma-aie-interactive` | UART-commanded AIE MA crossover |

Each repo builds on the previous. All share the same platform and system project.
