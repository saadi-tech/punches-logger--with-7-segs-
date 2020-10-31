/*
lcdpcf8574 lib sample

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#define F_CPU 8000000UL
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>

//Including the library to work with the 7-segments display/
#include "tm1637.h"
#include "tm1637_2.h"

#include <avr/sleep.h>
#include "fat.h"
#include "fat_config.h"
#include "partition.h"
#include "sd_raw.h"
#include "sd_raw_config.h"


#define DEBUG 1

#define UART_BAUD_RATE 9600
#include "uart.h"
#define PIN(x) (*(&x - 2))    /* address of input register of port x */


void set_hits(uint16_t );  //Display one
void set_last_hit(uint16_t ); // display two


bool open_file(char[] ,char[] );
void InitADC();
uint16_t ReadADC(uint8_t);
static uint8_t read_line(char* buffer, uint8_t buffer_length);
static uint32_t strtolong(const char* str);
static uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry);
static struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name);
static uint8_t print_disk_info(const struct fat_fs_struct* fs);
bool Init_sdcard();
uint8_t map_to_byte(uint16_t);
struct partition_struct* partition;
struct fat_fs_struct* fs;
struct fat_dir_entry_struct directory;
struct fat_dir_struct* dd;
struct fat_file_struct* fd;

bool hit_started_l,hit_started_u,hit_started_r;
bool sd_loaded = false;

uint16_t reading_l = 0;
uint16_t reading_r = 0;
uint16_t reading_u = 0;
uint16_t last_reading_l = 0;
uint16_t last_reading_r = 0;
uint16_t last_reading_u = 0;

uint8_t led = 0;
int32_t offset=0;
char temp[10];



volatile uint8_t hits[10];
volatile char hits_types[10];
char force_val[10];
volatile uint8_t hits_pt = 0;
uint16_t hits_counter = 0;
uint8_t last_hit = 0;

char times = 0;
int main(void)
{	
	//init lcd
	TM1637_init(1/*enable*/, 7/*brightness*/);
	TM1637_2_init(1/*enable*/, 7/*brightness*/);
	set_null_1();
	set_null_2();
	
	DDRB = 0x00;
	PORTB = 0x01;
	
	
	
	//init uart
	uart_init();
	InitADC();
	init_timer_interrupt();
    
	uart_puts_p(PSTR("Welcome..."));
	sd_loaded = Init_sdcard();
	if (sd_loaded){
	set_hits(0);
	set_last_hit(0);
	}
	else{
	while(1){
		cli();
		set_null_1();
		set_null_2();
		_delay_ms(500);
		TM1637_clear();
		TM1637_2_clear();
		_delay_ms(500);
	}
	}
	
	
    sei();

    

    _delay_ms(1000);
	
	//-------------
	set_hits(hits_counter);
	set_last_hit(last_hit);
    
    
	reset_hits();
    while(1) {
		
		
		if ( (PINB & (1<<0)) == 0){
			uart_puts("Button pressed..");
			_delay_ms(2000);
			
			if( (PINB & (1<<0)) == 0){
				reset_data();
				set_done1();
				set_done2();
				_delay_ms(1000);
				set_hits(hits_counter);
				set_last_hit(last_hit);
				
			}
		}
		
		
		for (int i = 0; i < 3;i++){
			
		switch (i){
			
			case 0 :	
				reading_l = ReadADC(0);
				if (!hit_started_l){
					if (reading_l > 100 && reading_l>last_reading_l){
					hit_started_l = true;
					}
				}
				else{
					if ( reading_l>100 && reading_l < last_reading_l){
				  
					hits[hits_pt] = map_to_byte(reading_l);
					hits_types[hits_pt] = 'l';
					last_hit = hits[hits_pt];
					
					hits_pt++;
				  
					hits_counter++;
					
					hit_started_l = false;
					
					set_hits(hits_counter);
					set_last_hit(last_hit);
					
				  
				  
					_delay_ms(130);
				
				  
				  
						}
					}
					last_reading_l = reading_l;
					break;
					
			case 1:
				reading_r = ReadADC(1);
				if (!hit_started_r ){
					if (reading_r  > 100 && reading_r >last_reading_r ){
						hit_started_r  = true;
					}
				}
				else{
					if ( reading_r >100 && reading_r  < last_reading_r ){
						
						hits[hits_pt] = map_to_byte(reading_r );
						hits_types[hits_pt] = 'r';
						last_hit = hits[hits_pt];
						
						hits_pt++;
						
						hits_counter++;
						
						hit_started_r  = false;
						
						set_hits(hits_counter);
						set_last_hit(last_hit);
						
						
						_delay_ms(130);
						
						
						
					}
				}
				last_reading_r  = reading_r ;
				break;
				
			case 2:
			reading_u = ReadADC(2);
			if (!hit_started_u ){
				if (reading_u  > 100 && reading_u >last_reading_u ){
					hit_started_u  = true;
				}
			}
			else{
				if ( reading_u >100 && reading_u  < last_reading_u ){
					
					hits[hits_pt] = map_to_byte(reading_u );
					hits_types[hits_pt] = 'u';
					last_hit = hits[hits_pt];
					
					hits_pt++;
					
					hits_counter++;
				
					hit_started_u  = false;
					
					
					set_hits(hits_counter);
					set_last_hit(last_hit);
					
					
					
					_delay_ms(130);
					
					
					
				}
			}
			last_reading_u  = reading_u ;
			break;
			
			}
		}
    }
}


