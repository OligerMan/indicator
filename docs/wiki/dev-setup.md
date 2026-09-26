# Окружение разработки

## Прошивка светильника (Windows)
- PlatformIO Core 6.1 установлен в `~/.platformio/penv`; `pio` не в PATH — вызывать
  `~/.platformio/penv/Scripts/pio.exe` или через расширение PlatformIO для VS Code.
- Платформа `espressif32@^6.5`, фреймворк Arduino (скачивается при первой сборке).
- Тесты ядра: `pio test -e native` — нужен gcc в PATH (есть: `C:\Users\<user>\gcc\bin`).
- Драйвер USB-UART DevKitC: CP2102 или CH340 (в зависимости от ревизии).

## Утилиты для ПК (`tools/`)
- Python 3.10+.
- `pip install numpy pillow` — для `stream_frames.py` (картинки/анимации).

## Симуляция и обучение ходока
- **WSL2 + Ubuntu 22.04/24.04**: JAX с CUDA работает только под Linux.
  - Драйвер NVIDIA для Windows с поддержкой WSL (обычный свежий драйвер).
  - Внутри WSL: `python -m venv`, `pip install -U "jax[cuda12]" mujoco mujoco-mjx playground`
    (актуальную команду сверить с README MuJoCo Playground).
  - Проверка: `python -c "import jax; print(jax.devices())"` → должен быть GPU.
- Ноутбук RTX 3080 Mobile 16 ГБ — основной для обучения; 3070 Ti 8 ГБ — хватит при меньшем
  числе параллельных сред.
- Визуализация: `mujoco.viewer` из WSL через WSLg. На Windows 10 (сборка 19044+) WSLg
  работает с WSL из Microsoft Store; если нет — рендерить видео в файл или смотреть модель
  в Windows-Python с `mujoco` (без JAX).
- CAD → модель: Onshape + `onshape-to-robot`.
