#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
//#include <stdbool.h>
//#include <inttypes.h>
#include "ili9341.h"
#include "ili9341gfx.h"

static FILE string_data = FDEV_SETUP_STREAM(ili9341_putchar_printf, NULL, _FDEV_SETUP_WRITE);

typedef struct
{
	char code[2];
	uint8_t quantity;
	uint8_t price;
}type_item;

void display_menu(void);
void display_menu1_f1(type_item *);
void display_menu1_f2(type_item *, uint8_t *);
void display_menu2(type_item *);
void display_menu3(type_item *);
void display_menu4(type_item *);
void display_selection_table(uint8_t *, uint8_t *, uint16_t, uint16_t, uint16_t, uint16_t *, void(*write)(uint16_t));
void write_price(uint16_t);
void write_quantity(uint16_t);
void payment(uint16_t);
void init_led(void);
uint16_t check_overflow(uint16_t, uint16_t, uint16_t);
void get_item(type_item *);
void write_item(type_item *);
void eeprom_write(uint16_t, uint8_t);
uint16_t eeprom_read(uint16_t);
void init_keypad(void);
int8_t scan_keypad(void);
void delay(uint16_t);

void init_item(type_item *);

/*
00 10000       05 12000         10 5000           15 8000
01 21000       06 5000          11 12000          16 5000
02 3000        07 8000          12 59000          17 3000
03 5000        08 12000         13 80000          18 8000
04 8000        09 22000         14 3000           19 8000
*/

/*int main(void)
{
	UBRRH = 0;
	UBRRL = 51;
	//Enable receiver and transmitter
	UCSRB = (1 << RXEN) | (1 << TXEN);
	//Set frame format: 8data, 2stop bit, no parity
	UCSRC = (1 << URSEL) | (3 << UCSZ0);

	//float a = 0.1;
	//float b = 0.2;
	//double x = 65530.67;
	//long int y = 0;
	int x = 100;
	for (;;)
	{
		while (!(UCSRA & (1 << UDRE)));
		//Put data into buffer, sends the data
		UDR = x;

		while (!(UCSRA & (1 << UDRE)));
		//Put data into buffer, sends the data
		UDR = '\r';
		delay(1000);
	}

	return 0;
}*/

int main(void)
{
	//for debugging
	//DDRD = 0xFF;
	//PORTD = 0x00;
	type_item item[20];
	//init_item(item);
	get_item(item);
	stdout = &string_data;
	uint8_t channel = 0;
	int8_t pressed = 0;
	uint16_t temp_x = 0;
	uint16_t temp_y = 0;

	ili9341_init();
	init_keypad();
	ili9341_setRotation(1);
	ili9341_clear(YELLOW);
	//printf("HELLO");
	
	for(;;)
	{
		write_item(item);
		init_led();
		//get_item(item);

		switch (channel)
		{
		case 0:
			display_menu();
			temp_x = ili9341_getx();
			temp_y = ili9341_gety();
			while ((pressed = scan_keypad()) != 15)
			{
				if (pressed >= 0 && pressed <= 9)
				{
					channel = check_overflow((uint16_t) channel, (uint16_t) pressed, 99);
				}
				else if (pressed == 10)
				{
					channel /= 10;
					ili9341_backspace(1, YELLOW);
				}
				ili9341_setcursor(temp_x, temp_y);
				printf("%hhu", channel);
			}

			if (channel < 1 || channel > 5)
			{
				ili9341_clear(YELLOW);
				printf("\nInput menu hanya 1 - 5");
				channel = 0;
			}
			break;

		case 1:
			display_menu1_f1(item);
			channel = 0;
			ili9341_clear(YELLOW);
			break;

		case 2 :
			display_menu2(item);
			channel = 0;
			ili9341_clear(YELLOW);
			break;

		case 3:
			display_menu3(item);
			channel = 0;
			ili9341_clear(YELLOW);
			break;

		case 4:
			display_menu4(item);
			channel = 0;
			ili9341_clear(YELLOW);
			break;

		case 5 :
			init_item(item);
			channel = 0;
			ili9341_clear(YELLOW);
			break;

		default:
			break;
		}
	}
	return 0;
}