void show_hits(){
	uart_puts("HITS:\n");
	for (int i =0 ; i < 10 ; i++){
		
		itoa(hits[i],force_val,10);
		uart_puts(force_val);
		uart_putc(',');
		uart_putc(hits_types[i]);
		uart_putc(' ');
		
	}
	uart_putc('\n');
}
void init_timer_interrupt(){
	OCR1A = 0x3D08;

	TCCR1B |= (1 << WGM12);
	// Mode 4, CTC on OCR1A

	TIMSK1 |= (1 << OCIE1A);
	//Set interrupt on compare match

	TCCR1B |= (1 << CS12) | (1 << CS10);
	// set prescaler to 1024 and start the timer
}



ISR (TIMER1_COMPA_vect)
{	
	uart_puts("Interrupt..");
	bool all_ok = true;
	times++;
	if (times == 2){
	// action to be done every 3 sec
	show_hits();
	for (int i =0 ; i < hits_pt ; i++){
		
		itoa(hits[i],force_val,10);
		strcat(force_val,",");
		char ap = hits_types[i];
		strncat(force_val, &ap, 1); 
		uart_puts(force_val);
		if(!open_file("data.txt",force_val)){
			all_ok = false;
		}
	}
	if (all_ok){
		uart_puts("L.\n");
	}
	else{
		uart_puts("Error logging.");
	}
	
	reset_hits();
	times = 0;
	}
}

void reset_data(){
	    struct fat_dir_entry_struct file_entry;
	    if(find_file_in_dir(fs, dd, "data.txt", &file_entry))
	    {
		    if(fat_delete_file(fs, &file_entry)){
				//
			}
		    
	    }
		else{
	    uart_puts_p(PSTR("error deleting file: "));
	    uart_puts( "data.txt");
	    uart_putc('\n');
		}
  
	    if(!fat_create_file(dd,  "data.txt", &file_entry))
	    {
		    uart_puts_p(PSTR("error creating file: "));
		    uart_puts( "data.txt");
		    uart_putc('\n');
	    }
		else{
			set_null_1();
			set_null_2();
		}
    
}
void reset_hits(){
	hits_pt = 0;
	for (char i = 0; i <10;i++){
		hits[i] = 0;
		hits_types[i] = 'e';
	}
	
}


