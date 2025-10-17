<h1 align="center">📺 Bilibili Weather TV — ESP8266 + WeChat Mini Program</h1>
<p align="center">
  <b>ESP8266 × WeChat Mini Program × I²C OLED × 3D-printed Enclosure</b><br>
  <i>A cross-disciplinary build that blends embedded firmware, PCB design, and industrial design.</i>
</p>

<p align="center">
  <img alt="platform" src="https://img.shields.io/badge/platform-ESP8266-blue">
  <img alt="display"  src="https://img.shields.io/badge/display-OLED%200.96%22-purple">
  <img alt="license"  src="https://img.shields.io/badge/license-MIT-informational">
</p>

---

## Overview (EN)

This project is a desktop “Bilibili TV” powered by **ESP8266**.  
It connects to Wi-Fi, synchronizes time via **NTP**, fetches **weather** and **Bilibili stats**, and supports **on-device message display** sent from a **WeChat Mini Program**.  
The hardware integrates USB/Li-ion power management and a 0.96″ I²C OLED; the enclosure is modeled in SolidWorks and 3D-printed.

### Key Features
- 🌤️ Weather via HTTP/JSON; configurable city and units  
- ⏰ Precise time using NTP with reconnect and fallback  
- 📊 Bilibili stats (e.g., views/followers) display rotation  
- 💬 Message board via UDP packets from the mini program  
- 🔋 Power solution: USB + Li-ion with charging & 3.3 V regulation  
- 🧱 SolidWorks enclosure; STEP export for easy modification

---

## Gallery (EN)

<div align="center">
  <table>
    <tr>
      <td align="center">
        <img src="assets/render.png" alt="Enclosure render" width="430"><br>
        <sub>Enclosure render (SolidWorks)</sub>
      </td>
      <td align="center">
        <img src="assets/schematic.png" alt="PCB schematic" width="430"><br>
        <sub>Main schematic (power / ESP8266 / I²C)</sub>
      </td>
    </tr>
    <tr>
      <td align="center">
        <img src="assets/pcb_real.jpg" alt="PCB real photo" width="430"><br>
        <sub>Assembled PCB (real photo)</sub>
      </td>
      <td align="center">
        <img src="assets/miniapp_ui.png" alt="WeChat mini program UI" width="430"><br>
        <sub>WeChat Mini Program — send message / set device IP</sub>
      </td>
    </tr>
  </table>
</div>

---

## Repository Layout (EN)
