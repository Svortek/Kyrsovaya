#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <stdbool.h>
#include <wctype.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SIZE 20

typedef struct {
    wchar_t *russian;
    wchar_t *english;
} WordPair;

typedef struct {
    WordPair *standardWords;
    int numStandardWords;
    int capacityStandardWords;
    WordPair *addedWords;
    int numAddedWords;
    int capacityAddedWords;
} Dictionary;

void initializeDictionary(Dictionary *dict) {
    dict->numStandardWords = 0;
    dict->capacityStandardWords = INITIAL_SIZE;
    dict->standardWords = malloc(dict->capacityStandardWords * sizeof(WordPair));
    if (dict->standardWords == NULL) {
        wprintf(L"Ошибка выделения памяти.\n");
        exit(1);
    }

    dict->numAddedWords = 0;
    dict->capacityAddedWords = INITIAL_SIZE;
    dict->addedWords = malloc(dict->capacityAddedWords * sizeof(WordPair));
    if (dict->addedWords == NULL) {
        wprintf(L"Ошибка выделения памяти.\n");
        exit(1);
    }
}

// Функция для приведения первой буквы строки к верхнему регистру
void capitalize(wchar_t *str) {
    if (str && *str) {
        *str = towupper(*str);
    }
}

// Функция для приведения строки к нижнему регистру
void toLowerCase(wchar_t *str) {
    for (; *str; ++str) *str = towlower(*str);
}

// Функция для перевода слова
bool translateWord(wchar_t word[], Dictionary *dict, wchar_t *translation) {
    toLowerCase(word); // Приводим входное слово к нижнему регистру

    for (int i = 0; i < dict->numStandardWords; i++) {
        wchar_t russianLower[50];
        wcscpy(russianLower, dict->standardWords[i].russian);
        toLowerCase(russianLower);

        if (wcscmp(russianLower, word) == 0) {
            wcscpy(translation, dict->standardWords[i].english);
            return true;
        }
    }

    for (int i = 0; i < dict->numAddedWords; i++) {
        wchar_t russianLower[50];
        wcscpy(russianLower, dict->addedWords[i].russian);
        toLowerCase(russianLower);

        if (wcscmp(russianLower, word) == 0) {
            wcscpy(translation, dict->addedWords[i].english);
            return true;
        }
    }

    return false;
}

// Функция для перевода текста
void translateText(wchar_t text[], Dictionary *dict) {
    wchar_t *result = malloc(1024 * sizeof(wchar_t));
    result[0] = L'\0';

    wchar_t buffer[50];
    wchar_t *word = text;
    wchar_t *start = text;

    while (*word != L'\0') {
        if (iswspace(*word) || iswpunct(*word)) {
            wcsncpy(buffer, start, word - start);
            buffer[word - start] = L'\0';

            if (wcslen(buffer) > 0) {
                wchar_t translation[50];
                if (translateWord(buffer, dict, translation)) {
                    wcscat(result, translation);
                } else {
                    wcscat(result, buffer);
                }
            }

            wchar_t punct[2] = {*word, L'\0'};
            wcscat(result, punct);

            start = word + 1;
        }
        word++;
    }

    if (start < word) {
        wcsncpy(buffer, start, word - start);
        buffer[word - start] = L'\0';

        if (wcslen(buffer) > 0) {
            wchar_t translation[50];
            if (translateWord(buffer, dict, translation)) {
                wcscat(result, translation);
            } else {
                wcscat(result, buffer);
            }
        }
    }

    wprintf(L"Переведенный текст: %ls\n", result);
    free(result);
}

// Функция для добавления нового слова в словарь
void addWord(Dictionary *dict) {
    while (true) {
        if (dict->numAddedWords >= dict->capacityAddedWords) {
            dict->capacityAddedWords *= 2;
            dict->addedWords = realloc(dict->addedWords, dict->capacityAddedWords * sizeof(WordPair));
            if (dict->addedWords == NULL) {
                wprintf(L"Ошибка выделения памяти.\n");
                exit(1);
            }
        }

        wchar_t russian[50];
        wchar_t english[50];

        wprintf(L"Введите новое слово на русском (или 0 для выхода): ");
        fgetws(russian, 50, stdin);
        wchar_t *newline = wcschr(russian, L'\n');
        if (newline) *newline = L'\0';

        if (wcscmp(russian, L"0") == 0) break;

        toLowerCase(russian); // Приводим слово к нижнему регистру

        wprintf(L"Введите перевод на английском: ");
        fgetws(english, 50, stdin);
        newline = wcschr(english, L'\n');
        if (newline) *newline = L'\0';

        toLowerCase(english); // Приводим слово к нижнему регистру
        capitalize(russian); // Приводим первую букву к верхнему регистру
        capitalize(english); // Приводим первую букву к верхнему регистру

        dict->addedWords[dict->numAddedWords].russian = malloc((wcslen(russian) + 1) * sizeof(wchar_t));
        dict->addedWords[dict->numAddedWords].english = malloc((wcslen(english) + 1) * sizeof(wchar_t));

        wcscpy(dict->addedWords[dict->numAddedWords].russian, russian);
        wcscpy(dict->addedWords[dict->numAddedWords].english, english);

        dict->numAddedWords++;
        wprintf(L"Слово успешно добавлено в словарь.\n");
    }
}

