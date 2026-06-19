# Arduino_Nano_Pozar
Sterownik Arduino do efektu pożaru w makiecie. Obsługuje dwa paski LED WS2812B, wyjścia tranzystorowe 5 V/12 V, silnik PWM i moduł kogutów. Tryb POŻAR działa przy stanie HIGH na przycisku, a OFF MODE przy stanie LOW. Kod zawiera debounce i konfigurację ACTIVE_LOW.


# Dokumentacja kodu — sterownik efektu pożaru z LED WS2812B, kogutami, silnikiem PWM i wyjściami tranzystorowymi

## 1. Opis działania programu

Program steruje modułem efektów specjalnych wykorzystującym:

* dwa paski LED WS2812B,
* wyjścia tranzystorowe 5 V i 12 V,
* wyjście PWM do sterowania silnikiem,
* sygnał sterujący do modułu „kogutów”,
* przycisk podłączony do pinu D2.

Układ pracuje w dwóch trybach:

* **tryb POŻAR** — aktywny, gdy na wejściu przycisku jest stan `HIGH`,
* **tryb OFF MODE** — aktywny, gdy na wejściu przycisku jest stan `LOW`.

Przycisk jest skonfigurowany jako `INPUT_PULLUP`, więc przy typowym podłączeniu przycisku między pin D2 a GND:

* puszczony przycisk daje stan `HIGH`,
* wciśnięty przycisk daje stan `LOW`.

Oznacza to, że przycisk działa bez przełączania typu toggle. Program na bieżąco sprawdza jego stan i ustawia odpowiedni tryb pracy.

---

## 2. Wymagane biblioteki

Program korzysta z dwóch bibliotek:

```cpp
#include <FastLED.h>
#include <Servo.h>
```

### FastLED

Biblioteka służy do obsługi pasków LED WS2812B.

W kodzie wyłączono przerwania dla FastLED:

```cpp
#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_INTERRUPT_RETRY_COUNT 0
```

Ma to poprawić stabilność działania pasków LED WS2812B, szczególnie przy generowaniu precyzyjnego sygnału czasowego.

### Servo

Biblioteka `Servo.h` jest używana do generowania impulsów sterujących modułem „kogutów”. W tym projekcie nie steruje klasycznym serwem, tylko wysyła impulsy o określonej długości.

---

## 3. Mapa pinów

| Funkcja     | Pin Arduino | Opis                                      |
| ----------- | ----------: | ----------------------------------------- |
| Przycisk    |          D2 | Wejście z `INPUT_PULLUP`, zwierane do GND |
| Pasek LED 1 |         D12 | Główny pasek WS2812B, 28 LED              |
| Pasek LED 2 |         D10 | Drugi pasek WS2812B, 6 LED                |
| Silnik PWM  |          D6 | Sterowanie silnikiem przez PWM            |
| OUT_5V      |          D8 | Wyjście tranzystorowe 5 V                 |
| OUT12_1     |          D5 | Wyjście tranzystorowe 12 V                |
| OUT12_2     |          D7 | Wyjście tranzystorowe 12 V                |
| OUT12_3     |    A0 / D14 | Wyjście tranzystorowe 12 V                |
| OUT12_4     |    A1 / D15 | Wyjście tranzystorowe 12 V                |
| Koguty      |         D11 | Sygnał sterujący modułem kogutów          |

---

## 4. Konfiguracja LED

Pierwszy pasek LED:

```cpp
#define LED_PIN      12
#define NUM_LEDS     28
#define LED_TYPE     WS2812B
#define COLOR_ORDER  GRB
#define BRIGHTNESS   120
```

Drugi pasek LED:

```cpp
#define LED2_PIN     10
#define NUM_LEDS2    6
```

Jasność całego efektu ustawiona jest na wartość `120` w skali 0–255.

---

## 5. Konfiguracja wyjść tranzystorowych

Wyjścia są zdefiniowane następująco:

```cpp
#define OUT_5V   8
#define OUT12_1  5
#define OUT12_2  7
#define OUT12_3  14
#define OUT12_4  15
```

Program posiada ustawienie `ACTIVE_LOW`:

```cpp
#define ACTIVE_LOW 0
```

Znaczenie:

* `ACTIVE_LOW 0` — wyjście włącza się stanem `HIGH`,
* `ACTIVE_LOW 1` — wyjście włącza się stanem `LOW`.

Jeśli po podłączeniu układu wyjścia działają odwrotnie, należy zmienić:

```cpp
#define ACTIVE_LOW 0
```

na:

```cpp
#define ACTIVE_LOW 1
```

---

## 6. Tryby pracy programu

### Tryb 0 — POŻAR

Tryb pożaru jest aktywny, gdy przycisk ma stan `HIGH`.

W tym trybie:

