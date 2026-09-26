# Документация

## Общее
- [vision.md](vision.md) — идея, референс, цели и не-цели
- [roadmap.md](roadmap.md) — этапы обоих треков и текущий статус
- [references.md](references.md) — ссылки: библиотеки, проекты-референсы, даташиты
- [glossary.md](glossary.md) — термины (HUB75, DOF, sim2real, …)
- [decisions/](decisions/) — архитектурные решения (ADR): что решили и почему

## Светильник (`lamp/`)
| Документ | О чём |
|---|---|
| [lamp/design.md](lamp/design.md) | дизайн-документ: требования, архитектура, риски |
| [lamp/hardware.md](lamp/hardware.md) | электрика: пины, питание, схема соединений |
| [lamp/mechanics.md](lamp/mechanics.md) | рамка, крепления, рассеиватель, печать |
| [lamp/firmware.md](lamp/firmware.md) | устройство прошивки, сцены, диаграмма состояний |
| [lamp/protocol.md](lamp/protocol.md) | текстовые команды, HTTP API, UDP-поток кадров |
| [lamp/bringup.md](lamp/bringup.md) | первый запуск по шагам, чек-лист |
| [lamp/bom.md](lamp/bom.md) | список покупок |
| [lamp/measurements.md](lamp/measurements.md) | журнал замеров (масса, ток, частота) |

## Ходок (`walker/`)
| Документ | О чём |
|---|---|
| [walker/design.md](walker/design.md) | дизайн-документ: стадии, требования, бюджет массы |
| [walker/kinematics.md](walker/kinematics.md) | почему 2DOF-пятизвенник, потом 3DOF; геометрия ноги |
| [walker/mechanics.md](walker/mechanics.md) | углепластиковые трубки, печатные узлы, стопы |
| [walker/actuators-power.md](walker/actuators-power.md) | STS3215, шина, аккумулятор 3S, преобразователи |
| [walker/control-rl.md](walker/control-rl.md) | управление, симуляция, обучение, sim2real |
| [walker/integration.md](walker/integration.md) | как светильник становится частью ходока |
| [walker/bom.md](walker/bom.md) | список покупок по стадиям |

## Вики (справочные знания)
- [wiki/hub75.md](wiki/hub75.md) — как устроены HUB75-панели и что с ними бывает
- [wiki/esp32-hub75-memory.md](wiki/esp32-hub75-memory.md) — расчёт памяти и частоты обновления
- [wiki/power-li-ion.md](wiki/power-li-ion.md) — Li-ion, BMS, преобразователи, безопасность
- [wiki/feetech-bus-servos.md](wiki/feetech-bus-servos.md) — шинные серво Feetech STS
- [wiki/rl-locomotion.md](wiki/rl-locomotion.md) — RL для шагающих: инструменты и подводные камни
- [wiki/printing.md](wiki/printing.md) — печать на Bambu P1S: материалы, допуски, вставки
- [wiki/dev-setup.md](wiki/dev-setup.md) — окружение: PlatformIO, Python, WSL2 для JAX

## Журнал
- [journal/](journal/) — дневник работ: что сделано, что сломалось, выводы
