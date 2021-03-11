## Описание
Мини-файловая система

## Сборка и запуск
Проект собирается с помощью cmake

- build.sh: сборка проекта
- run.sh: сборка + запуск проекта

## Структура проекта

- `main.c`: точка входа в проект. Описана логика взаимодействия с пользователем.
- `headers/`: директоряи с хедерами
- `src/util.c`: реализовы общий функции для работы io.
- `src/api.c`: реализация функций для взаимодействия с файловой системой
- `src/fs.c`: реализация внутреннего устройства мини файловой системы

## Доступные команды

- `ls`: выводит содержимое **текущей** директории. Не принимает никаких аргументов.
- `mkdir` dirname: создает в текущей директории директорию с названием dirname. Строка dirname должна состоять из 
неболее чем **27** символов латинского алфавита (для других сценариев корректная работа не гарантируется).
- `cd` dirname: перемещение из текущей директории в директорию с названием dirname.
- `touch` filename: создание файла с названием filename в текущей директории.
- `echo`: запись данных в файл. Для использования необходимо ввести команду и затем либо в новой строке либо через пробел 
написать желаемые данные для добавления в конец файла. После ввода данных на новой строке написать название файла.
- `echoline`: команда, аналогичная команде `echo`, но добавляет символ перевода строки к записанным данным.
- `cat` filename: выводит содержимое файла filename, **лежащего в текущей директории**.
- `rm` filename: удаляет файл filename из текущей деректории. В для удаления директорий используется рекурсивная стратегия.
- `pull` src dest: подтягивает файл src из настоящей файловой системы в файл с именем dest в текущей директории. Файл dest не должен 
существовать до вызова этой команды.
- `push` src dest: заливает файл src из текущей директории в настоящую файловую систему в файл, который определяется путем dest.