void display_menu(void)
{
	ili9341_settextcolour(BLUE, YELLOW);
	ili9341_setcursor(65, 0);
	ili9341_settextsize(3);
	printf("PILIH MENU\n");
	ili9341_settextcolour(BLUE, YELLOW);
	ili9341_settextsize(2);
	printf("1. Barang dijual\n");
	printf("2. Barang dibeli\n");
	printf("3. Cek Stock dan harga\n");
	printf("4. Ganti Harga\n");
	printf("5. Default\n");
	printf("Masukkan Pilihan : ");
	return;
}

void display_menu1_f1(type_item *x)
{
	ili9341_clear(BLACK);
	uint8_t sell[20] = {};

	ili9341_settextcolour(WHITE, BLACK);
	ili9341_settextsize(2);
	ili9341_drawvline(100, 0, 202, BLUE);
	ili9341_drawvline(200, 0, 202, BLUE);

	ili9341_setcursor(25, 0);
	printf("KODE");

	ili9341_setcursor(120, 0);
	printf("STOCK\n");

	ili9341_setcursor(225, 0);
	printf("DIJUAL\n");

	ili9341_drawhline(0, ili9341_gety(), 330, BLUE);
	ili9341_drawhline(0, 202, 330, BLUE);
	ili9341_settextsize(1);
	ili9341_write('\n');
	uint16_t temp_y = ili9341_gety();

	for (register uint8_t i = 0; i < 20; i++)
	{
		ili9341_setcursor(5, i * 8 + 24);
		printf("%c%c", x[i].code[1], x[i].code[0]);
		ili9341_setcursor(105, i * 8 + 24);
		printf("%hhu", x[i].quantity);
		ili9341_setcursor(205, i * 8 + 24);
		printf("%hhu", sell[i]);
	}
	ili9341_settextsize(2);
	ili9341_setcursor(20, 210);
	printf("CONFIRM");
	ili9341_setcursor(230, 210);
	printf("BACK");

	uint8_t choice = 0;
	uint16_t threshold[20] = {};
	for (register uint8_t i = 0; i < 20; i++)
		threshold[i] = x[i].quantity;
	ili9341_setcursor(0, temp_y);
	display_selection_table(sell, &choice, WHITE, BLACK, BLUE, threshold, write_quantity);
	
	if (choice == 20) //BACK to menu
		display_menu1_f2(x, sell);

	return;
}

void display_menu1_f2(type_item *x, uint8_t *y)
{
	ili9341_clear(BLACK);
	ili9341_settextcolour(WHITE, BLACK);
	ili9341_settextsize(2);

	ili9341_setcursor(4, 2);
	printf("JUMLAH");
	ili9341_setcursor(87, 2);
	printf("KODE");
	ili9341_setcursor(155, 2);
	printf("HARGA");
	ili9341_setcursor(245, 2);
	printf("TOTAL\n");

	ili9341_drawhline(0, ili9341_gety() + 5, 320, BLUE);
	
	ili9341_setcursor(0, ili9341_gety() + 10);
	ili9341_settextsize(1);

	uint16_t total = 0;
	uint16_t temp_total = 0;
	//uint8_t temp_y = 0;
	for (register uint8_t i = 0; i < 20; i++)
	{
		if (y[i] != 0)
		{
			x[i].quantity -= y[i];
			ili9341_setcursor(5, ili9341_gety());
			printf("%hhu", y[i]);
			ili9341_setcursor(85, ili9341_gety());
			printf("%c%c", x[i].code[1], x[i].code[0]);
			ili9341_setcursor(145, ili9341_gety());
			printf("%hhu.000", x[i].price);
			ili9341_setcursor(235, ili9341_gety());
			temp_total = (y[i] * x[i].price);
			write_price(temp_total);
			total += temp_total;
		}
	}
	ili9341_drawvline(80, 0, ili9341_gety() + 2, BLUE);
	ili9341_drawvline(140, 0, ili9341_gety() + 2, BLUE);
	ili9341_drawhline(0, ili9341_gety() + 2, 320, BLUE);

	ili9341_setcursor(90, ili9341_gety() + 5);
	printf("TOTAL");
	ili9341_setcursor(235, ili9341_gety());
	write_price(total);

	ili9341_drawvline(230, 0, ili9341_gety() + 2, BLUE);
	ili9341_drawhline(0, ili9341_gety() + 2, 320, BLUE);

	ili9341_write('\n');
	if (total)
		payment(total);
	else
		printf("TIDAK ADA INPUT\n");

	ili9341_settextsize(2);
	ili9341_setcursor(130, 220);
	ili9341_settextcolour(WHITE, BLUE);
	printf("BACK");
	while (scan_keypad() != 15);
	return;
}

