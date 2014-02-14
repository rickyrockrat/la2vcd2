/*
 * la2vcd: convert logic analyzer waveforms to a VCD file
 *
 * Main program
 * Copyright 2005 Eric Smith <eric@brouhaha.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.  Note that permission is
 * not granted to redistribute this program under the terms of any
 * other version of the General Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA
 *
 * The following changes were made by gpib@rickyrockrat.net:
 * The bug fixes are as follows:
 *
 * 1) Signal splitting doesn't allow all signals, i.e. 3-5 only gets 3-4:
 * Line 535:
 * <signal->source_mask |= (((1ULL << i) - 1ULL) & ~ ((1ULL << j) - 1ULL));
 * >signal->source_mask |= (((1ULL << (i+1)) - 1ULL) & ~ ((1ULL << j) - 1ULL));
 *
 * 2) An error occurs if bits are > 32 ("fatal error: field bit positions out of range").
 * Line 761:
 * <if (new_signals->source_mask & ~ ((1 << new_input_file->bit_count) - 1)) 
 * >if (new_signals->source_mask & ~ ((1ULL << new_input_file->bit_count) - 1))
 *
 * On 01/17/2010, rickyrockrat moved the core to la2vcd_lib.c, and made various changes
 * to support that effort. Much still needs to be done to move all the error control from
 * the fatal exit function to letting errors drift up to the calling application(s).
 */


#include "libla2vcd2.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#define _VERSION "la2vcd2 ""PACKAGE_VERSION"
char *progname;

extern signal_info_t *_vcd_add_signal (struct signal_info **first, struct signal_info **last, struct input_file_info *f,char *name, int type, int lsb, int msb);

void usage (FILE *f)
{
	fprintf(f,"la2vcd %s\n",TOSTRING(PACKAGE_VERSION));
  fprintf (f, "usage:\n");
  fprintf (f, "  %s [options] signalfile...]\n", progname);
  fprintf (f, "options:\n");
  fprintf (f, "  -ta <timefile>     times from file (mutually exclusive with -td)\n");
  fprintf (f, "  -td <time>         set delta time\n");
  fprintf (f, "  -r <radix>         set radix of next input file\n");
  fprintf (f, "  -b <count>         set bit count of next input file\n");
  fprintf (f, "  -s                 split next input file into individual bits\n");
  fprintf (f, "  -sf <bits> <name>  split a field from the next input file\n");
  fprintf (f, "  -ts <time>         set timescale of VCD file (default 1 ns)\n");
  fprintf (f, "  -mn <modulename>   set module name of signals in VCD file\n");
  fprintf (f, "  -o <vcdfile>       write VCD output to file (default stdout)\n");
  fprintf (f, "syntax:\n");
  fprintf (f, "  <radix> ::= 2 | 8 | 16\n");
  fprintf (f, "  <time> ::= <integer><unit>\n");
  fprintf (f, "  <unit> ::= s|ms|us|ns|ps|fs\n");
  fprintf (f, "  <bits> ::= <bitr>|<bitr>,<bits>\n");
  fprintf (f, "  <bitr> ::= <integer>|<integer>-<integer>\n");
}



 /************************************************************************/
void fatal (int ret, char *format, ...)
{
  va_list ap;

  fprintf (stderr, "fatal error: ");
  va_start (ap, format);
  vfprintf (stderr, format, ap);
  va_end (ap);
  if (ret == 1)
    usage (stderr);
  exit (ret);
}