* włączane są wyjścia `OUT_5V` i `OUT12_1`,
* wyłączane są wyjścia `OUT12_2`, `OUT12_3`, `OUT12_4`,
* silnik PWM pracuje z pełną mocą,
* paski LED pokazują animację pożaru,
* moduł kogutów zostaje ustawiony w tryb `FIRE_MODE`.

Dla ustawienia:

```cpp
#define ACTIVE_LOW 0
```

stany wyjść w trybie POŻAR są następujące:

```cpp
OUT_5V   = HIGH
OUT12_1  = HIGH
OUT12_2  = LOW
OUT12_3  = LOW
OUT12_4  = LOW
```

Silnik:

```cpp
analogWrite(MOTOR_PWM_PIN, 255);
```

czyli pracuje na 100% mocy PWM.

---

### Tryb 1 — OFF MODE

Tryb OFF jest aktywny, gdy przycisk ma stan `LOW`.

W tym trybie:

* wyłączane są `OUT_5V` i `OUT12_1`,
* włączane są `OUT12_2`, `OUT12_3`, `OUT12_4`,
* silnik zostaje zatrzymany,
* paski LED zostają wygaszone,
* moduł kogutów zostaje ustawiony w tryb `OFF_MODE`.

Dla ustawienia:

```cpp
#define ACTIVE_LOW 0
```

stany wyjść w trybie OFF MODE są następujące:

```cpp
OUT_5V   = LOW
OUT12_1  = LOW
OUT12_2  = HIGH
OUT12_3  = HIGH
OUT12_4  = HIGH
```

Silnik:

```cpp
analogWrite(MOTOR_PWM_PIN, 0);
```

czyli jest wyłączony.

---

## 7. Obsługa przycisku

Przycisk jest podłączony do pinu D2 i skonfigurowany jako:

```cpp
pinMode(BTN_PIN, INPUT_PULLUP);
```

Program używa prostego filtrowania drgań styków, czyli debounce:

```cpp
#define DEBOUNCE_MS 30
```

Oznacza to, że zmiana stanu przycisku zostaje uznana dopiero wtedy, gdy utrzyma się stabilnie przez 30 ms.

Logika działania:

```cpp
if (stableBtn == HIGH) {
  mode = 0;
} else {
  mode = 1;
}
```

Czyli:

* `HIGH` — tryb POŻAR,
* `LOW` — tryb OFF MODE.

---

## 8. Sterowanie kogutami

Moduł kogutów jest sterowany przez pin D11:

```cpp
#define KOGUT_PIN 11
```

Do sterowania używana jest biblioteka `Servo`, ale w praktyce program wysyła impulsy o określonej długości.

Ustawienia impulsów:

```cpp
const uint16_t PULSE_LOW   = 1000;
const uint16_t PULSE_HIGH  = 2200;
const uint16_t PERIOD_MS   = 100;
```

Jeden „klik” składa się z:

1. impulsu `2200 us`,
2. powrotu do impulsu bazowego `1000 us`.

Funkcja wykonująca jeden klik:

```cpp
void clickOnce()
```

Po każdym kliknięciu program zakłada, że moduł kogutów przechodzi do następnego trybu.

Zakres trybów kogutów:

```cpp
const uint8_t FIRST_MODE = 1;
const uint8_t LAST_MODE  = 7;
```

Tryb pożaru:

```cpp
const uint8_t FIRE_MODE = 1;
```

Tryb wyłączenia:

```cpp
const uint8_t OFF_MODE = 7;
```

Program zapamiętuje aktualny tryb kogutów w zmiennej:

```cpp
uint8_t currentMode
```

Dzięki temu nie wysyła kliknięć bez potrzeby. Kliknięcia są wykonywane tylko wtedy, gdy trzeba przejść do innego trybu.

---

## 9. Animacja LED

Animacja LED działa tylko w trybie POŻAR.

Program używa dwóch kolorów:

```cpp
const CRGB COLOR_RED    = CRGB(255, 0, 0);
const CRGB COLOR_ORANGE = CRGB(255, 80, 0);
```

Efekt polega na płynnym mieszaniu czerwonego i pomarańczowego koloru.

Szybkość animacji określają:

```cpp
#define FPS        60
#define FIXED_BPM  24
```

Pierwszy pasek LED ma 28 diod podzielonych na 4 grupy po 7 diod:

```cpp
const uint8_t GROUPS[][7] = {
  {  0,  1,  2,  3, 27, 26, 25 },
  {  4,  5,  6,  7, 24, 23, 22 },
  {  8,  9, 10, 11, 21, 20, 19 },
  { 12, 13, 14, 15, 18, 17, 16 }
};
```

Grupy świecą naprzemiennie, dzięki czemu efekt pożaru jest bardziej dynamiczny.

Drugi pasek LED świeci jednolitym kolorem wynikającym z aktualnego przejścia między czerwonym i pomarańczowym.

---

## 10. Funkcja `setup()`

