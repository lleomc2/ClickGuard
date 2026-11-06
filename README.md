# ClickGuard

A lightweight Geode mod for Geometry Dash 2.2081 on Android.

- Optional guard against extremely fast duplicate press inputs.
- Enable or disable it in Geode mod settings.
- Configurable guard window from 0 to 150 ms (default: 45 ms).
- Release events are always passed through.
- Filtering is tracked separately for each player and button.

## Build

GitHub Actions builds an Android 64-bit package on pushes and manual runs. Open the Actions tab and download the ClickGuard-Android64 artifact from a successful run.

## Note

A short guard window can also filter intentional rapid taps. If inputs feel delayed or missed, lower the value or set it to 0 to disable filtering.