bool Init_sdcard(){
		//lcd_gotoxy(0,0);
		//lcd_puts("Setting SD-CARD.");
		bool all_ok = true;
	        /* setup sd card slot */
		set_sleep_mode(SLEEP_MODE_IDLE);
        if(!sd_raw_init())
        {
#if DEBUG
            uart_puts_p(PSTR("MMC/SD initialization failed\n"));
			all_ok = false;
#endif
            //continue;
        }
		else{
			uart_puts_p(PSTR("MMC/SD initialization Succeeded.\n"));
		}

        /* open first partition */
      partition = partition_open(sd_raw_read,
                                                            sd_raw_read_interval,
#if SD_RAW_WRITE_SUPPORT
                                                            sd_raw_write,
                                                            sd_raw_write_interval,
#else
                                                            0,
                                                            0,
#endif
                                                            0
                                                           );

        if(!partition)
        {
            /* If the partition did not open, assume the storage device
             * is a "superfloppy", i.e. has no MBR.
             */
            partition = partition_open(sd_raw_read,
                                       sd_raw_read_interval,
#if SD_RAW_WRITE_SUPPORT
                                       sd_raw_write,
                                       sd_raw_write_interval,
#else
                                       0,
                                       0,
#endif
                                       -1
                                      );
            if(!partition)
            {
#if DEBUG
                uart_puts_p(PSTR("opening partition failed\n"));
				all_ok = false;
#endif
                //continue;
            }
			else{
				uart_puts_p(PSTR("Opening partition Succeeded.\n"));
			}
			
        }

        /* open file system */
         fs = fat_open(partition);
        if(!fs)
        {
#if DEBUG
            uart_puts_p(PSTR("opening filesystem failed\n"));
			all_ok = false;
#endif
            //continue;
        }
		else{
			uart_puts_p(PSTR("opening filesystem  Succeeded.\n"));
		}

        /* open root directory */
        
        fat_get_dir_entry_of_path(fs, "/", &directory);

        dd = fat_open_dir(fs, &directory);
        if(!dd)
        {
#if DEBUG
            uart_puts_p(PSTR("opening root directory failed\n"));
			all_ok = false;
#endif
            //continue;
        }
		else{
			uart_puts_p(PSTR("opening root directory Done\n"));
		}
        
        /* print some card information as a boot message */
        //print_disk_info(fs);
		_delay_ms(500);
		return all_ok;
}

uint8_t map_to_byte(uint16_t val){
	return (val/10);
}


bool open_file(char file_name[],char force[]){
	bool all_ok = true;
	 fd = open_file_in_dir(fs, dd, file_name);
	if(!fd)
	{
		uart_puts_p(PSTR("error opening "));
		uart_puts(file_name);
		uart_putc('\n');
		all_ok = false;
		//continue;
	}
	else{
		/*uart_puts_p(PSTR("Opened file: "));
		uart_puts(file_name);
		uart_putc('\n');*/
	}
	
	 offset = 0;
	if(!fat_seek_file(fd, &offset, FAT_SEEK_END))
	{
		uart_puts_p(PSTR("error seeking on "));
		uart_puts(file_name);
		uart_putc('\n');

		fat_close_file(fd);
		all_ok = false;
	}
//uart_puts_p(PSTR("Done seeking\n"));
/* write text to file */
char line[10];
uart_puts("Force: ");
uart_puts(force);
uart_putc('\n');
snprintf(line, sizeof (line), "%s\n", force);
uart_puts("Force with type: ");
uart_puts(line);
uart_putc('\n');

		if(fat_write_file(fd, (uint8_t*) line, strlen(line)) != strlen(line))
		{
			uart_puts_p(PSTR("error writing to file\n"));
			all_ok = false;
		}
	

	fat_close_file(fd);
	//uart_puts_p(PSTR("closed file\n"));
	return all_ok;
}
//------------------------------------------------------------
void InitADC()
{
	// Select Vref=AVcc
	ADMUX |= (1<<REFS0);
	//set prescaller to 128 and enable ADC
	ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);
}
//----------------------------------------------------------

uint16_t ReadADC(uint8_t ADCchannel)
{
	//select ADC channel with safety mask
	ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
	//single conversion mode
	ADCSRA |= (1<<ADSC);
	// wait until ADC conversion is complete
	while( ADCSRA & (1<<ADSC) );
	return ADC;
}

uint8_t read_line(char* buffer, uint8_t buffer_length)
{
	memset(buffer, 0, buffer_length);

	uint8_t read_length = 0;
	while(read_length < buffer_length - 1)
	{
		uint8_t c = uart_getc();

		if(c == 0x08 || c == 0x7f)
		{
			if(read_length < 1)
			continue;

			--read_length;
			buffer[read_length] = '\0';

			uart_putc(0x08);
			uart_putc(' ');
			uart_putc(0x08);

			continue;
		}

		uart_putc(c);

		if(c == '\n')
		{
			buffer[read_length] = '\0';
			break;
		}
		else
		{
			buffer[read_length] = c;
			++read_length;
		}
	}

	return read_length;
}

