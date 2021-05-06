#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define INP_SIZE 50000
#define SOLO_SIZE 11
#define DOUBLE_SIZE 16
#define KEYWORDS_COUNT 18
#define KEYWORD_LENGTH 200
#define STATES_STACK_SIZE 1000
#define SUPER_DOUBLE_SIZE 3

typedef enum {
	FOR,
	IF,
	FOR_SINGLE,
	IF_SINGLE,
	DECLARATION,
	INIT,
	FUNC_INIT,
	STRUCT,
	TYPEDEF,
	TYPEDEF_STRUCT,
	INCLUDING,
	FOR_BODY,
	DEFINE,
	ELSE,
	CASE_WITH_BRACES,
	CASE,
	NOTHING
} STATE;

typedef struct keyword {
	char value[KEYWORD_LENGTH];
	STATE state;
	int size;
} KEYWORD;

void pushKeyword(KEYWORD *keywords, char *value, STATE state) {
	strcpy(keywords[(*keywords).size].value, value);
	keywords[(*keywords).size].state = state;
	++((*keywords).size);
}

typedef struct stateStack {
	STATE states[STATES_STACK_SIZE];
	int curSize;
} STATE_STACK;

void statePush(STATE_STACK *stateStack, STATE *currentState, STATE value) {
	*currentState = value;
	stateStack->states[stateStack->curSize++] = value;
}

STATE statePop(STATE_STACK *stateStack) {
	int size = stateStack->curSize;
	if ((*stateStack).curSize < 0)
		printf("Error in statePop\n");

	if (size > 1)
		(stateStack->curSize)--;
	else {
		printf("Popa");
	}
	return stateStack->states[stateStack->curSize - 1];
}

// Печать табуляций
void printTabs(int tabCount, FILE *file) {
	for (int i = 0; i < tabCount; ++i) {
		fputs("\t", file);
	}
}


/* Если мы совершили \n, то нужно проверить, идёт ли дальше } или нет. Это нужно, чтобы избежать непредвиденных
 * табуляций или \n
 */
void checkCloseBracket(const char *input, int *i, int *tabs, FILE *file, STATE *currentState, STATE_STACK *states) {
	// Начинаем со следующего символа
	++(*i);

	int endOfLine = 0;
	// Пропускаем все пробелы, табуляции и \n, так как они не нужны
	while (input[*i] == ' ' || input[*i] == '\t' || input[*i] == '\n') {
		++(*i);
		if (input[*i + 1] == '\n' && endOfLine != 2 && *currentState != TYPEDEF_STRUCT && *currentState != FUNC_INIT &&
		    *currentState != STRUCT) {
			fputc('\n', file);
			++endOfLine;
		}
	}

	// Если после пропусков мы стоим на }, то уменьшаем кол-во табуляций, печатаем их, выводим }, переводим строку и
	// опять проверяем на }, так как мы поставили \n
	if (input[*i] == '}') {
		if (*currentState == CASE)
			(*tabs)--;
		if (states->curSize != 1)
			*currentState = statePop(states);
		(*tabs)--;
		printTabs(*tabs, file);
		fputs("}\n", file);
		while (*currentState == FOR_SINGLE || *currentState == IF_SINGLE) {
			--(*tabs);
			*currentState = statePop(states);
		}

		checkCloseBracket(input, i, tabs, file, currentState, states);
	} else {
		if (*currentState != FOR)
			printTabs(*tabs, file);
		--(*i);
	}
}

// Проверка, специальный ли знак
bool isSpecial(char symbol) {
	return symbol == '=' || symbol == '+' || symbol == '-' || symbol == '*' || symbol == '/' ||
	       symbol == '%' || symbol == '<' || symbol == '>' || symbol == '!' || symbol == '&' ||
	       symbol == '|';
}

