# Индикатор v2

> — А хотите, я его стукну? Он станет фиолетовый в крапинку!
> — «Тайна третьей планеты», 1981

Индикатор настроения из мультфильма, вторая версия. Первая была светильником из
светодиодной ленты с рассеивателем, которым управляют постукиваниями (`legacy/indicator-v1`).
Вторая — два полноцветных экрана HUB75 128×64 с обеих сторон, Wi-Fi, а потом и ноги.

## Два трека
| Трек | Папка | Статус | Суть |
|---|---|---|---|
| Светильник | [`lamp/`](lamp/) | в работе | ESP32 + 2 панели P2.5, стуки/наклон, веб-управление, поток кадров |
| Ходок | [`walker/`](walker/) | проектирование | четвероногая платформа: 2DOF-пятизвенник → 3DOF, RL-походка |

Потом светильник становится «головой и телом» ходока — см. [docs/walker/integration.md](docs/walker/integration.md).

## С чего начать
- Идея и цели — [docs/vision.md](docs/vision.md)
- План — [docs/roadmap.md](docs/roadmap.md)
- Вся документация — [docs/README.md](docs/README.md)
- Первый запуск панелей — [docs/lamp/bringup.md](docs/lamp/bringup.md)

## Быстрый старт прошивки
```sh
cd lamp/firmware
pio test -e native                 # тесты логики на ПК
pio run -e panel_test -t upload    # проверка панелей
pio run -e lamp -t upload          # основная прошивка
pio device monitor                 # команды: help
```
Wi-Fi: скопировать `include/secrets.example.h` в `include/secrets.h`. Без него светильник
поднимает точку доступа `indicator-setup` (пароль `indicator`), страница — http://192.168.4.1.