Funkcja `setup()` wykonuje konfigurację początkową programu.

Wykonywane są kolejno:

1. krótka pauza po starcie układu,
2. ustawienie pinu przycisku jako `INPUT_PULLUP`,
3. ustawienie wyjść tranzystorowych,
4. ustawienie pinu PWM silnika,
5. inicjalizacja pasków LED,
6. ustawienie jasności LED,
7. inicjalizacja sygnału kogutów,
8. odczyt początkowego stanu przycisku,
9. ustawienie trybu startowego zgodnie ze stanem przycisku.

Jeżeli przy starcie przycisk ma stan `HIGH`, program od razu przechodzi w tryb POŻAR.

Jeżeli przy starcie przycisk ma stan `LOW`, program przechodzi w tryb OFF MODE.

---

## 11. Funkcja `loop()`

Główna pętla programu wykonuje się cały czas.

W każdej iteracji:

1. odczytywany jest stan przycisku,
2. wykonywany jest debounce,
3. na podstawie stabilnego stanu przycisku ustawiany jest tryb pracy,
4. przy zmianie trybu ustawiane są koguty,
5. wykonywany jest kod odpowiedni dla trybu POŻAR lub OFF MODE.

Program nie działa już jako przełącznik toggle. Nie zapamiętuje kolejnych kliknięć przycisku. Tryb zależy bezpośrednio od aktualnego stanu wejścia.

---

## 12. Najważniejsze zmienne

| Zmienna           | Znaczenie                                           |
| ----------------- | --------------------------------------------------- |
| `mode`            | Aktualny tryb programu: `0` POŻAR, `1` OFF MODE     |
| `prevMode`        | Poprzedni tryb programu, używany do wykrycia zmiany |
| `stableBtn`       | Stabilny, przefiltrowany stan przycisku             |
| `lastBtnReading`  | Ostatni odczytany stan przycisku                    |
| `lastBtnChangeMs` | Czas ostatniej zmiany stanu przycisku               |
| `currentMode`     | Aktualny zapamiętany tryb kogutów                   |
| `leds[]`          | Tablica diod pierwszego paska LED                   |
| `leds2[]`         | Tablica diod drugiego paska LED                     |

---

## 13. Uwagi montażowe

Przycisk powinien być podłączony między pin D2 a GND.

Ponieważ używany jest `INPUT_PULLUP`, nie trzeba dodawać zewnętrznego rezystora podciągającego.

Przy typowym podłączeniu:

* puszczony przycisk = `HIGH` = tryb POŻAR,
* wciśnięty przycisk = `LOW` = tryb OFF MODE.

Jeżeli działanie ma być odwrotne, należy zamienić logikę w tym fragmencie:

```cpp
if (stableBtn == HIGH) {
  mode = 0;
} else {
  mode = 1;
}
```

na:

```cpp
if (stableBtn == HIGH) {
  mode = 1;
} else {
  mode = 0;
}
```

---

## 14. Możliwe problemy i diagnoza

### Efekt działa odwrotnie

Jeżeli wyjścia tranzystorowe włączają się odwrotnie niż powinny, należy zmienić:

```cpp
#define ACTIVE_LOW 0
```

na:

```cpp
#define ACTIVE_LOW 1
```

albo odwrotnie.

### Koguty ustawiają zły tryb

Program zakłada, że po starcie moduł kogutów znajduje się w trybie OFF, czyli w trybie numer 7.

Jeżeli moduł po włączeniu zasilania startuje w innym trybie, zmienna:

```cpp
currentMode = LAST_MODE;
```

może wymagać zmiany na rzeczywisty tryb startowy modułu.

### LED-y migają niestabilnie

Należy sprawdzić:

* czy pasek LED ma wspólną masę z Arduino,
* czy zasilacz ma odpowiednią wydajność prądową,
* czy sygnał danych do LED nie jest prowadzony zbyt długim przewodem,
* czy pierwszy LED otrzymuje poprawny sygnał na wejście DIN.

### Przycisk nie działa

Należy sprawdzić:

* czy jeden styk przycisku jest podłączony do D2,
* czy drugi styk przycisku jest podłączony do GND,
* czy w kodzie jest ustawione `pinMode(BTN_PIN, INPUT_PULLUP);`.

---

## 15. Podsumowanie

Program realizuje prosty sterownik dwóch stanów pracy:

* stan wysoki na przycisku uruchamia pełny efekt pożaru,
* stan niski na przycisku przełącza układ w tryb OFF.

Kod obsługuje jednocześnie animację LED, wyjścia tranzystorowe, silnik PWM oraz moduł kogutów. Zmiana trybu jest wykonywana tylko wtedy, gdy rzeczywiście zmieni się stabilny stan wejścia przycisku, dzięki czemu koguty nie są niepotrzebnie przełączane w każdej pętli programu.