void processSpecial(FILE *output_file, const char *input, const char *special_solo, const char special_double[][2],
                    const char super_special_double[][2], const char *buf, int *i, bool *isOperator) {
	// Проверим, может ли наша переменная бьыть двойным особым (++, --) знаком
	for (int j = 0; j < SUPER_DOUBLE_SIZE; j++) {
		if (!strncmp(buf, super_special_double[j], 2)) {
			fputs(buf, output_file);
			(*isOperator) = true;
			// Увеличиваем i, потому что input[i + 1] = buf[1], а buf[1] обрабатывается этой функцией
			(*i)++;
			break;
		}
	}

	// Проверим, может ли наша переменная быть двойным специальным знаком
	if (!(*isOperator))

		for (int j = 0; j < DOUBLE_SIZE; j++) {
			if (!strncmp(buf, special_double[j], 2)) {
				// Тут проверяем, есть ли слева от знака пробел и справа. Если нет, то ставим
				if (input[(*i) - 1] != ' ')
					fputc(' ', output_file);
				fputs(buf, output_file);
				if (input[(*i) + 2] != ' ')
					fputc(' ', output_file);
				(*isOperator) = true;
				// Увеличиваем i, потому что input[i + 1] = buf[1], а buf[1] обрабатывается этой функцией
				(*i)++;
				break;
			}
		}

	// Если это не двойной знак, то одинарный
	if (!(*isOperator)) {
		for (int j = 0; j < SOLO_SIZE; j++) {
			if (buf[0] == special_solo[j]) {
				if (input[(*i) - 1] != ' ')
					fputc(' ', output_file);
				fputc(special_solo[j], output_file);
				if (input[(*i) + 1] != ' ')
					fputc(' ', output_file);

				(*isOperator) = true;
				break;
			}
		}
	}
}

void
checkKeyword(char *lex, KEYWORD *keywords, STATE_STACK *stateStack, STATE *currentState) {
	for (int i = 0; i < KEYWORDS_COUNT; ++i) {
		if (!strcmp(keywords[i].value, lex)) {
			// Если у нас typedef struct / enum, то это определение нового типа данных в формате typedef
			if (*currentState == TYPEDEF && keywords[i].state == STRUCT) {
				statePop(stateStack);
				statePush(stateStack, currentState, TYPEDEF_STRUCT);
			} else {
				statePush(stateStack, currentState, keywords[i].state);
			}
		}
	}
}

void clearLex(char *lex, int *pointer) {
	for (int i = 0; i < KEYWORD_LENGTH; ++i) {
		lex[i] = '\0';
	}
	*pointer = 0;
}

bool processCase(const char *input, int i, int *tabCount, STATE *currentState, STATE_STACK *states) {
	char lex[KEYWORD_LENGTH] = {0};

	while (input[i] == ' ' || input[i] == '\n' || input[i] == '\t') {
		++i;
	}

	if (input[i] == '{' && *currentState != CASE_WITH_BRACES) {
		/*fputs("{\n", output_file);
		(*tabCount)++;
		printTabs(*tabCount, output_file);*/

		(*tabCount)--;
		statePop(states);
		statePush(states, currentState, CASE_WITH_BRACES);

		return false;
	}

	if (input[i] == '}' && *currentState == CASE) {
		(*tabCount)--;
		*currentState = statePop(states);
	}

	while (isalnum(input[i])) {
		sprintf(&lex[strlen(lex)], "%c", input[i++]);
	}

	if (!strcmp(lex, "case") || !strcmp(lex, "default")) {
		(*tabCount)--;
		*currentState = statePop(states);
	}
	return true;
}


