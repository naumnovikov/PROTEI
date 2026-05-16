## Процесс сборки

### Основная программа
```bash
git clone -b task_03_part1 https://github.com/naumnovikov/PROTEI.git
cd PROTEI
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```
### Сборка и запуск основного приложения App
```bash
cmake --build .
./app
```
### Сборка и запуск основного приложения Server
```bash
cmake --build . --target server
 ./server
```
### Сборка и запуск тестов
```bash
cmake --build . --target protei_tests
 ./TESTS/protei_tests
```