uint32_t strtolong(const char* str)
{
	uint32_t l = 0;
	while(*str >= '0' && *str <= '9')
	l = l * 10 + (*str++ - '0');

	return l;
}

uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry)
{
	while(fat_read_dir(dd, dir_entry))
	{
		if(strcmp(dir_entry->long_name, name) == 0)
		{
			fat_reset_dir(dd);
			return 1;
		}
	}

	return 0;
}

struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name)
{
	struct fat_dir_entry_struct file_entry;
	if(!find_file_in_dir(fs, dd, name, &file_entry))
	return 0;

	return fat_open_file(fs, &file_entry);
}

uint8_t print_disk_info(const struct fat_fs_struct* fs)
{
	if(!fs)
	return 0;

	struct sd_raw_info disk_info;
	if(!sd_raw_get_info(&disk_info))
	return 0;

	uart_puts_p(PSTR("manuf:  0x")); uart_putc_hex(disk_info.manufacturer); uart_putc('\n');
	uart_puts_p(PSTR("oem:    ")); uart_puts((char*) disk_info.oem); uart_putc('\n');
	uart_puts_p(PSTR("prod:   ")); uart_puts((char*) disk_info.product); uart_putc('\n');
	uart_puts_p(PSTR("rev:    ")); uart_putc_hex(disk_info.revision); uart_putc('\n');
	uart_puts_p(PSTR("serial: 0x")); uart_putdw_hex(disk_info.serial); uart_putc('\n');
	uart_puts_p(PSTR("date:   ")); uart_putw_dec(disk_info.manufacturing_month); uart_putc('/');
	uart_putw_dec(disk_info.manufacturing_year); uart_putc('\n');
	uart_puts_p(PSTR("size:   ")); uart_putdw_dec(disk_info.capacity / 1024 / 1024); uart_puts_p(PSTR("MB\n"));
	uart_puts_p(PSTR("copy:   ")); uart_putw_dec(disk_info.flag_copy); uart_putc('\n');
	uart_puts_p(PSTR("wr.pr.: ")); uart_putw_dec(disk_info.flag_write_protect_temp); uart_putc('/');
	uart_putw_dec(disk_info.flag_write_protect); uart_putc('\n');
	uart_puts_p(PSTR("format: ")); uart_putw_dec(disk_info.format); uart_putc('\n');
	uart_puts_p(PSTR("free:   ")); uart_putdw_dec(fat_get_fs_free(fs)); uart_putc('/');
	uart_putdw_dec(fat_get_fs_size(fs)); uart_putc('\n');

	return 1;
}

#if FAT_DATETIME_SUPPORT
void get_datetime(uint16_t* year, uint8_t* month, uint8_t* day, uint8_t* hour, uint8_t* min, uint8_t* sec)
{
	*year = 2007;
	*month = 1;
	*day = 1;
	*hour = 0;
	*min = 0;
	*sec = 0;
}
#endif


void set_hits(uint16_t val){
	TM1637_display_colon(0);
	for (int i = 0;i <4 ; i++){
		TM1637_display_digit(3-i,val%10);
		val = val/10;
	}
}

void set_last_hit(uint16_t val){
	TM1637_2_display_colon(0);
	
	for (int i = 0;i <4 ; i++){
		TM1637_2_display_digit(3-i,val%10);
		val = val/10;
	}
	
}

void set_null_1(){
	TM1637_display_colon(0);
	
	for (int i = 0;i <4 ; i++){
		 TM1637_display_segments(i, 0b1000000);
		
	}
	
}

void set_null_2(){
	TM1637_2_display_colon(0);
	
	for (int i = 0;i <4 ; i++){
	 TM1637_2_display_segments(i, 0b1000000);
		
	}
	
}

void set_done1(){
TM1637_display_colon(0);
TM1637_display_segments(0, 0b0111111);
TM1637_display_segments(1, 0b1011100);
TM1637_display_segments(2, 0b1010100);
TM1637_display_segments(3, 0b1111001);
}
void set_done2(){
	TM1637_2_display_colon(0);
	TM1637_2_display_segments(0, 0b0111111);
	TM1637_2_display_segments(1, 0b1011100);
	TM1637_2_display_segments(2, 0b1010100);
	TM1637_2_display_segments(3, 0b1111001);
}