void display_menu2(type_item *x)
{
	ili9341_clear(CYAN);
	uint8_t buy[20] = {};

	ili9341_settextcolour(MAGENTA, CYAN);
	ili9341_settextsize(2);
	ili9341_drawvline(100, 0, 202, BLUE);
	ili9341_drawvline(200, 0, 202, BLUE);

	ili9341_setcursor(25, 0);
	printf("KODE");

	ili9341_setcursor(120, 0);
	printf("STOCK\n");

	ili9341_setcursor(225, 0);
	printf("DIBELI\n");

	ili9341_drawhline(0, ili9341_gety(), 330, BLUE);
	ili9341_drawhline(0, 202, 330, BLUE);
	ili9341_settextsize(1);
	ili9341_write('\n');
	uint16_t temp_y = ili9341_gety();

	for (register uint8_t i = 0; i < 20; i++)
	{
		ili9341_setcursor(5, i * 8 + temp_y);
		printf("%c%c", x[i].code[1], x[i].code[0]);
		ili9341_setcursor(105, i * 8 + temp_y);
		printf("%hhu", x[i].quantity);
		ili9341_setcursor(205, i * 8 + temp_y);
		printf("%hhu", buy[i]);
	}
	ili9341_settextsize(2);
	ili9341_setcursor(20, 210);
	printf("CONFIRM");
	ili9341_setcursor(230, 210);
	printf("BACK");

	uint8_t choice = 0;
	uint16_t threshold[20] = {};
	for (register uint8_t i = 0; i < 20; i++)
		threshold[i] = 250 - x[i].quantity;
	ili9341_setcursor(0, temp_y);
	display_selection_table(buy, &choice, MAGENTA, CYAN, NAVY, threshold, write_quantity);
	

	if (choice == 20)
	{
		for (register uint8_t i = 0; i < 20; i++)
		{
			if (buy[i] != 0)
			{
				x[i].quantity += buy[i];
			}
		}
	}

	return;
}

void display_menu3(type_item *x)
{
	ili9341_clear(RED);
	ili9341_settextcolour(GREEN, RED);
	ili9341_settextsize(2);
	ili9341_drawvline(80, 0, 202, BLUE);
	ili9341_drawvline(180, 0, 202, BLUE);
	
	ili9341_setcursor(15, 0);
	printf("KODE");

	ili9341_setcursor(100, 0);
	printf("STOCK\n");
	ili9341_setcursor(95, ili9341_gety());
	printf("(unit)");

	ili9341_setcursor(220, 0);
	printf("HARGA\n");
	ili9341_setcursor(190, ili9341_gety());
	printf("(per unit)\n");

	ili9341_drawhline(0, ili9341_gety(), 330, BLUE);
	ili9341_settextsize(1);
	ili9341_write('\n');
	for (register uint8_t i = 0; i < 20; i++)
	{
		//printf("%s %80hu %100hu\n", x[i].code, x[i].quantity, x[i].price);
		ili9341_setcursor(5, ili9341_gety());
		printf("%c%c", x[i].code[1], x[i].code[0]);
		ili9341_setcursor(85, ili9341_gety());
		printf("%hhu", x[i].quantity);
		ili9341_setcursor(185, ili9341_gety());
		write_price(x[i].price);
	}
	ili9341_drawhline(0, 202, 330, BLUE);
	
	ili9341_settextsize(2);
	ili9341_setcursor(130, 210);
	ili9341_settextcolour(GREEN, PURPLE);
	printf("BACK");
	while (scan_keypad() != 15);
	return;
}

