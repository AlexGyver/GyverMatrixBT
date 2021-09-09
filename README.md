![PROJECT_PHOTO](https://github.com/AlexGyver/GyverMatrixBT/blob/master/proj_img.jpg)
# Bluetooth адресная матрица на Arduino
* [Описание проекта](#chapter-0)
* [Папки проекта](#chapter-1)
* [Схемы подключения](#chapter-2)
* [Материалы и компоненты](#chapter-3)
* [Как скачать и прошить](#chapter-4)
* [FAQ](#chapter-5)
* [Полезная информация](#chapter-6)
[![AlexGyver YouTube](http://alexgyver.ru/git_banner.jpg)](https://www.youtube.com/channel/UCgtAOyEQdAyjvm9ATCi_Aig?sub_confirmation=1)

<a id="chapter-0"></a>
## Описание проекта
Матрица на адресных светодиодах с управлением по Bluetooth  
Гайд по матрицам: https://alexgyver.ru/matrix_guide/  
Страница проекта на сайте: https://alexgyver.ru/gyvermatrixbt/  
Описания к новыем версиям прошивок и приложений смотри на странице проекта    
#### Особенности:
 - Управление со смартфона по Bluetooth
#### Режимы:
 - Рисование
 - Загрузка картинок
 - Бегущая строка  
#### Эффекты:
 - "Дыхание" яркости
 - Смена цвета
 - Снегопад
 - Блуждающий кубик
 - Радуга
 - Огонь
 - The Matrix
 - Летающие частицы
 - Линии  
#### Игры:
 - Змейка
 - Tетриc
 - Лабиринт  
#### Возможности:
- Автоподключение к матрице при запуске
- Настройки яркости и скорости отображения
- Использование акселерометра в играх

<a id="chapter-1"></a>
## Папки
**ВНИМАНИЕ! Если это твой первый опыт работы с Arduino, читай [инструкцию](#chapter-4)**
- **libraries** - библиотеки проекта. Заменить имеющиеся версии (в этом проекте внешних библиотек нет)
- **firmware** - прошивки для Arduino
- **schemes** - схемы подключения компонентов
- **Android** - файлы с приложениями, примерами для Android и Thunkable
- **3Dprint** - файлы для печати и исходники

<a id="chapter-2"></a>
## Схемы
![SCHEME](https://github.com/AlexGyver/GyverMatrixBT/blob/master/schemes/scheme.jpg)

<a id="chapter-3"></a>
## Вам скорее всего пригодится
* [Всё для пайки (паяльники и примочки)](http://alexgyver.ru/all-for-soldering/)
* [Недорогие инструменты](http://alexgyver.ru/my_instruments/)
* [Все существующие модули и сенсоры Arduino](http://alexgyver.ru/arduino_shop/)
* [Электронные компоненты](http://alexgyver.ru/electronics/)
* [Аккумуляторы и зарядные модули](http://alexgyver.ru/18650/)

<a id="chapter-4"></a>
## Как скачать и прошить
* [Первые шаги с Arduino](http://alexgyver.ru/arduino-first/) - ультра подробная статья по началу работы с Ардуино, ознакомиться первым делом!
* Скачать архив с проектом
> На главной странице проекта (где ты читаешь этот текст) вверху справа зелёная кнопка **Clone or download**, вот её жми, там будет **Download ZIP**
* Установить библиотеки в  
`C:\Program Files (x86)\Arduino\libraries\` (Windows x64)  
`C:\Program Files\Arduino\libraries\` (Windows x86)
* **Подключить внешнее питание 5 Вольт**
* Подключить Ардуино к компьютеру
* Запустить файл прошивки (который имеет расширение .ino)
* Настроить IDE (COM порт, модель Arduino, как в статье выше)
* Настроить что нужно по проекту
* Нажать загрузить
* Скачать GyverMatrixBT с Play Market
* Пользоваться  

## Настройки в коде
    USE_BUTTONS 0       // использовать физические кнопки управления (0 нет, 1 да)
    BUTT_UP 3           // кнопка вверх
    BUTT_DOWN 5         // кнопка вниз
    BUTT_LEFT 2         // кнопка влево
    BUTT_RIGHT 4        // кнопка вправо

    LED_PIN 6           // пин ленты
    BRIGHTNESS 60       // стандартная маскимальная яркость (0-255)

    WIDTH 16            // ширина матрицы
    HEIGHT 16           // высота матрицы

    MATRIX_TYPE 0       // тип матрицы: 0 - зигзаг, 1 - последовательная
    CONNECTION_ANGLE 0  // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
    STRIP_DIRECTION 0   // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз

    SCORE_SIZE 0        // размер букв счёта в игре. 0 - маленький (для 8х8), 1 - большой
    FONT_TYPE 1			// (0 / 1) два вида маленького шрифта

    GLOBAL_COLOR_1 CRGB::Green    // основной цвет №1 для игр
    GLOBAL_COLOR_2 CRGB::Orange   // основной цвет №2 для игр
	
<a id="chapter-5"></a>
## FAQ
### Основные вопросы
В: Как скачать с этого грёбаного сайта?  
О: На главной странице проекта (где ты читаешь этот текст) вверху справа зелёная кнопка **Clone or download**, вот её жми, там будет **Download ZIP**

В: Скачался какой то файл .zip, куда его теперь?  
О: Это архив. Можно открыть стандартными средствами Windows, но думаю у всех на компьютере установлен WinRAR, архив нужно правой кнопкой и извлечь.

В: Я совсем новичок! Что мне делать с Ардуиной, где взять все программы?  
О: Читай и смотри видос http://alexgyver.ru/arduino-first/

В: Вылетает ошибка загрузки / компиляции!
О: Читай тут: https://alexgyver.ru/arduino-first/#step-5

В: Сколько стоит?  
О: Ничего не продаю.

### Вопросы по этому проекту

<a id="chapter-6"></a>
## Полезная информация
* [Мой сайт](http://alexgyver.ru/)
* [Основной YouTube канал](https://www.youtube.com/channel/UCgtAOyEQdAyjvm9ATCi_Aig?sub_confirmation=1)
* [YouTube канал про Arduino](https://www.youtube.com/channel/UC4axiS76D784-ofoTdo5zOA?sub_confirmation=1)
* [Мои видеоуроки по пайке](https://www.youtube.com/playlist?list=PLOT_HeyBraBuMIwfSYu7kCKXxQGsUKcqR)
* [Мои видеоуроки по Arduino](http://alexgyver.ru/arduino_lessons/)
