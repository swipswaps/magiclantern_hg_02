/*
 * Imported and significantly modified from dump_toolkit
 *
 * This program replaces the update_manager() routine in the 5D Mark 2
 * firmware flashing program at location 0x805ab8 and is used to copy the
 * ROM images to files on the compact flash card.
 *
 * The memory layout of the 5D is:
 *
 * 0x0080_0000 Flasher starting address
 * 0xFF00_0000 Strings and other stuff in ROM
 * 0xFF80_0000 Main firmware in ROM
 */
/*
 * Copyright (C) 2009 Trammell Hudson <hudson+ml@osresearch.net>
 * Original copyright unknown.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include <stdint.h>

#define ROM0_ADDRESS	0xF8000000
#define ROM0_SIZE	0x7FFFF0
#define ROM1_ADDRESS	0xF0000000
#define ROM1_SIZE	0x800000

#define O_WRONLY        0x001
#define O_CREAT         0x200

#pragma long_calls
extern void * open(const char *name, int flags, int mode);	// mode is for VxW only
extern int close(void * file);
extern int write(void * file, void * buffer, unsigned long nbytes);
extern void * creat(char *name, int flags);
extern void shutdown(void);

/* Not sure what these do, but they are called by shutdown */
extern void abort_firmup1( void );
extern void abort_firmup2( void );
extern void reboot_icu( void *, void *, uint32_t );
#pragma no_long_calls

#define INVALID_HANDLE	((void *)0xFFFFFFFF)
#define NULL			((void *)0)

#define LOG() \
	do { \
		uint32_t error = __LINE__; \
		if( logfile != INVALID_HANDLE ) \
			write( logfile, &error, sizeof(error) ); \
	} while(0)


static inline void *
reg_sp( void )
{
	void * sp;
	__asm__ __volatile__( "mov %0, sp" : "=r"(sp) );
	return sp;
}

int main( void )
{
	void * const	rom0_start	= (void *) ROM0_ADDRESS;
	uint32_t	rom0_size	= ROM0_SIZE;
	void * const	rom1_start	= (void *) ROM1_ADDRESS;
	uint32_t	rom1_size	= ROM1_SIZE;
	void * const	ram_start	= (void*) 0xFF800000;
	uint32_t	ram_size	= ROM0_SIZE;

	char fname[12];

	fname[0] = 'A'; fname[1] = ':'; fname[2] = '/';
	fname[3] = 'R'; fname[4] = 'O'; fname[5] = 'M'; fname[6] = '0';
	fname[7] = '.';
	fname[8] = 'l'; fname[9] = 'o'; fname[10] = 'g';
	fname[11] = '\0';
	

	const uint32_t model_id = *(uint32_t*) 0x800000;

	// Verify that we are on a 5D
	if( model_id != 0x80000218 )
		while(1);

#if 0
	// Go into supervisor mode
	asm(
                 "MRS     R0, CPSR\n"
                 "BIC     R0, R0, #0x3F\n"
                 "ORR     R0, R0, #0xD3\n"
                 "MSR     CPSR, R0\n"
		: : : "r0"
	);
#endif


	void * logfile = open( fname, O_WRONLY | O_CREAT, 0666 );
	if( logfile == INVALID_HANDLE )
		logfile = creat( fname, O_WRONLY );
	if( logfile == INVALID_HANDLE )
		shutdown();
	
	write( logfile, &model_id, sizeof(model_id) );

	// Change the name to .bin
	fname[8] = 'b'; fname[9] = 'i'; fname[10] = 'n'; fname[11] = '\0';

	void *f = open(fname, O_CREAT|O_WRONLY, 0644);
	if( INVALID_HANDLE == f )
	{
		LOG();
		f = creat( fname, O_WRONLY );	// O_CREAT is not working in DRYOS
	}
	if( INVALID_HANDLE == f )
	{
		LOG();
		shutdown();
	}

	write( f, rom0_start, rom0_size);
	close( f );

	if( rom1_size == 0 )
	{
		LOG();
		shutdown();
	}

	// Change the name from ROM0 to ROM1
	fname[6] = '1';
	f = open(fname, O_CREAT|O_WRONLY, 0644);
	if( INVALID_HANDLE == f )
	{
		LOG();
		f = creat( fname, O_WRONLY );	// O_CREAT is not working in DRYOS
	}
	if( INVALID_HANDLE == f )
	{
		LOG();
		shutdown();
	}

	LOG();
	write(f, rom1_start, rom1_size);
	close(f);


#if 0
	// Change the name to RAM0.bin
	fname[4] = 'A';
	fname[6] = '0';

	f = open(fname, O_CREAT|O_WRONLY, 0644);
	if( INVALID_HANDLE == f )
	{
		LOG();
		f = creat( fname, O_WRONLY );	// O_CREAT is not working in DRYOS
	}
	if( INVALID_HANDLE == f )
	{
		LOG();
		shutdown();
	}

	LOG();
	write( f, ram_start, ram_size);
	close( f );
#endif



	// Generate some info about the CPU
	// cpu id code == 0x41059461
	{
	uint32_t value;
	asm(
                 "MRS     %0, CPSR\n"
                 "BIC     %0, %0, #0x1F\n" // clear out mode bits
                 "ORR     %0, %0, #0x13\n" // set supervisor mode
                 "MSR     CPSR, %0\n"
		"mrc p15, 0, %0, c0, c0, 0" : "=r"(value) );
	write( logfile, &value, sizeof(value) );
	}

#define writelog_mrc( cn, cm, arg ) \
	do { \
		uint32_t value; \
		asm( "mrc p15, 0, %0, " #cn ", " #cm ", " #arg "\n" \
			: "=r"(value) \
		); \
		write( logfile, &value, sizeof(value) ); \
	} while(0);

	LOG();
	writelog_mrc( c1, c0, 0 ); // control register

	writelog_mrc( c2, c0, 0 ); // data cache bits
	writelog_mrc( c2, c0, 1 ); // inst cache bits

	writelog_mrc( c3, c0, 0 ); // data bufferable bits
	writelog_mrc( c3, c0, 1 ); // inst bufferable bits

	writelog_mrc( c5, c0, 2 ); // extended data access
	writelog_mrc( c5, c0, 3 ); // extended inst access

	writelog_mrc( c6, c0, 0 ); // region 0
	writelog_mrc( c6, c1, 0 ); // region 1
	writelog_mrc( c6, c2, 0 ); // region 2
	writelog_mrc( c6, c3, 0 ); // region 3
	writelog_mrc( c6, c4, 0 ); // region 4
	writelog_mrc( c6, c5, 0 ); // region 5
	writelog_mrc( c6, c6, 0 ); // region 6
	writelog_mrc( c6, c7, 0 ); // region 7

	writelog_mrc( c9, c0, 0 ); // data lockdown
	writelog_mrc( c9, c0, 1 ); // inst lockdown
	writelog_mrc( c9, c1, 0 ); // data tcm
	writelog_mrc( c9, c2, 1 ); // inst tcm

	LOG();
	abort_firmup1();
	LOG();
	abort_firmup2();
	LOG();
	uint32_t dummy = 0;
	reboot_icu( 0x80010003, dummy, 4 );
	LOG();

	LOG();
	close( logfile );

	return 0;
}