void display_menu4(type_item *x)
{
	ili9341_clear(GREEN);
	uint8_t price[20] = {};

	ili9341_settextcolour(BLACK, GREEN);
	ili9341_settextsize(2);
	ili9341_drawvline(100, 0, 202, BLUE);
	ili9341_drawvline(200, 0, 202, BLUE);

	ili9341_setcursor(25, 0);
	printf("KODE");

	ili9341_setcursor(120, 0);
	printf("HARGA\n");
	ili9341_setcursor(120, ili9341_gety());
	printf("LAMA\n");

	ili9341_setcursor(225, 0);
	printf("HARGA\n");
	ili9341_setcursor(225, ili9341_gety());
	printf("BARU\n");

	ili9341_drawhline(0, ili9341_gety(), 330, BLUE);
	ili9341_drawhline(0, 202, 330, BLUE);
	ili9341_settextsize(1);
	ili9341_write('\n');
	uint16_t temp_y = ili9341_gety();

	for (register uint8_t i = 0; i < 20; i++)
	{
		ili9341_setcursor(5, i * 8 + temp_y);
		printf("%c%c", x[i].code[1], x[i].code[0]);
		ili9341_setcursor(105, i * 8 + temp_y);
		write_price(x[i].price);
		ili9341_setcursor(205, i * 8 + temp_y);
		write_price(price[i]);
	}
	ili9341_settextsize(2);
	ili9341_setcursor(20, 210);
	printf("CONFIRM");
	ili9341_setcursor(230, 210);
	printf("BACK");

	uint8_t choice = 0;
	uint16_t threshold[20] = {};
	for (register uint8_t i = 0; i < 20; i++)
		threshold[i] = 255;
	ili9341_setcursor(0, temp_y);
	display_selection_table(price, &choice, BLACK, GREEN, RED, threshold, write_price);

	if (choice == 20)
	{
		for (register uint8_t i = 0; i < 20; i++)
		{
			if (price[i] != 0)
			{
				x[i].price = price[i];
			}
		}
	}

	return;
}

void display_selection_table(uint8_t *data, uint8_t *choice, uint16_t text_c, uint16_t bg_c, uint16_t highlight_c, uint16_t *threshold, void (*write)(uint16_t))
{
	uint16_t temp_y = ili9341_gety();
	int8_t pressed = 0;
	uint8_t prev_choice = 0;
	do
	{
		//LEFT = 12 RIGHT = 14 UP = 11 DOWN = 13
		if (pressed != -1)
		{
			prev_choice = *choice;
			if (pressed == 11)
			{
				if (*choice == 21)
					*choice = 19;
				else if (*choice != 0)
					(*choice)--;
			}
			else if (pressed == 12)
			{
				if (*choice >= 3 && *choice <= 19)
					*choice -= 3;
				else if (*choice == 21)
					(*choice)--;
			}
			else if (pressed == 13)
			{
				*choice >= 20 ? *choice : (*choice)++;
			}
			else if (pressed == 14)
			{
				if (*choice >= 0 && *choice <= 16)
					*choice += 3;
				else if (*choice == 20)
					(*choice)++;
			}

			if (*choice == prev_choice)
				prev_choice = 100;
		}

		ili9341_settextcolour(text_c, bg_c);
		if (prev_choice >= 0 && prev_choice <= 19)
		{
			ili9341_settextsize(1);
			ili9341_setcursor(205, prev_choice * 8 + temp_y);
			write(data[prev_choice]);
		}
		else if (prev_choice == 20)
		{
			ili9341_settextsize(2);
			ili9341_setcursor(20, 210);
			printf("CONFIRM");
		}
		else if (prev_choice == 21)
		{
			ili9341_settextsize(2);
			ili9341_setcursor(230, 210);
			printf("BACK");
		}

		ili9341_settextcolour(text_c, highlight_c);
		if (*choice >= 0 && *choice <= 19)
		{
			ili9341_settextsize(1);
			ili9341_setcursor(205, *choice * 8 + temp_y);
			write(data[*choice]);
			if (data[*choice] == 0)
				ili9341_setcursor(205, *choice * 8 + temp_y);

			if (pressed >= 0 && pressed <= 9)
			{
				data[*choice] = check_overflow((uint16_t)data[*choice], (uint16_t)pressed, threshold[*choice]);
			}
			else if (pressed == 10)
			{
				if (data[*choice] > 0)
				{
					data[*choice] /= 10;
					ili9341_backspace(1, bg_c);
				}
			}
			else if (pressed == 15)
			{
				prev_choice = *choice;
				*choice = 20;
			}
		}
		else if (*choice == 20)
		{
			ili9341_settextsize(2);
			ili9341_setcursor(20, 210);
			printf("CONFIRM");
		}
		else if (*choice == 21)
		{
			ili9341_settextsize(2);
			ili9341_setcursor(230, 210);
			printf("BACK");
		}
	} while ((pressed = scan_keypad()) != 15 || *choice <= 19);
}

