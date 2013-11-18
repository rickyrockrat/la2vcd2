/** \file ******************************************************************
\n\b File:        la2vcd2.h
\n\b Author:      Initial: Eric Smith <eric@brouhaha.com>
									Library conversion: Doug Springer
\n\b Company:     DNK Designs Inc.
\n\b Date:        01/16/2010  3:43 pm
\n\b Description: Header file for the la2vcd library
*/ /************************************************************************
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2 as
 published by the Free Software Foundation.  Note that permission is
 not granted to redistribute this program under the terms of any
 other version of the General Public License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111  USA

 Rickyrockrat split most of this from la2vcd.c so it can be used as a library.

Change Log: \n

*/
#ifndef _LA2VCD2_H_ 
#define _LA2VCD2_H_  1

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum VarTypes { V_EVENT, V_PARAMETER,
                V_INTEGER, V_REAL, V_REAL_PARAMETER=V_REAL, V_REALTIME=V_REAL, V_REG, V_SUPPLY0,
                V_SUPPLY1, V_TIME, V_TRI, V_TRIAND, V_TRIOR,
                V_TRIREG, V_TRI0, V_TRI1, V_WAND, V_WIRE, V_WOR, V_PORT, V_IN=V_PORT, V_OUT=V_PORT, V_INOUT=V_PORT,
                V_END, V_LB, V_COLON, V_RB, V_STRING };


#ifndef TOSTRING
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#endif

#define INPUT_FILE_BUF_SIZE 255

typedef struct input_file_info{
  struct input_file_info *next;
  FILE *inf;
  int radix;
  int bit_count;
  char *name;
  char *buf;
  uint64_t allocated_mask;  // bits which are allocated to signals
  uint64_t value;           /**digital  */
  double rvalue;            /**analog  */
  int var_type;             /**var type for this file, currently V_WIRE and V_REAL  */
} input_file_info_t;

typedef struct signal_info{
  struct signal_info *next;
  struct input_file_info *input_file;
  uint64_t source_mask;  // which bits from input file are extracted and
                         // packed to get the value of this signal
  int bit_count;         // total bits in signal

  int max_index;         // index range when displayed as vector
  int min_index;
  char *name;
  char *vcd_sym;
  double rvalue;            /**analog  */
  double prvalue;
  
  int var_type;          /**  type of signal, i.e.event | integer | parameter | real | reg | 
                          supply0 | supply1 | time | tri | triand | 
                          trior | trireg | tri0 | tri1 | wand | wire | wor  */
  uint64_t value;         
  uint64_t prev_value;
} signal_info_t;


struct la2vcd {
	double time_delta;
	double time_scale;
	double time_offset;
	double current_time;
	double prev_time;
	FILE *outf;
	FILE *timef;
	
	struct input_file_info file_info;
	char *module_name;
	struct signal_info *first_signal;
	struct signal_info *last_signal;
	struct input_file_info *first_input_file;
	struct input_file_info *last_input_file;
};

struct la2vcd *open_la2vcd (char *out, char *timefile, double timedelta, double timescale, char *mname);
double parse_time_la2vcd (char *p, double *numeric, char **unit);
double parse_time_scale_la2vcd (char *p);
void vcd_read_sample(struct la2vcd *l);
void vcd_read_sample_real(struct la2vcd *l, double val);
signal_info_t *split_field (struct signal_info **first, struct signal_info **last,struct input_file_info *f,char *bits, char *name);
int add_input_file(struct input_file_info **first, struct input_file_info **last, char *in, int radix, int bits, int type);
signal_info_t *split_signals_from_input (signal_info_t *first_signal,signal_info_t *last_signal, input_file_info_t *input_file);
int vcd_add_signal (struct la2vcd *l, int type, int msb, int lsb, char *name);
int vcd_add_file(struct la2vcd *l, char *in, int radix, int bits, int type);
int write_vcd_header (struct la2vcd *l);
int advance_time (struct la2vcd *l);
void write_vcd_data (struct la2vcd *l);
int read_one_line (struct la2vcd *l);
void write_vcd_trailer (struct la2vcd *l);
void close_la2vcd(struct la2vcd *l);

#endif 

