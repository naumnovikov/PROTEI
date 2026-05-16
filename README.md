# PROTEI

## Краткое описание (цель)
**PROTEI** — консольное приложение на C++20, созданное в рамках обучения в IT-школе НТЦ ПРОТЕЙ.

## Основные возможности
- Загрузка и валидация конфигурации из JSON-файла (IP, порт, IMEI, IMSI, координаты, протокол, список узлов)
- Команды управления:
  - `Active`   — активация устройства
  - `Move`     — изменение координат
  - `Protocol` — переключение протокола (JSON/BINARY)
  - `Exit`     — завершение работы
- Логирование (spdlog)
- Юнит-тесты (Google Test) для основных компонентов

## Зависимости и версии
Все зависимости загружаются автоматически через CMake `FetchContent`.

| Библиотека      | Версия  | Назначение                       |
|-----------------|---------|----------------------------------|
| nlohmann/json   | 3.11.3  | Парсинг JSON-конфигураций        |
| spdlog          | 1.13.0  | Быстрое логирование |
| Google Test     | 1.14.0  | Фреймворк модульного тестирования |

**Требования к системе:**
- CMake >= 3.16
- Компилятор с поддержкой C++20 (GCC ≥ 12)
- Доступ в интернет при первой сборке

## Процесс сборки

### Основная программа
```bash
git clone https://github.com/naumnovikov/PROTEI.git
cd PROTEI
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./PROTEI
```
NB: Пока нет Pull Request, клонируется проект через develop:
```bash
git clone -b develop https://github.com/naumnovikov/PROTEI.git
```
### Сборка и запуск тестов
```bash
cmake --build . --target protei_tests
 ./TESTS/protei_tests
```