void write_price(uint16_t x)
{
	if (x >= 1000)
		printf("RP%3hu.%03hu.000\n", x / 1000, x % 1000);
	else if (x > 0)
		printf("RP%7hu.000\n", x);
	else
		printf("RP%11hu\n", x);
	return;
}

void write_quantity(uint16_t x)
{
	printf("%hu", x);
	return;
}

void payment(uint16_t x)
{
	ili9341_setcursor(0, ili9341_gety());
	printf("TUNAI     : ");
	int8_t pressed = 0;
	uint16_t payment = 0;
	uint16_t temp_x = ili9341_getx();
	uint16_t temp_y = ili9341_gety();
	while ((pressed = scan_keypad()) != 15 || payment < x)
	{
		if (pressed >= 0 && pressed <= 9)
		{
			payment = check_overflow(payment, pressed, 65535);
		}else if (pressed == 10)
		{
			payment /= 10;
			ili9341_backspace(1, GREEN);
		}
		ili9341_setcursor(temp_x, temp_y);
		write_price(payment);
	}
	printf("\nTOTAL     : ");
	write_price(x);
	ili9341_write('\n');
	ili9341_drawhline(0, ili9341_gety() + 2, 200, BLUE);
	printf("\nKembalian : ");
	write_price(payment - x);
	return;
}