int main() {
	FILE *input_file = fopen("input.c", "rt");
	FILE *output_file = fopen("output.c", "wt");
	/*sprintf(&a[strlen(a)], "%s", "abc");
	sprintf(&a[strlen(a)], "%s", "qwerty");*/

	char input[INP_SIZE] = {0};
	char output[INP_SIZE] = {0};
	char special_solo[SOLO_SIZE] = {'+', '-', '*', '/', '%', '=', '%', '<', '>', '&', '|'};
	char special_double[DOUBLE_SIZE][2] = {{"=="},
	                                       {"&&"},
	                                       {"||"},
	                                       {"<<"},
	                                       {">>"},
	                                       {"!="},
	                                       {">="},
	                                       {"<="},
	                                       {"+="},
	                                       {"-="},
	                                       {"*="},
	                                       {"/="},
	                                       {"%="},
	                                       {"&="},
	                                       {"|="}};
	//TODO добавить special_triple: <<=, >>=

	char super_special_double[SUPER_DOUBLE_SIZE][2] = {{"++"},
	                                                   {"--"},
	                                                   {"->"}};
	KEYWORD keywords[KEYWORDS_COUNT];
	pushKeyword(keywords, "for", FOR);
	pushKeyword(keywords, "while", FOR);
	pushKeyword(keywords, "if", IF);
	pushKeyword(keywords, "int", DECLARATION);
	pushKeyword(keywords, "double", DECLARATION);
	pushKeyword(keywords, "long", DECLARATION);
	pushKeyword(keywords, "float", DECLARATION);
	pushKeyword(keywords, "short", DECLARATION);
	pushKeyword(keywords, "char", DECLARATION);
	pushKeyword(keywords, "void", DECLARATION);
	pushKeyword(keywords, "struct", STRUCT);
	pushKeyword(keywords, "enum", STRUCT);
	pushKeyword(keywords, "typedef", TYPEDEF);
	pushKeyword(keywords, "include", INCLUDING);
	pushKeyword(keywords, "switch", IF);
	pushKeyword(keywords, "else", ELSE);
	pushKeyword(keywords, "define", DEFINE);
	pushKeyword(keywords, "do", TYPEDEF_STRUCT);

	STATE_STACK states;
	states.curSize = 0;

	// keywords - стек ключевых слов Си
	// states - стек состояний

	char buf[3] = {0};  // Буфер для специальных знаков. Третий символ нужен \0 для корректной работы strcmp
	int text_size = 0;  // Размер текста из файла

	// Чистаем из файла в массив
	while (!feof(input_file)) {
		input[text_size] = (char) fgetc(input_file);
		text_size++;
	}

	// Количество табуляций. Поддерживаем на протяжении всей программы
	int tabCount = 0;

	// Текущая лексема
	char lex[KEYWORD_LENGTH] = {0};
	int lexSize = 0;

	// Поддержка правильной скобочной последовательности
	int bracketSequence = 0;

	// Состояние по умолчанию
	STATE currentState = NOTHING;
	statePush(&states, &currentState, NOTHING);

	// Был ли в строке знак равно? Нужно для определения инициализации
	bool wasEqual = false;

	// Определение вложенности циклов
	int loopDepth = 0;
	int maxLoopDepth = 0;

	// Обработка текста
	for (int i = 0; i < text_size - 1; i++) {
		// Берём два последовательных символа в буфер
		buf[0] = input[i];
		buf[1] = input[i + 1];

		// Лексема - это последовательность букв и цифр + нижнее подчёркивание
		if (isalnum(input[i]) || input[i] == '_') {
			lex[lexSize++] = input[i];
		} else {
			// ОБРАБОТКА ЛЕКСЕМЫ

			/*
			 * Очень мелкое исключение. Так случилось, что мы можем написать вот так:
			 * int
			 * a;
			 * В таком случае \n проигнорируется (мы их сами печатаем, когда надо) и int сольётся с 'a'
			 * wasInit помечает, была ли объявлена переменная на момент проверки лексемы. Если она не была объявлена, а
			 * потом объявилась, то мы проверяем, идёт ли после типа данных пробел. Если нет, то печатаем
			 */

			//TODO TEST
			if (!strcmp(lex, "char")) {
				printf("asd");
			}
			//

			bool wasInit = false;
			if (currentState == DECLARATION)
				wasInit = true; //TODO name of variable

			checkKeyword(lex, keywords, &states, &currentState);

			// TODO (char) -> не объявление
			int j = i + 1;
			while (input[j] == ' ' || input[j] == '\n' || input[j] == '\t') {
				++j;
			}
			if (input[j] == ')' && !wasInit && currentState == DECLARATION)
				currentState = statePop(&states);

			if (!wasInit && currentState == DECLARATION && wasEqual)
				currentState = statePop(&states);

			if (!wasInit && currentState == DECLARATION && input[i] != ' ') {
				fputc(' ', output_file);
				while (input[i + 1] == '\n' || input[i + 1] == ' ' || input[i + 1] == '\t')
					++i;
				continue;
			}
			clearLex(lex, &lexSize);
		}

		// Проверка else
		if (currentState == ELSE) {
			while (input[i] == ' ' || input[i] == '\n' || input[i] == '\t') {
				i++;
			}
			if (input[i] == '{') {
				statePop(&states);
				statePush(&states, &currentState, FOR_BODY);
				fputs(" {\n", output_file);
				++i;
			} else {
				statePop(&states);
				statePush(&states, &currentState, IF_SINGLE);
				fputc('\n', output_file);

			}
			tabCount++;
			printTabs(tabCount, output_file);
			--i;
			continue;
		}

		if (currentState == DEFINE) {
			fputc(input[i], output_file);
			i++;
			while (input[i] != '\n') {
				fputc(input[i], output_file);
				++i;
			}
			fputc('\n', output_file);
			currentState = statePop(&states);
			continue;
		}

		// Смотрим на случаи особых знаков, после которых нам надо ставить пробел или \n
		switch (input[i]) {
			case '(':
				++bracketSequence;
				// Если это for, while или if, то мы ставим пробел между словом и '(', если его нет
				if ((currentState == FOR || currentState == IF) && input[i - 1] != ' ')
					fputc(' ', output_file);
				putc('(', output_file);
				while (input[i + 1] == ' ')
					++i;

				break;
			case ')':
				--bracketSequence;
				fputc(')', output_file);
				// Если это конец определения for, while или if (то есть то что в круглых скобках), то мы сбрасываем состояние
				if (bracketSequence == 0 && (currentState == FOR || currentState == IF)) {
					// Опускаем все пробелы, переносы строки и табуляции...
					while (input[i + 1] == ' ' || input[i + 1] == '\n' || input[i + 1] == '\t') {
						++i;
					}
					/*
					 * ... и смотрим, нашли ли мы '{'. Если НЕ нашли, то это одиночный for / if, тогда ставим \n и табы
					 * вручную, меняем состояние на SINGLE. Но если мы нашли '{', то это for / if с {},  тогда ставим
					 * пробел между ) и {, сбрасываем состояние for / if
					 */
					if (input[i + 1] != '{') {
						if (input[i + 1] == ';') {
							fputs(";\n", output_file);
							printTabs(tabCount, output_file);
							++i;
							statePop(&states);
							break;
						}
						if (currentState == IF) {
							currentState = statePop(&states);
							statePush(&states, &currentState, IF_SINGLE);
						} else {
							currentState = statePop(&states);
							statePush(&states, &currentState, FOR_SINGLE);
						}
						fputc('\n', output_file);
						++tabCount;
						printTabs(tabCount, output_file);
					} else {
						fputc(' ', output_file);
						currentState = statePop(&states);
						statePush(&states, &currentState, FOR_BODY);
						//  TODO if убивается?
					}
				}

				break;
			case ':':
				/*
				 * Case для switch. Не доделано. Но суть в том, что если мы находим {, то case обрамляется {}, тогда
				 * ставим пробел между : и {, но если { не нашли, то вручную ставим \n и табы
				 */
				/*while (input[i + 1] == ' ' || input[i + 1] == '\t' || input[i + 1] == '\n') {
					++i;
				}
				if (input[i + 1] == '{') {
					fputs(": {\n", output_file);
					printTabs(tabCount, output_file);
				} else {
					fputs(":\n", output_file);
					printTabs(tabCount, output_file);
				}*/
				currentState = CASE;
				statePush(&states, &currentState, CASE);

				fputc(':', output_file);
				++tabCount;
				if (processCase(input, i + 1, &tabCount, &currentState, &states)) {
					fputc('\n', output_file);
					printTabs(tabCount, output_file);
				}

				break;
			case '{':
				/*
				 * Так как структуры не имеют в своей с определяющей части круглых скобок, приходится отдельно
				 * рассматривать постановку пробела перед {
				 */
				if ((currentState == STRUCT || currentState == TYPEDEF_STRUCT) && input[i - 1] != ' ') {
					fputc(' ', output_file);
				}

				/*
				 * Шаблон: _data_type name() {} присущ инициализации функции. Это значит, что мы сейчас мы встретили
				 * объявление (_data_type), а так же НЕ встретили знак '=' (то есть это НЕ переменная). По сути,
				 * Наше состояние DECLARATION вызвано объявлением функции ИЛИ аргументов в функции. Тогда изменяем
				 * состояние с DECLARATION на FUNC_INIT и заодно ставим пробел между ) и {, а также удаляем все
				 * состояния DECLARATION до сего момента
				 */
				if (currentState == DECLARATION && wasEqual == false) {
					while (currentState == DECLARATION) {
						currentState = statePop(&states);
					}
					statePush(&states, &currentState, FUNC_INIT);
					if (input[i - 1] != ' ')
						fputc(' ', output_file);
				}

				/*
				 * Если это инициализация переменной, то { используется в char array[] = {0}. Поэтому просто печатаем {
				 * Если это не инициализация переменной, то всё что угодно остальное после чего мы ставим \n и табы и
				 * ищем }, чтобы не ошибиться в последующих табуляциях
				 */

				if (currentState == INIT) {
					fputs("{", output_file);
				} else {
					fputs("{\n", output_file);
					tabCount++;
					checkCloseBracket(input, &i, &tabCount, output_file, &currentState, &states);
				}

				while (input[i + 1] == ' ' || input[i + 1] == '\n' || input[i + 1] == '\t') {
					++i;
				}
				if (input[i + 1] == '{' && currentState != INIT) {
					statePush(&states, &currentState, NOTHING);
				}

				break;
			case '}':
				/*
				 * Редкий случай, когда нам надо обработать } внезапно, вне функции checkCloseBracket.
				 * Если это скобка от структуры, то ставим \n, табы, а потом уже }. Сюда мы можем попасть в случае,
				 * если у нас enum, в котором последнее значение может не заканчиваться на ;, например:
				 * enum {
				 *    one,
				 *    two
				 * };
				 */
				if (currentState == TYPEDEF_STRUCT || currentState == STRUCT) {
					tabCount--;
					fputc('\n', output_file);
					printTabs(tabCount, output_file);
					currentState = statePop(&states);
				}
				fputc('}', output_file);

				break;
			case ';':
				if (currentState == INIT || currentState == DECLARATION)
					currentState = statePop(&states);

				if (currentState == CASE || currentState == CASE_WITH_BRACES)
					processCase(input, i + 1, &tabCount, &currentState, &states);

				/*
				 * Если вы дошли до сюда, то значит шарите в стратегиях
				 */

				// Сбрасываем пометку встреченного знака '=', так как ; сигнализирует о конце команды
				wasEqual = false;

				/*
				 * В зависимости от текущего состояния программы, мы обрабатываем ; по-разному.
				 * В for нам нужно ставить пробелы после ; (в круглых скобках).
				 * В инициализации / объявлении поменять состояние на "до".
				 * Во всех остальных ситуациях действуем по ситуации. Что же тут происходит...
				 */
				switch (currentState) {
					case FOR: {
						fputc(';', output_file);
						if (input[i + 1] != ' ')
							fputc(' ', output_file);

						break;
					}
					case DECLARATION:
					case INIT:
						// Сбрасываем состояние
						currentState = statePop(&states);

					default:
						fputc(';', output_file);

						// Если мы сбросили состояние DECLARATION или INIT, то возможно мы вернулись в скобки for
						if (currentState == FOR) {
							fputc(' ', output_file);
						} else {
							fputc('\n', output_file);
						}

						/*
						 * Так случилось, что CLion при форматировании убирает \n, если их больше двух. Поэтому мы тоже
						 * распечатаем 0-2 \n
						 */
						int endOfLine = 0;
						while (input[i + 1] == ' ' || input[i + 1] == '\t' || input[i + 1] == '\n' || input[i + 1] == '/') {
							//TODO!!!!
							++i;
							if (input[i] == '/' && input[i + 1] == '/') {
								printTabs(tabCount, output_file);
								fputs("//", output_file);
								i += 2;
								while (input[i] != '\n') {
									fputc(input[i], output_file);
									i++;
								}
								fputc('\n', output_file);
							}

							if (input[i] == '/' && input[i + 1] == '*') {
								fputs("/*", output_file);
								i += 2;
								while (strncmp(&input[i], "*/", 2 * sizeof(char)) != 0) {
									fputc(input[i], output_file);
									i++;
								}
								fputs("*/", output_file);
								i += 2;

								/*if (input[i] == '\n') {
									fputc('\n', output_file);
									printTabs(tabCount, output_file);
								}*/

							}

							if (input[i + 1] == '\n' && endOfLine != 2) {
								fputc('\n', output_file);
								++endOfLine;
							}
						}

						/*
						 * Чтобы случайно не перепутать, к какой табуляции относится следующая {, уберём все одиночные
						 * for / while / if, так как ; сигнализирует о конце оных
						 */
						while (currentState == FOR_SINGLE || currentState == IF_SINGLE) {
							--tabCount;
							currentState = statePop(&states);
						}

						/*
						 * Мы встретили } ? Значит, закончился какой-то блок кода. Нужно определить, что закончилось
						 * и как следует расставлять \n и пробелы
						 */
						if (input[i + 1] == '{')
							statePush(&states, &currentState, NOTHING);

						if (input[i + 1] == '}') {
							// Выведем } и табуляции
							tabCount--;
							printTabs(tabCount, output_file);
							fputs("}", output_file);
							i++;

							/*
							 * Если это структура, то ; ставится в конце. Но между } и ; могут быть объекты структуры
							 * или названия типа данных. Поэтому нам нужно понять, нужно ли ставить пробел или нет.
							 * В ином случае, мы просто ставим \n
							 */
							if (currentState != TYPEDEF_STRUCT && currentState != STRUCT)
								fputs("\n", output_file);
							else {
								while (input[i + 1] == ' ' || input[i + 1] == '\t' || input[i + 1] == '\n') {
									++i;
								}
								if (input[i + 1] != ';')
									fputc(' ', output_file);

							}

							/*
							 * Так как } означает какое-то состояние, то выгружаем его.
							 * Есть случай, когда { } ставятся чисто по рофлу, без какой-либо структуры.
							 * + ещё такое возможно в case, который не обрабатывается. В таком случае выгрузится NOTHING
							 */

							currentState = statePop(&states);
						}

						while (currentState == FOR_SINGLE || currentState == IF_SINGLE) {
							--tabCount;
							currentState = statePop(&states);
						}

						checkCloseBracket(input, &i, &tabCount, output_file, &currentState, &states);

						break;
				}

				break;


			case ',':
				fputs(", ", output_file);
				while (input[i + 1] == ' ' || input[i + 1] == '\t' || input[i + 1] == '\n')
					i++;
				break;
			case ' ': {
				int j = i;
				while (input[j] == ' ' || input[j] == '\t' || input[j] == '\n')
					j--;
				if (input[j] == '{')
					break;

				while (input[i + 1] == ' ' || input[i + 1] == '\t' || input[i + 1] == '\n')
					i++;
				if (input[i + 1] != ')')
					fputc(' ', output_file);
				break;
			}

			case '=':
				wasEqual = true;

				if (currentState == DECLARATION) {
					statePop(&states);
					statePush(&states, &currentState, INIT);
				}
			default: {
				if (!strncmp(buf, "//", 2 * sizeof(char))) {
					fputs("//", output_file);
					i += 2;
					while (input[i] != '\n') {
						fputc(input[i], output_file);
						i++;
					}
					fputc('\n', output_file);
					while (input[i + 1] == '\n' || input[i + 1] == ' ' || input[i + 1] == '\t')
						++i;
					if (input[i + 1] == '}') {
						printTabs(tabCount - 1, output_file);
					} else {
						printTabs(tabCount, output_file);
					}

					break;
				}

				//TODO превращение многострочных комментариев в однострочные
				if (!strncmp(buf, "/*", 2 * sizeof(char))) {
					fputs("/*", output_file);
					i += 2;
					while (strncmp(&input[i], "*/", 2 * sizeof(char)) != 0) {
						fputc(input[i], output_file);
						i++;
					}
					fputs("*/", output_file);
					i += 1;

					if (input[i + 1] == '\n') {
						fputc('\n', output_file);
						printTabs(tabCount, output_file);
					}
					break;
				}

				if (input[i] == '\"' || input[i] == '\'') {
					bool isDouble;
					if (input[i] == '\"')
						isDouble = true;
					else
						isDouble = false;
					fputc(input[i], output_file);
					++i;
					while (input[i] != '\"' && isDouble || input[i] != '\'' && !isDouble) {
						if (input[i] == '\\') {
							fprintf(output_file, "%c%c", input[i], input[i + 1]);
							i += 2;
						} else {
							fputc(input[i], output_file);
							++i;
						}
					}
					fputc(input[i], output_file);

					break;
				}

				if (input[i] == '\'') {
					++i;
					fputc('\'', output_file);
					while (input[i] != '\'') {
						if (input[i] == '\\') {
							fprintf(output_file, "%c%c", input[i], input[i + 1]);
							i += 2;
						} else {
							fputc(input[i], output_file);
							++i;
						}
					}
					fputc('\"', output_file);

					break;
				}
				// Переменная для пометки состояния буфера. True, если в буфере оператор
				bool isOperator = false;

				// Если текущий символ - оператор, то поступаем с ним соответствующе
				if (isSpecial(input[i]) && currentState != INCLUDING) {
					bool beginOfDefinition = false;

					if (input[i] == '*') {
						int j = i - 1;
						while (input[j] == ' ' || input[j] == '\n' || input[j] == '\t') {
							j--;
						}
						if (isSpecial(input[j]))
							beginOfDefinition = true;
					}

					if (input[i] == '*' &&
					    (beginOfDefinition == true || currentState == DECLARATION || input[i - 1] == '(' ||
					     (currentState == FUNC_INIT && wasEqual != true))) {
						while (input[i] == '*') {
							fputc('*', output_file);
							++i;
						}
						while (input[i] == ' ') {
							i++;
						}
						--i;
						break;
					}
					processSpecial(output_file, input, special_solo, special_double, super_special_double, buf, &i,
					               &isOperator);
				} else {
					if (input[i] == '>') {
						fputs(">\n", output_file);
						while (input[i + 1] == ' ' || input[i + 1] == '\t' || input[i + 1] == '\n') {
							++i;
						}
						currentState = statePop(&states);
					}
				}

				// Если не оператор, то выводим символ
				if (!isOperator && (input[i] != '\n' || currentState == INCLUDING)) {
					fputc(buf[0], output_file);
				}
			}
		}


	}

	fclose(output_file);
	fclose(input_file);

	return 0;
}