// Функция для добавления нового текста в словарь
void addText(Dictionary *dict) {
    wchar_t text[500];
    wprintf(L"Введите текст на русском: ");
    fgetws(text, 500, stdin);
    wchar_t *newline = wcschr(text, L'\n');
    if (newline) *newline = L'\0';

    wchar_t buffer[50];
    wchar_t *word = text;
    wchar_t *start = text;

    while (*word != L'\0') {
        if (iswspace(*word) || iswpunct(*word)) {
            wcsncpy(buffer, start, word - start);
            buffer[word - start] = L'\0';

            if (wcslen(buffer) > 0) {
                wchar_t translation[50];
                if (!translateWord(buffer, dict, translation)) {
                    wprintf(L"Слово '%ls' не найдено в словаре.\n", buffer);
                    wprintf(L"Введите перевод слова на английском: ");
                    wchar_t newEnglish[50];
                    fgetws(newEnglish, 50, stdin);
                    wchar_t *newline = wcschr(newEnglish, L'\n');
                    if (newline) *newline = L'\0';

                    if (dict->numAddedWords >= dict->capacityAddedWords) {
                        dict->capacityAddedWords *= 2;
                        dict->addedWords = realloc(dict->addedWords, dict->capacityAddedWords * sizeof(WordPair));
                        if (dict->addedWords == NULL) {
                            wprintf(L"Ошибка выделения памяти.\n");
                            exit(1);
                        }
                    }

                    toLowerCase(buffer); // Приводим слово к нижнему регистру перед добавлением
                    toLowerCase(newEnglish); // Приводим перевод к нижнему регистру перед добавлением
                    capitalize(buffer); // Приводим первую букву к верхнему регистру
                    capitalize(newEnglish);

                    dict->addedWords[dict->numAddedWords].russian = malloc((wcslen(buffer) + 1) * sizeof(wchar_t));
                    dict->addedWords[dict->numAddedWords].english = malloc((wcslen(newEnglish) + 1) * sizeof(wchar_t));

                    wcscpy(dict->addedWords[dict->numAddedWords].russian, buffer);
                    wcscpy(dict->addedWords[dict->numAddedWords].english, newEnglish);
                    dict->numAddedWords++;
                    wprintf(L"Слово успешно добавлено в словарь.\n");
                }
            }

            start = word + 1;
        }
        word++;
    }

    if (start < word) {
        wcsncpy(buffer, start, word - start);
        buffer[word - start] = L'\0';

        if (wcslen(buffer) > 0) {
            wchar_t translation[50];
            if (!translateWord(buffer, dict, translation)) {
                wprintf(L"Слово '%ls' не найдено в словаре.\n", buffer);
                wprintf(L"Введите перевод слова на английском: ");
                wchar_t newEnglish[50];
                fgetws(newEnglish, 50, stdin);
                wchar_t *newline = wcschr(newEnglish, L'\n');
                if (newline) *newline = L'\0';

                if (dict->numAddedWords >= dict->capacityAddedWords) {
                    dict->capacityAddedWords *= 2;
                    dict->addedWords = realloc(dict->addedWords, dict->capacityAddedWords * sizeof(WordPair));
                    if (dict->addedWords == NULL) {
                        wprintf(L"Ошибка выделения памяти.\n");
                        exit(1);
                    }
                }

                toLowerCase(buffer); // Приводим слово к нижнему регистру перед добавлением
                toLowerCase(newEnglish); // Приводим перевод к нижнему регистру перед добавлением
                capitalize(buffer); // Приводим первую букву к верхнему регистру
                capitalize(newEnglish);

                dict->addedWords[dict->numAddedWords].russian = malloc((wcslen(buffer) + 1) * sizeof(wchar_t));
                dict->addedWords[dict->numAddedWords].english = malloc((wcslen(newEnglish) + 1) * sizeof(wchar_t));

                wcscpy(dict->addedWords[dict->numAddedWords].russian, buffer);
                wcscpy(dict->addedWords[dict->numAddedWords].english, newEnglish);
                dict->numAddedWords++;
                wprintf(L"Слово успешно добавлено в словарь.\n");
            }
        }
    }
}