void init_led(void)
{
	DDRA = 0xFF;
	PORTA &= ~_BV(0);
	PORTA &= ~_BV(1);
	PORTA &= ~_BV(2);
	PORTA &= ~_BV(3);

	uint8_t row = 0;
	//jika stock kurang dari atau sama dengan 30, lampu menyala
	//ROW1-----------------------------------------------------
	if (eeprom_read(0x02) <= 30)
	{
		row |= 0b00000001;
	}
	else
	{
		row &= 0b11111110;
	}

	if (eeprom_read(0x06) <= 30)
	{
		row |= 0b00000010;
	}
	else
	{
		row &= 0b11111101;
	}

	if (eeprom_read(0x0A) <= 30)
	{
		row |= 0b00000100;
	}
	else
	{
		row &= 0b11111011;
	}

	if (eeprom_read(0x0E) <= 30)
	{
		row |= 0b00001000;
	}
	else
	{
		row &= 0b11110111;
	}
	PORTA = row;
	PORTA &= ~_BV(4);	//Clock ON
	PORTA &= ~_BV(5);
	PORTA &= ~_BV(6);
	PORTA |= _BV(7);

	PORTA &= ~_BV(4); //Clock OFF
	PORTA &= ~_BV(5);
	PORTA &= ~_BV(6);
	PORTA &= ~_BV(7);

	//ROW2-----------------------------------------------------
	if (eeprom_read(0x12) <= 30)
	{
		row |= 0b00000001;
	}
	else
	{
		row &= 0b11111110;
	}

	if (eeprom_read(0x16) <= 30)
	{
		row |= 0b00000010;
	}
	else
	{
		row &= 0b11111101;
	}

	if (eeprom_read(0x1A) <= 30)
	{
		row |= 0b00000100;
	}
	else
	{
		row &= 0b11111011;
	}

	if (eeprom_read(0x1E) <= 30)
	{
		row |= 0b00001000;
	}
	else
	{
		row &= 0b11110111;
	}
	PORTA = row;
	PORTA |= _BV(4);	//Clock ON
	PORTA &= ~_BV(5);
	PORTA &= ~_BV(6);
	PORTA |= _BV(7);

	PORTA |= _BV(4); //Clock OFF
	PORTA &= ~_BV(5);
	PORTA &= ~_BV(6);
	PORTA &= ~_BV(7);

	//ROW3-----------------------------------------------------
	if (eeprom_read(0x22) <= 30)
	{
		row |= 0b00000001;
	}
	else
	{
		row &= 0b11111110;
	}

	if (eeprom_read(0x26) <= 30)
	{
		row |= 0b00000010;
	}
	else
	{
		row &= 0b11111101;
	}

	if (eeprom_read(0x2A) <= 30)
	{
		row |= 0b00000100;
	}
	else
	{
		row &= 0b11111011;
	}

	if (eeprom_read(0x2E) <= 30)
	{
		row |= 0b00001000;
	}
	else
	{
		row &= 0b11110111;
	}
	PORTA = row;
	PORTA &= ~_BV(4);	//Clock ON
	PORTA |= _BV(5);
	PORTA &= ~_BV(6);
	PORTA |= _BV(7);

	PORTA &= ~_BV(4); //Clock OFF
	PORTA |= _BV(5);
	PORTA &= ~_BV(6);
	PORTA &= ~_BV(7);

	//ROW4-----------------------------------------------------
	if (eeprom_read(0x32) <= 30)
	{
		row |= 0b00000001;
	}
	else
	{
		row &= 0b11111110;
	}

	if (eeprom_read(0x36) <= 30)
	{
		row |= 0b00000010;
	}
	else
	{
		row &= 0b11111101;
	}

	if (eeprom_read(0x3A) <= 30)
	{
		row |= 0b00000100;
	}
	else
	{
		row &= 0b11111011;
	}

	if (eeprom_read(0x3E) <= 30)
	{
		row |= 0b00001000;
	}
	else
	{
		row &= 0b11110111;
	}
	PORTA = row;
	PORTA |= _BV(4);	//Clock ON
	PORTA |= _BV(5);
	PORTA &= ~_BV(6);
	PORTA |= _BV(7);

	PORTA |= _BV(4); //Clock OFF
	PORTA |= _BV(5);
	PORTA &= ~_BV(6);
	PORTA &= ~_BV(7);

	//ROW5-----------------------------------------------------
	if (eeprom_read(0x42) <= 30)
	{
		row |= 0b00000001;
	}
	else
	{
		row &= 0b11111110;
	}

	if (eeprom_read(0x46) <= 30)
	{
		row |= 0b00000010;
	}
	else
	{
		row &= 0b11111101;
	}

	if (eeprom_read(0x4A) <= 30)
	{
		row |= 0b00000100;
	}
	else
	{
		row &= 0b11111011;
	}

	if (eeprom_read(0x4E) <= 30)
	{
		row |= 0b00001000;
	}
	else
	{
		row &= 0b11110111;
	}
	PORTA = row;
	PORTA &= ~_BV(4);	//Clock ON
	PORTA &= ~_BV(5);
	PORTA |= _BV(6);
	PORTA |= _BV(7);

	PORTA &= ~_BV(4); //Clock OFF
	PORTA &= ~_BV(5);
	PORTA |= _BV(6);
	PORTA &= ~_BV(7);

	return;
}

uint16_t check_overflow(uint16_t a, uint16_t b, uint16_t c)
{
	if (c > 9)
	{
		uint16_t temp = (c - b) / 10;

		if (temp >= a)
			return (10 * a + b);
	}
	else
	{
		if (10 * a + b <= c)
			return (10 * a + b);
	}
	return a;
}

