Репа: https://github.com/xammi/Curs_SMTP

## Исходные материалы
* [Пример простого неблокирующего сервера от IBM](https://www.ibm.com/support/knowledgecenter/en/ssw_i5_54/rzab6/poll.htm)
* [Документация по SMTP](https://tools.ietf.org/html/rfc5321)
* [Пример реализованного конечного автомата SMTP](https://github.com/ibillxia/xsmtp)
* [Порядок команд "на пальцах"](https://www.codeproject.com/Articles/20604/SMTP-Server)
* [Межпроцессное взаимодействие в System V](https://gist.github.com/sacko87/3327485)

## Реализовано
* все нужные команды (HELO, EHLO, MAIL FROM, RCPT TO, VRFY, DATA, QUIT, NOOP, RSET)
* неблокирующий ввод-вывод в одном потоке с вызовом poll (12 вариант)
* машина состояний smtp с необходимыми проверками и кодами ошибок
* складывание писем с SMTP-заголовками в Maildir
* конфигурация в отдельном файле + libconfig
* системные тесты на python
* проверены утечки памяти valgrind-ом
* логирование в отдельном процессе
* компилировать с помощью GNU Make, а не qmake
* написана расчетно-пояснительная записка

## Осталось сделать
* генерация документации с помощью doxygen
* проверять обратную зону DNS
* проверить с почтовым клиентом