// Функция для редактирования существующего слова
void editWord(Dictionary *dict) {
    wchar_t russian[50];
    wprintf(L"Введите слово на русском для редактирования: ");
    fgetws(russian, 50, stdin);
    wchar_t *newline = wcschr(russian, L'\n');
    if (newline) *newline = L'\0';

    toLowerCase(russian); // Приводим слово к нижнему регистру

    for (int i = 0; i < dict->numStandardWords; i++) {
        wchar_t russianLower[50];
        wcscpy(russianLower, dict->standardWords[i].russian);
        toLowerCase(russianLower);

        if (wcscmp(russianLower, russian) == 0) {
            wchar_t english[50];
            wprintf(L"Введите новый перевод на английском: ");
            fgetws(english, 50, stdin);
            newline = wcschr(english, L'\n');
            if (newline) *newline = L'\0';

            toLowerCase(english); // Приводим слово к нижнему регистру
            capitalize(english); // Приводим первую букву к верхнему регистру

            free(dict->standardWords[i].english);
            dict->standardWords[i].english = malloc((wcslen(english) + 1) * sizeof(wchar_t));
            wcscpy(dict->standardWords[i].english, english);
            wprintf(L"Слово успешно отредактировано.\n");
            return;
        }
    }

    for (int i = 0; i < dict->numAddedWords; i++) {
        wchar_t russianLower[50];
        wcscpy(russianLower, dict->addedWords[i].russian);
        toLowerCase(russianLower);

        if (wcscmp(russianLower, russian) == 0) {
            wchar_t english[50];
            wprintf(L"Введите новый перевод на английском: ");
            fgetws(english, 50, stdin);
            newline = wcschr(english, L'\n');
            if (newline) *newline = L'\0';

            toLowerCase(english); // Приводим слово к нижнему регистру
            capitalize(english); // Приводим первую букву к верхнему регистру

            free(dict->addedWords[i].english);
            dict->addedWords[i].english = malloc((wcslen(english) + 1) * sizeof(wchar_t));
            wcscpy(dict->addedWords[i].english, english);
            wprintf(L"Слово успешно отредактировано.\n");
            return;
        }
    }

    wprintf(L"Слово не найдено в словаре.\n");
}

// Функция для удаления слова из словаря
void deleteWord(Dictionary *dict) {
    wchar_t russian[50];
    wprintf(L"Введите слово на русском для удаления: ");
    fgetws(russian, 50, stdin);
    wchar_t *newline = wcschr(russian, L'\n');
    if (newline) *newline = L'\0';

    toLowerCase(russian); // Приводим слово к нижнему регистру

    for (int i = 0; i < dict->numStandardWords; i++) {
        wchar_t russianLower[50];
        wcscpy(russianLower, dict->standardWords[i].russian);
        toLowerCase(russianLower);

        if (wcscmp(russianLower, russian) == 0) {
            free(dict->standardWords[i].russian);
            free(dict->standardWords[i].english);
            for (int j = i; j < dict->numStandardWords - 1; j++) {
                dict->standardWords[j] = dict->standardWords[j + 1];
            }
            dict->numStandardWords--;
            wprintf(L"Слово успешно удалено из словаря.\n");
            return;
        }
    }

    for (int i = 0; i < dict->numAddedWords; i++) {
        wchar_t russianLower[50];
        wcscpy(russianLower, dict->addedWords[i].russian);
        toLowerCase(russianLower);

        if (wcscmp(russianLower, russian) == 0) {
            free(dict->addedWords[i].russian);
            free(dict->addedWords[i].english);
            for (int j = i; j < dict->numAddedWords - 1; j++) {
                dict->addedWords[j] = dict->addedWords[j + 1];
            }
            dict->numAddedWords--;
            wprintf(L"Слово успешно удалено из словаря.\n");
            return;
        }
    }

    wprintf(L"Слово не найдено в словаре.\n");
}

