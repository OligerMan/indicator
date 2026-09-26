# Ссылки

## Референс
- «Тайна третьей планеты» (Союзмультфильм, 1981) — [вики Союзмультфильма](https://soyuzmultfilm.fandom.com/ru/wiki/%D0%A2%D0%B0%D0%B9%D0%BD%D0%B0_%D0%A2%D1%80%D0%B5%D1%82%D1%8C%D0%B5%D0%B9_%D0%9F%D0%BB%D0%B0%D0%BD%D0%B5%D1%82%D1%8B), [форум: «Индикатор на ножках»](https://mults.info/forum/viewtopic.php?t=482)
- v1 проекта — `legacy/indicator-v1/`

## Светильник
- [ESP32-HUB75-MatrixPanel-DMA](https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA) — драйвер панелей (используем v3.0.15)
- [esphome/esp-hub75](https://registry.platformio.org/libraries/esphome/esp-hub75) — альтернативный драйвер, если панели окажутся несовместимы
- [WLED MoonModules — HUB75](https://mm.kno.wled.ge/2D/HUB75/) — быстрый тест панелей готовой прошивкой
- [SmartMatrix wiki: HUB75 Panels](https://github.com/pixelmatix/SmartMatrix/wiki/HUB75-Panels) — типы панелей и драйверов
- [Adafruit: RGB LED Matrix Basics — Power](https://learn.adafruit.com/32x16-32x32-rgb-led-matrix/powering) — потребление панелей
- [hzeller/rpi-rgb-led-matrix](https://github.com/hzeller/rpi-rgb-led-matrix) — вывод на HUB75 с Raspberry Pi (рассмотрен, не выбран)
- [Adafruit MPU6050](https://github.com/adafruit/Adafruit_MPU6050) — драйвер IMU

## Ходок
- [Open Duck Mini](https://github.com/apirrone/Open_Duck_Mini) — главный референс: STS3215, Pi Zero 2 W, BNO055, MuJoCo Playground, ONNX на борту
- [Open Duck Mini Runtime](https://deepwiki.com/apirrone/Open_Duck_Mini_Runtime) — программа на борту
- [Hackaday: DIY BDX droid](https://hackaday.com/2025/04/05/disneys-bipedal-bdx-series-droid-gets-the-diy-treatment/)
- Stanford Doggo / Ghost Minitaur — пятизвенные ноги с моторами в корпусе (найти публикации: Kau et al., 2019 «Stanford Doggo»; Kenneally et al., 2016 «Minitaur»)
- [Petoi Bittle](https://www.petoi.com/) — маленький четвероногий с 8 DOF
- [Feetech STS3215 12 В](https://www.feetechrc.com/525603.html) — характеристики
- [Robo9: тест STS3215 — люфт, повторяемость, момент](https://robonine.com/testing-of-feetech-sts3215-servomotor-backlash-repeatability-and-torque/)
- [MuJoCo Playground](https://github.com/google-deepmind/mujoco_playground) — обучение на GPU (JAX)
- [onshape-to-robot](https://github.com/Rhoban/onshape-to-robot) — экспорт CAD в URDF/MJCF
- LeRobot (Hugging Face) — пример работы с шиной Feetech из Python
