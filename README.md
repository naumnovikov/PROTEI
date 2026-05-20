## Процесс сборки

### Основная программа
```bash
git clone -b task_03 https://github.com/naumnovikov/PROTEI.git
cd PROTEI
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```
NB: В нынешней логике работы App нужно сначала запускать сервер, а только потом App. В будущем это будет дорабатываться.
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
cmake -DBUILD_TESTS=ON ..
cmake --build .
./TESTS/protei_tests
```