void get_item(type_item *x)
{
	for (register uint8_t i = 0; i < 20; i++)
	{
		for (register uint8_t j = 0; j < 4; j++)
		{
			if (j == 0)
				x[i].code[1] = eeprom_read(4 * i + j);
			else if (j == 1)
				x[i].code[0] = eeprom_read(4 * i + j);
			else if (j == 2)
				x[i].quantity = eeprom_read(4 * i + j);
			else if (j == 3)
				x[i].price = eeprom_read(4 * i + j);
		}
	}
	return;
}

void write_item(type_item *x)
{
	for (register uint8_t i = 0; i < 20; i++)
	{
		for (register uint8_t j = 0; j < 4; j++)
		{
			if (j == 0)
				eeprom_write(4 * i + j, x[i].code[1]);
			else if (j == 1)
				eeprom_write(4 * i + j, x[i].code[0]);
			else if (j == 2)
				eeprom_write(4 * i + j, x[i].quantity);
			else if (j == 3)
				eeprom_write(4 * i + j, x[i].price);
		}
	}
	return;
}

void eeprom_write(uint16_t write_addr, uint8_t write_data)
{
	SREG &= 0x7F;
	while (EECR & (1 << 1)); // EEWE
	EEAR = write_addr;
	EEDR = write_data;
	EECR |= (1 << 2);
	EECR |= (1 << 1);
	SREG |= (1 << 7);
	return;
}

uint16_t eeprom_read(uint16_t read_addr)
{
	SREG &= 0x7F;
	while (EECR & (1 << 1)); //EEWE
	EEAR = read_addr;
	EECR |= (1 << 0); //EERE
	SREG |= (1 << 7);
	return EEDR;
}

void init_keypad(void)
{
	DDRC = 0x03;
	PORTC = 0xFC;

	for (uint8_t i = 0; i < 4; i++)
	{
		PORTC |= _BV(0); //ABCD
		PORTC |= _BV(1); //1000 -> 1111
		PORTC &= ~_BV(1);
	}
	return;
}