// Функция для отображения всего словаря
void displayDictionary(Dictionary *dict) {
    for (int i = 0; i < dict->numStandardWords; i++) {
        wprintf(L"%ls: %ls\n", dict->standardWords[i].russian, dict->standardWords[i].english);
    }

    for (int i = 0; i < dict->numAddedWords; i++) {
        wprintf(L"%ls: %ls\n", dict->addedWords[i].russian, dict->addedWords[i].english);
    }
}

// Функция для сохранения словаря в файл
void saveDictionary(Dictionary *dict) {
    wchar_t filename[100];
    wprintf(L"Введите имя файла для сохранения: ");
    fgetws(filename, 100, stdin);
    wchar_t *newline = wcschr(filename, L'\n');
    if (newline) *newline = L'\0';

    FILE *file = _wfopen(filename, L"w");
    if (file == NULL) {
        wprintf(L"Ошибка открытия файла.\n");
        return;
    }

    fwprintf(file, L"STANDARD_WORDS\n");
    for (int i = 0; i < dict->numStandardWords; i++) {
        fwprintf(file, L"%ls %ls\n", dict->standardWords[i].russian, dict->standardWords[i].english);
    }

    fwprintf(file, L"ADDED_WORDS\n");
    for (int i = 0; i < dict->numAddedWords; i++) {
        fwprintf(file, L"%ls %ls\n", dict->addedWords[i].russian, dict->addedWords[i].english);
    }

    fclose(file);
    wprintf(L"Словарь сохранен в файл %ls.\n", filename);
}

// Функция для загрузки словаря из файла
void loadDictionary(Dictionary *dict) {
    wchar_t filename[100];
    wprintf(L"Введите имя файла для загрузки: ");
    fgetws(filename, 100, stdin);
    wchar_t *newline = wcschr(filename, L'\n');
    if (newline) *newline = L'\0';

    FILE *file = _wfopen(filename, L"r");
    if (file == NULL) {
        wprintf(L"Ошибка открытия файла.\n");
        return;
    }

    // Освобождаем текущий словарь
    for (int i = 0; i < dict->numStandardWords; i++) {
        free(dict->standardWords[i].russian);
        free(dict->standardWords[i].english);
    }
    free(dict->standardWords);

    for (int i = 0; i < dict->numAddedWords; i++) {
        free(dict->addedWords[i].russian);
        free(dict->addedWords[i].english);
    }
    free(dict->addedWords);

    // Инициализируем новый словарь
    initializeDictionary(dict);

    wchar_t russian[50];
    wchar_t english[50];
    wchar_t buffer[100];
    bool readingStandardWords = true;

    while (fgetws(buffer, 100, file)) {
        if (wcscmp(buffer, L"STANDARD_WORDS\n") == 0) {
            readingStandardWords = true;
            continue;
        }
        if (wcscmp(buffer, L"ADDED_WORDS\n") == 0) {
            readingStandardWords = false;
            continue;
        }

        if (swscanf(buffer, L"%49ls %49ls", russian, english) == 2) {
            toLowerCase(russian); // Приводим слово к нижнему регистру перед добавлением
            toLowerCase(english); // Приводим перевод к нижнему регистру перед добавлением
            capitalize(russian); // Приводим первую букву к верхнему регистру
            capitalize(english); // Приводим первую букву к верхнему регистру

            if (readingStandardWords) {
                if (dict->numStandardWords >= dict->capacityStandardWords) {
                    dict->capacityStandardWords *= 2;
                    dict->standardWords = realloc(dict->standardWords, dict->capacityStandardWords * sizeof(WordPair));
                    if (dict->standardWords == NULL) {
                        wprintf(L"Ошибка выделения памяти.\n");
                        exit(1);
                    }
                }
                dict->standardWords[dict->numStandardWords].russian = malloc((wcslen(russian) + 1) * sizeof(wchar_t));
                dict->standardWords[dict->numStandardWords].english = malloc((wcslen(english) + 1) * sizeof(wchar_t));

                wcscpy(dict->standardWords[dict->numStandardWords].russian, russian);
                wcscpy(dict->standardWords[dict->numStandardWords].english, english);
                dict->numStandardWords++;
            } else {
                if (dict->numAddedWords >= dict->capacityAddedWords) {
                    dict->capacityAddedWords *= 2;
                    dict->addedWords = realloc(dict->addedWords, dict->capacityAddedWords * sizeof(WordPair));
                    if (dict->addedWords == NULL) {
                        wprintf(L"Ошибка выделения памяти.\n");
                        exit(1);
                    }
                }
                dict->addedWords[dict->numAddedWords].russian = malloc((wcslen(russian) + 1) * sizeof(wchar_t));
                dict->addedWords[dict->numAddedWords].english = malloc((wcslen(english) + 1) * sizeof(wchar_t));

                wcscpy(dict->addedWords[dict->numAddedWords].russian, russian);
                wcscpy(dict->addedWords[dict->numAddedWords].english, english);
                dict->numAddedWords++;
            }
        }
    }

    fclose(file);
    wprintf(L"Словарь загружен из файла %ls.\n", filename);
}

