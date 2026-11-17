для теста работы вытащите данные файлы в корень репозитория и напишите в терминале следущие команды:


g++ -std=c++17 -Wall -Wextra -O2 main.cpp bmp.cpp watermark.cpp -o watermark_app


.\watermark_app.exe original.bmp mark.bmp result.bmp