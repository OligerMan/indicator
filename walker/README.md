# Ходок

Четвероногая платформа для светильника. Стадия 1 — 2DOF-пятизвенник (8 × STS3215),
стадия 2 — 3DOF (12 × STS3215). Походка — RL в MuJoCo Playground, политика на Pi Zero 2 W.

| Папка | Что | Этап |
|---|---|---|
| [`sim/`](sim/) | модели MJCF, среды и обучение | B0, B3, B6 |
| [`cad/`](cad/) | ноги, узлы бедра, шасси | B1, B2, B5 |
| [`firmware/`](firmware/) | стенд одной ноги, утилиты серво | B1 |
| [`brain/`](brain/) | программа на Pi: шина серво, IMU, запуск политики, телеметрия | B2, B4 |

Документация: [дизайн](../docs/walker/design.md) · [кинематика](../docs/walker/kinematics.md) ·
[механика](../docs/walker/mechanics.md) · [приводы и питание](../docs/walker/actuators-power.md) ·
[управление и RL](../docs/walker/control-rl.md) · [объединение](../docs/walker/integration.md) ·
[покупки](../docs/walker/bom.md)
