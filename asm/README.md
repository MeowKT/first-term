
Все действия в инструкциях совершаются из папки с данным файлом

Файл с умножением назовите `mul.asm`, а вычитание `sub.asm`. Если хотите собрать код без него, то закомментируйте в `CMakeLists.txt` строчки, связанные с ними

Инструкция по сборке:
```
mkdir build
cd build
cmake ..
make
```
Инструкция по тестированиию:
```
cd tests
./test_256.sh
```