/**  
-o outf
-ta timef
-td time_delta
-ts time_scale
-r next_radix
-b next_bit_count
next_split
-sf signal = split_field (argv [1], argv [2]);
-mn module_name
*/
int main (int argc, char *argv [])
{
	struct la2vcd *l;
  progname = "la2vcd2";
  
  int next_radix = 0;
  int next_bit_count = 0;
  bool next_split = false;
  signal_info_t *new_signals,*first_signal,*last_signal;
	input_file_info_t *first_input_file, *last_input_file;
  double timedelta,timescale;
	char *mname, *out, *tfile;
	timedelta=timescale=0;
	mname=out=tfile=NULL;
	first_signal=last_signal=new_signals = NULL;
	l=NULL;
	first_input_file=last_input_file=NULL;
  while (--argc)
    {
      argv++;
      if (argv [0][0] == '-')
				{
			  if ((strcmp (argv [0], "-o") == 0) ||
			      (strcmp (argv [0], "--output-file") == 0))
			    {
			      if (argc <= 1)
							fatal (1, "-o option must be followed by output filename\n");
			      if (NULL != out)
							fatal (1, "only one output file may be specified\n");
						out=strdup(argv[1]);
			      argc--;
			      argv++;
			    }
			  else if ((strcmp (argv [0], "-ta") == 0) ||
				   (strcmp (argv [0], "--time-file-abs") == 0))
			    {
			      if (argc <= 1)
							fatal (1, "-ta option must be followed by timestamp filename\n");
					
			      if (NULL !=tfile)
							fatal (1, "only one timestamp file may be specified\n");
			      if (timedelta)
							fatal (1, "-ta and -td options are mutually exclusive\n");
						tfile=strdup(argv[1]);
			      argc--;
			      argv++;
			    }
			  else if ((strcmp (argv [0], "-td") == 0) ||
				   (strcmp (argv [0], "--time-delta") == 0))
			    {
			      if (argc <= 1)
							fatal (1, "-td option must be followed by time\n");
			      if (timedelta)
							fatal (1, "only one time delta may be specified\n");
			      if (NULL != tfile)
							fatal (1, "-ta and -td options are mutually exclusive\n");
			      timedelta = parse_time_la2vcd (argv [1], NULL, NULL);
			      argc--;
			      argv++;
			    }
			  else if ((strcmp (argv [0], "-ts") == 0) ||
				   (strcmp (argv [0], "--time-scale") == 0))
			    {
			      if (argc <= 1)
							fatal (1, "-ts option must be followed by time scale\n");
			      if (timescale)
							fatal (1, "only one time scale may be specified\n");
			      if(-1 ==(timescale = parse_time_scale_la2vcd (argv [1])) )
			      	fatal (1,"bad -ts option\n");
			      argc--;
			      argv++;
			    }
			  else if ((strcmp (argv [0], "-r") == 0) ||
				   (strcmp (argv [0], "--radix") == 0))
			    {
			      if (argc <= 1)
							fatal (1, "-r option must be followed by radix\n");
			      next_radix = atoi (argv [1]);
			      if ((next_radix != 2) && (next_radix != 8) && (next_radix != 16))
							fatal (1, "invalid radix\n");
			      argc--;
			      argv++;
			    }
			  else if ((strcmp (argv [0], "-b") == 0) ||
				   (strcmp (argv [0], "--bit-count") == 0))
			    {
			      if (argc <= 1)
							fatal (1, "-b option must be followed by bit count\n");
			      next_bit_count = atoi (argv [1]);
			      if ((next_bit_count < 1) || (next_bit_count > 64))
							fatal (1, "bit count must be in range 1 through 64 inclusive\n");
			      argc--;
			      argv++;
			    }
			  else if ((strcmp (argv [0], "-s") == 0) ||
				   (strcmp (argv [0], "--split") == 0))
			    next_split = true;
			  else if ((strcmp (argv [0], "-sf") == 0) ||
				   (strcmp (argv [0], "--split-field") == 0))
			    {
			      signal_info_t *signal;
			      if (argc <= 2)
							fatal (1, "-sf option must be followed by bit(s) and name \n");
			      signal = split_field (&first_signal,&last_signal,NULL,argv [1], argv [2]);
			      if (! new_signals)
							new_signals = signal;
			      argc -= 2;
			      argv += 2;
			    }
			  else if ((strcmp (argv [0], "-mn") == 0) ||
				   (strcmp (argv [0], "--module-name") == 0))
			    {
			      if (argc <= 1)
							fatal (1, "-mn option must be followed by module name\n");
					  if(NULL != mname)
					  	fatal (1, "-mn option only allowed once\n");
			      mname = strdup(argv [1]);
			      argc--;
			      argv++;
			    }
			  else
			    fatal (1, "unrecognized option '%s'\n", argv [0]);
			}
      else
				{
				  if(add_input_file(&first_input_file, &last_input_file,argv[0],next_radix,next_bit_count,V_WIRE))
				  	fatal(1,"Error Adding Input File\n");
					input_file_info_t *new_input_file =last_input_file;
				  next_radix = 0;
				  next_bit_count = 0;
				  if (next_split)
				    {
				      new_signals = split_signals_from_input (first_signal,last_signal,new_input_file);
				      next_split = false;
				    }
			
				  if (! new_signals)
				    {
							new_signals=_vcd_add_signal (&first_signal,&last_signal,new_input_file, 
								new_input_file->name, V_WIRE, 0,new_input_file->bit_count); //was  new_input_file->bit_count,new_input_file->bit_count
						//fprintf(stderr,"Bug Alert!! This code may not work!!");
				    }
			
				  while (new_signals)
				    {
				      new_signals->input_file = new_input_file;
						printf("Sig %s\n",new_signals->name);
				      if (new_signals->source_mask & ~ ((1ULL << new_input_file->bit_count) - 1))
								fatal (3, "field bit positions out of range %lx %d\n",new_signals->source_mask,new_input_file->bit_count);
				      if (new_signals->source_mask & new_input_file->allocated_mask)
								fprintf (stderr, "warning: bits of input file allocated to multiple signals\n");
				      new_input_file->allocated_mask |= new_signals->source_mask;
				      new_signals = new_signals->next;
				    }
				}
    }
	if(NULL==(l=open_la2vcd(out,tfile,timedelta,timescale,mname)))
		fatal(1,"Unable to init lib\n");
	l->first_input_file=first_input_file;
	l->last_input_file=last_input_file;
	l->first_signal=first_signal;
	l->last_signal=last_signal;
  if (NULL == l->first_input_file)
    fatal (1, "at least one input file file must be specified\n");

  if(-1 == write_vcd_header (l)){
		fatal(3,"VCD Header write failed\n");
	}

  if (l->timef)
    {
      if (! advance_time (l))  // get the first timestamp
				fatal (3, "no data in timestamp file\n");
      l->time_offset = l->current_time;  // first time becomes zero in the output
      l->current_time = 0;            // VCF file, so we use it as time_offset
    }

  write_vcd_data (l);  // we've already got the first set of samples

  while (1)
    {
      advance_time (l);
      if (1 == read_one_line (l))
				write_vcd_data (l);
      else
				break;
    }

	close_la2vcd(l);

  exit (0);
}