// Освобождение памяти
void freeDictionary(Dictionary *dict) {
    for (int i = 0; i < dict->numStandardWords; i++) {
        free(dict->standardWords[i].russian);
        free(dict->standardWords[i].english);
    }
    free(dict->standardWords);

    for (int i = 0; i < dict->numAddedWords; i++) {
        free(dict->addedWords[i].russian);
        free(dict->addedWords[i].english);
    }
    free(dict->addedWords);
}

int main() {
    setlocale(LC_ALL, ".866");
    setlocale(LC_CTYPE, "ru_RU.UTF-8");

    Dictionary dict;
    initializeDictionary(&dict);

    // Инициализация словаря
    wchar_t *initialWords[10][2] = {
        {L"Привет", L"Hello"},
        {L"Пока", L"Goodbye"},
        {L"Спасибо", L"Thank you"},
        {L"Пожалуйста", L"Please"},
        {L"Да", L"Yes"},
        {L"Нет", L"No"},
        {L"Извините", L"Sorry"},
        {L"Утро", L"Morning"},
        {L"Вечер", L"Evening"},
        {L"Ночь", L"Night"}
    };

    for (int i = 0; i < 10; i++) {
        dict.standardWords[dict.numStandardWords].russian = malloc((wcslen(initialWords[i][0]) + 1) * sizeof(wchar_t));
        dict.standardWords[dict.numStandardWords].english = malloc((wcslen(initialWords[i][1]) + 1) * sizeof(wchar_t));
        wcscpy(dict.standardWords[dict.numStandardWords].russian, initialWords[i][0]);
        wcscpy(dict.standardWords[dict.numStandardWords].english, initialWords[i][1]);
        dict.numStandardWords++;
    }

    int choice;
    while (true) {
        wprintf(L"Меню:\n");
        wprintf(L"1. Перевести слово\n");
        wprintf(L"2. Перевести текст\n");
        wprintf(L"3. Добавить новое слово\n");
        wprintf(L"4. Добавить новый текст для перевода\n");
        wprintf(L"5. Редактировать существующее слово\n");
        wprintf(L"6. Удалить слово\n");
        wprintf(L"7. Отобразить словарь\n");
        wprintf(L"8. Сохранить словарь в файл\n");
        wprintf(L"9. Загрузить словарь из файла\n");
        wprintf(L"0. Выход\n");
        wprintf(L"Выберите опцию: ");
        wscanf(L"%d", &choice);
        getchar(); // Удаление символа новой строки из входного буфера

        switch (choice) {
            case 1: {
                wchar_t input[50];
                while (true) {
                    wprintf(L"Введите слово на русском (для выхода введите '0'): ");
                    fgetws(input, 50, stdin);
                    wchar_t *newline = wcschr(input, L'\n');
                    if (newline) *newline = L'\0';

                    if (wcscmp(input, L"0") == 0) break;

                    wchar_t translation[50];
                    if (translateWord(input, &dict, translation)) {
                        wprintf(L"Перевод: %ls\n", translation);
                    } else {
                        wprintf(L"Слово не найдено в словаре.\n");
                    }
                }
                break;
            }
            case 2: {
                wchar_t text[500];
                wprintf(L"Введите текст на русском: ");
                fgetws(text, 500, stdin);
                wchar_t *newline = wcschr(text, L'\n');
                if (newline) *newline = L'\0';

                translateText(text, &dict);
                break;
            }
            case 3:
                addWord(&dict);
                break;
            case 4:
                addText(&dict);
                break;
            case 5:
                editWord(&dict);
                break;
            case 6:
                deleteWord(&dict);
                break;
            case 7:
                displayDictionary(&dict);
                break;
            case 8:
                saveDictionary(&dict);
                break;
            case 9:
                loadDictionary(&dict);
                break;
            case 0:
                freeDictionary(&dict);
                return 0;
            default:
                wprintf(L"Некорректный выбор. Пожалуйста, попробуйте снова.\n");
        }
    }
}