int8_t scan_keypad(void)
{
	PORTC &= ~_BV(0); //ABCD
	PORTC |= _BV(1);  //0111
	PORTC &= ~_BV(1);
	for (uint8_t i = 0; i < 100; i++)
	{
		if (bit_is_clear(PINC, 2))
		{
			//1
			while (bit_is_clear(PINC, 2));
			return 1;
		}else if (bit_is_clear(PINC, 3))
		{
			//2
			while (bit_is_clear(PINC, 3));
			return 2;
		}else if (bit_is_clear(PINC, 4))
		{
			//3
			while (bit_is_clear(PINC, 4));
			return 3;
		}else if (bit_is_clear(PINC, 5))
		{
			//DEL
			while (bit_is_clear(PINC, 5));
			return 10;
		}else if (bit_is_clear(PINC, 6))
		{
			//UP
			while (bit_is_clear(PINC, 6));
			return 11;
		}/*else if (bit_is_clear(PINC, 7))
		{
			//EMPTY
			while (bit_is_clear(PINC, 7));
			return 16;
		}*/
	}

	PORTC |= _BV(0); //ABCD
	PORTC |= _BV(1); //1011
	PORTC &= ~_BV(1);
	for (uint8_t i = 0; i < 100; i++)
	{
		if (bit_is_clear(PINC, 2))
		{
			//4
			while (bit_is_clear(PINC, 2));
			return 4;
		}else if (bit_is_clear(PINC, 3))
		{
			//5
			while (bit_is_clear(PINC, 3));
			return 5;
		}else if (bit_is_clear(PINC, 4))
		{
			//6
			while (bit_is_clear(PINC, 4));
			return 6;
		}else if (bit_is_clear(PINC, 5))
		{
			//LEFT
			while (bit_is_clear(PINC, 5));
			return 12;
		}else if (bit_is_clear(PINC, 6))
		{
			//DOWN
			while (bit_is_clear(PINC, 6));
			return 13;
		}else if (bit_is_clear(PINC, 7))
		{
			//RIGHT
			while (bit_is_clear(PINC, 7));
			return 14;
		}
	}
	//delay_ms(1000);

	PORTC |= _BV(0); //ABCD
	PORTC |= _BV(1); //1101
	PORTC &= ~_BV(1);
	for (uint8_t i = 0; i < 100; i++)
	{
		if (bit_is_clear(PINC, 2))
		{
			//7
			while (bit_is_clear(PINC, 2));
			return 7;
		}else if (bit_is_clear(PINC, 3))
		{
			//8
			while (bit_is_clear(PINC, 3));
			return 8;
		}else if (bit_is_clear(PINC, 4))
		{
			//9
			while (bit_is_clear(PINC, 4));
			return 9;
		}/*else if (bit_is_clear(PINC, 5))
		{
			//EMPTY
			while (bit_is_clear(PINC, 5));
			return 16;
		}else if (bit_is_clear(PINC, 6))
		{
			//EMPTY //ENTER
			while (bit_is_clear(PINC, 6));
			return 16;
		}else if (bit_is_clear(PINC, 7))
		{
			//EMPTY
			while (bit_is_clear(PINC, 7));
			return 16;
		}*/
	}
	//delay_ms(1000);

	PORTC |= _BV(0); //ABCD
	PORTC |= _BV(1); //1110
	PORTC &= ~_BV(1);
	for (uint8_t i = 0; i < 100; i++)
	{
		/*if (bit_is_clear(PINC, 2))
		{
			//EMPTY
			while (bit_is_clear(PINC, 2));
			return 16;
		}*/
		if (bit_is_clear(PINC, 3))
		{
			//0
			while (bit_is_clear(PINC, 3));
			return 0;
		}/*else if (bit_is_clear(PINC, 4))
		{
			//EMPTY
			while (bit_is_clear(PINC, 4));
			return 16;
		}else if (bit_is_clear(PINC, 5))
		{
			//EMPTY
			while (bit_is_clear(PINC, 5));
			return 16;
		}*/else if (bit_is_clear(PINC, 6))
		{
			//ENTER
			while (bit_is_clear(PINC, 6));
			return 15;
		}/*else if (bit_is_clear(PINC, 7))
		{
			//EMPTY
			while (bit_is_clear(PINC, 7));
			return 16;
		}*/
	}
	//delay_ms(1000);
	return -1;
}

void delay(uint16_t x)
{
	do
		_delay_ms(1);
	while (x--);
	return;
}

void init_item(type_item *x)
{
	for (register uint8_t i = 0; i < 20; i++)
	{
		x[i].code[0] = i % 10 + 0x30;
		x[i].code[1] = i / 10 + 0x30;
		x[i].quantity = 200;
	}
	x[0].price = 10;
	x[1].price = 21;
	x[2].price = 3;
	x[3].price = 5;
	x[4].price = 8;
	x[5].price = 12;
	x[6].price = 50;
	x[7].price = 8;
	x[8].price = 12;
	x[9].price = 22;
	x[10].price = 5;
	x[11].price = 12;
	x[12].price = 59;
	x[13].price = 80;
	x[14].price = 3;
	x[15].price = 8;
	x[16].price = 5;
	x[17].price = 3;
	x[18].price = 8;
	x[19].price = 8;

	for (register uint8_t i = 0; i < 20; i++)
	{
		for (register uint8_t j = 0; j < 4; j++)
		{
			if (j == 0)
				eeprom_write(4 * i + j, x[i].code[1]);
			else if (j == 1)
				eeprom_write(4 * i + j, x[i].code[0]);
			else if (j == 2)
				eeprom_write(4 * i + j, x[i].quantity);
			else if (j == 3)
				eeprom_write(4 * i + j, x[i].price);
		}
	}
	return;
}
/*
00 10000       05 12000         10 5000           15 8000
01 21000       06 5000          11 12000          16 5000
02 3000        07 8000          12 59000          17 3000
03 5000        08 12000         13 80000          18 8000
04 8000        09 22000         14 3000           19 8000
*/