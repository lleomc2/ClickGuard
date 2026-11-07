# ClickGuard

A tick-based input-buffer mod for Geometry Dash **2.2081 on Android 64-bit**.

## Features

- Queues both press and release events, then applies them after the selected number of physics ticks.
- Buffer choices: **2, 4, 8, 16 ticks**, or **OFF** for raw input.
- Wave inputs use a maximum of 8 ticks to keep the delay shorter.
- A small circular button appears in the level pause menu. Tap it to open the ClickGuard panel and cycle the buffer setting.
- The Geode mod settings page also has an enable switch and a tick-window setting.
- One physics tick is **1/240 second** (about 4.17 ms). At 60 FPS, 4 ticks are about one frame.

## Important behavior

This version uses a fixed tick queue. It delays press and release events by the selected amount; it does **not** run AfterFlow's hidden ghost-player simulation or predict safe landing/orb timings. Larger values add more input delay, so start with 2 ticks and set the buffer to OFF if it feels uncomfortable.

This is an input-assist mod. Check the rules of any level list or leaderboard before using timing assists there.

## Target

- Geometry Dash 2.2081
- Geode SDK 5.11.2
- Android64 only

## Build

Push to `main` or run the GitHub Actions workflow manually. Download the `ClickGuard-Android` artifact from a successful run; it contains the `.geode` package.
