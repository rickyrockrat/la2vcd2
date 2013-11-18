/** \file ******************************************************************
\n\b File:        la2vcd_lib.c
\n\b Author:      Initial: Eric Smith <eric@brouhaha.com>
									Library conversion: Doug Springer
\n\b Company:     DNK Designs Inc.
\n\b Date:        01/16/2010  4:02 pm
\n\b Description: 
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

 Rickyrockrat added library build
Change Log: \n

*/

#include "la2vcd2.h"

#define MAX_VCD_SYM_LEN 3
/**from vcd2lxt.c, gtkwave  */


static char *vartypes[]={ "event", "parameter",
                "integer", "real", "real_parameter", "realtime", "reg", "supply0",
                "supply1", "time", "tri", "triand", "trior",
                "trireg", "tri0", "tri1", "wand", "wire", "wor", "port", "in", "out", "inout",
                "$end", "", "", "", ""};
 /************************************************************************/
static void *alloc (size_t size)
{
  void *p;

  p = calloc (1, size);
  if (! p)
    fprintf(stderr,"Memory allocation failed\n");
  return (p);
}
 /************************************************************************/
static char *newstr (char *orig)
{
  int len;
  char *r;

  len = strlen (orig);
  if(NULL ==(r = (char *) alloc (len + 1)) )
  	return NULL;
  memcpy (r, orig, len + 1);
  return (r);
}
 /************************************************************************/
static void strip_whitespace (char *p)
{
  int len = strlen (p);
  char *p2 = p;

  while (len && isspace (p [len - 1]))
    p [--len] = '\0';

  while ((*p2) && isspace (*p2))
    {
      p2++;
      len--;
    }

  if (p != p2)
    memmove (p, p2, len);
}
/************************************************************************/
static int bits_set (uint64_t x)
{
  int count = 0;

  while (x)
    {
      x &= ~ - x;
      count++;
    }

  return count;
}
/************************************************************************/
static int significant_bits (uint64_t x)
{
  int count = 0;
  uint64_t i = 0;

  while (i < x)
    {
      i = (i << 1) + 1;
      count++;
    }
  return count;
}
/************************************************************************/
static uint64_t extract_and_pack_bits (uint64_t src, uint64_t mask)
{
  uint64_t dest = 0;
  uint64_t db = 1;
  uint64_t sb = 1;

  while (mask)
    {
      if (mask & sb)
	{
	  if (src & sb)
	    {
	      dest |= db;
	    }
	  mask &= ~ sb;
	  db <<= 1;
	}
      sb <<= 1;
    }
  return dest;
}
/************************************************************************/
static void fprint_binary (FILE *o, uint64_t val, int bits)
{
  int bit;
  bool leading = true;
  while (--bits >= 0)
    {
      bit = ((val & (1LL << bits)) != 0LL);
      if (leading && (bits != 0) && ! bit)
	continue;
      leading = false;
      fputc ('0' + bit, o);
    }
}
/************************************************************************/
signal_info_t *alloc_new_signal (struct signal_info **first_signal, struct signal_info **last_signal)
{
	signal_info_t *signal;
  if(NULL ==(signal = alloc (sizeof (signal_info_t))) )
  	return NULL;
	/*fprintf(stderr,"Salloc %p ",signal); */
  if (*first_signal)
    ((struct signal_info *)*last_signal)->next = signal;
  else
    *first_signal = signal;
  *last_signal = signal;
	/*fprintf(stderr,"Done\n"); */
  return signal;
}
/************************************************************************/
static char* new_vcd_sym (void)
{
  int i;
  static int chars = 1;
  static char vcd_sym [MAX_VCD_SYM_LEN + 1] = { ' ', 0 };

  for (i = chars - 1; i >= 0; i--)
    {
      if ((++vcd_sym [i]) <= '~')
	break;
      vcd_sym [i] = '!';
    }

  if (i < 0)
    {
      if ((++chars) > MAX_VCD_SYM_LEN){
				fprintf(stderr,"Too many signals for VCD file\n");
				return NULL;
			}
	
      memset (vcd_sym, '!', chars - 1);
    }

  return newstr (vcd_sym);
}
/************************************************************************/
int write_vcd_header (struct la2vcd *l)
{
  time_t t;
  signal_info_t *signal;
  char buf [100];

  t = time (NULL);
  strftime (buf, sizeof (buf), "%b %d, %Y  %H:%M:%S", localtime (& t));

  fprintf (l->outf, "$date\n");
  fprintf (l->outf, "    %s\n", buf);
  fprintf (l->outf, "$end\n");
  fprintf (l->outf, "$version\n");
  fprintf (l->outf, "    TOOL: %s\n",TOSTRING(VERSION));
  fprintf (l->outf, "$end\n");
  fprintf (l->outf, "$time_scale\n");
  fprintf (l->outf, "   %d ns\n",(int)(l->time_scale/(double)(1e-9)));
  fprintf (l->outf, "$end\n");
  fprintf (l->outf, "$scope module %s $end\n", l->module_name);
  for (signal = l->first_signal; signal; signal = signal->next) {
    if(NULL ==(signal->vcd_sym = new_vcd_sym ()) ){
		  return -1;
		}
    if(V_WIRE == signal->var_type)  {
      if (signal->bit_count == 1)
    	fprintf (l->outf, "$var wire 1 %s %s [%d] $end\n",
    		 signal->vcd_sym,
    		 signal->name,
    		 signal->min_index);
          else
    	fprintf (l->outf, "$var wire %d %s %s [%d:%d] $end\n",
    		 signal->bit_count, 
    		 signal->vcd_sym,
    		 signal->name,
    		 signal->max_index,
    		 signal->min_index);
           
    } else if(V_REAL == signal->var_type){
        fprintf (l->outf, "$var real 1 %s %s $end\n",
    		 signal->vcd_sym,
    		 signal->name);
    }
  }  

  fprintf (l->outf, "$upscope $end\n");
  fprintf (l->outf, "$enddefinitions $end\n");
	return 0;
}
/************************************************************************/
static void write_time (struct la2vcd *l)
{
  fprintf (l->outf, "#%ld\n", (int64_t) (l->current_time / l->time_scale));
}
/************************************************************************/
void write_vcd_data (struct la2vcd *l)
{
  static bool first_time = true;
  bool wrote_time = false;
  signal_info_t *signal;

  if (first_time)
    {
      write_time (l);
      fprintf (l->outf, "$dumpvars\n");
    }

  for (signal = l->first_signal; signal; signal = signal -> next) {
      signal->prev_value = signal->value;
      signal->prvalue=signal->rvalue;
    if(V_WIRE ==signal->var_type){
      signal->value = extract_and_pack_bits (signal->input_file->value,
					     signal->source_mask);
      if(first_time || (signal->value != signal->prev_value)) {
  	    if ((! first_time) && (! wrote_time))  {
  	      write_time (l);
  	      wrote_time = true;
  	    }
        if (signal->bit_count == 1)
  	      fprintf (l->outf, "%ld", signal->value & 1);
  	    else	 {
  	      fprintf (l->outf, "b");
  	      fprint_binary (l->outf, signal->value, signal->bit_count);
  	      fprintf (l->outf, " ");
  	    }  
  	    
  	    fprintf (l->outf, "%s\n", signal->vcd_sym);
      }
    }  else if(V_REAL == signal->var_type){
      signal->rvalue=signal->input_file->rvalue;
      if (first_time || (signal->rvalue != signal->prvalue)) {
  	    if ((! first_time) && (! wrote_time))  {
  	      write_time (l);
  	      wrote_time = true;
  	    }
      
          fprintf(l->outf,"r%.16g %s\n",signal->rvalue, signal->vcd_sym);      
      }
	  }
  }

  if (first_time)
    {
      fprintf (l->outf, "$end\n");
      first_time = false;
    }
}
/************************************************************************/
void write_vcd_trailer (struct la2vcd *l)
{
  signal_info_t *signal;

  fprintf (l->outf, "$dumpoff\n");
  for (signal = l->first_signal; signal; signal = signal -> next)
    if (signal->bit_count == 1)
      fprintf (l->outf, " x%s\n", signal->vcd_sym);
    else
      fprintf (l->outf, "bx %s\n", signal->vcd_sym);
  fprintf (l->outf, "$end\n");
}
/************************************************************************/
static int read_timestamp_file_header (struct la2vcd *l)
{
  if (l->timef) {
    char ts_buf [100];
    fgets (ts_buf, sizeof (ts_buf), l->timef);
    if (ferror (l->timef)){
			fprintf(stderr,"error reading timestamp file\n");
			return 1;
		}
	
    if (feof (l->timef)){
			fprintf(stderr,"eof reading timestamp file\n");
			return 1;
		}
	
  }
	return 0;
}
/************************************************************************/
static int read_input_file_header (input_file_info_t *input_file)
{
  fgets (input_file->buf, INPUT_FILE_BUF_SIZE, input_file->inf);
      
  if (ferror (input_file->inf)){
		fprintf(stderr,"error reading input file headers\n");
		return 1;
	}
  if (feof (input_file->inf)){
		fprintf(stderr,"eof while reading input file headers\n");
		return 1;
	}
    

  strip_whitespace (input_file->buf);

  if(NULL ==(input_file->name = newstr (input_file->buf)) ){
		return 1;
	}
	return 0;
}

/************************************************************************/
// returns true if OK (not EOF)
// returns -1 on err, 0 if OK, 1 for EOF
int advance_time (struct la2vcd *l)
{
  int eof = 0;
  l->prev_time = l->current_time;
  if (l->timef)  {
    char ts_buf [100];
    fgets (ts_buf, sizeof (ts_buf), l->timef);
    if (ferror (l->timef)){
			fprintf(stderr,"error reading timestamp file\n");
			return -1;
		}
	
    eof = feof (l->timef);
    if (! eof)
			l->current_time = atof (ts_buf) - l->time_offset;
  }
  else
    l->current_time = l->prev_time + l->time_delta;
  return ! eof;
}

/***************************************************************************/
/** .
\n\b Arguments:
\n\b Returns:
****************************************************************************/
void read_sample(char *buf,input_file_info_t *input_file)
{
	strip_whitespace (buf);

  if(V_WIRE == input_file->var_type ){
    input_file->value = strtoull (buf, NULL, input_file->radix);
    input_file->value &= ((1LL << input_file->bit_count) - 1);  
  } else if( V_REAL == input_file->var_type){
    input_file->rvalue=strtold(buf,NULL);
    /*printf("%e ",input_file->rvalue); */
  }
  
}

/***************************************************************************/
/** Wrapper for read_sample.
\n\b Arguments:
\n\b Returns:
****************************************************************************/
void vcd_read_sample(struct la2vcd *l)
{
	read_sample(l->first_input_file->buf,l->first_input_file);
}

/***************************************************************************/
/** Wrapper for read_sample.
\n\b Arguments:
\n\b Returns:
****************************************************************************/
void vcd_read_sample_real(struct la2vcd *l, double val)
{
  l->first_input_file->rvalue=val;
}
/************************************************************************/
// returns true if OK (not EOF)
static int read_input_file_sample (input_file_info_t *input_file)
{
  fgets (input_file->buf, INPUT_FILE_BUF_SIZE, input_file->inf);
      
  if (ferror (input_file->inf)){
		fprintf(stderr,"error reading input file '%s'\n", input_file->name);
		return -1;
	}
    

  if (feof (input_file->inf))
    return 0;
	read_sample(input_file->buf,input_file);

  return 1;
}

/************************************************************************/
int read_one_line (struct la2vcd *l)
{
  input_file_info_t *input_file;
  int eof_count = 0;
  int non_eof_count = 0;
	int x;

  for (input_file = l->first_input_file; input_file; input_file = input_file -> next){
		x=read_input_file_sample (input_file);
		if(-1 == x)
			return -1;
    if (x)
      non_eof_count++;
    else
      eof_count++;
	}
  	

  if (eof_count && ! non_eof_count)
    return 0;  // done

  if (eof_count){
		fprintf(stderr,"files contain differing line counts\n");
		return -1;
	}
    

  return 1;  // not yet done
}

/************************************************************************/
void close_all_files (struct la2vcd *l)
{
  input_file_info_t *input_file;
  if (NULL != l->outf)
  fclose (l->outf);
  if (l->timef)
    fclose (l->timef);
  for (input_file = l->first_input_file; input_file; input_file = input_file -> next)
  	if(NULL != input_file->inf)
    	fclose (input_file->inf);
}
/************************************************************************/
signal_info_t *split_signals_from_input (signal_info_t *first_signal,signal_info_t *last_signal, input_file_info_t *input_file)
{
  signal_info_t *first_new_signal = NULL;
  int bit;
  for (bit = input_file->bit_count - 1; bit >= 0; bit--) {
		signal_info_t *signal;
    if(NULL ==(signal = alloc_new_signal (&first_signal, &last_signal)) ){
				fprintf(stderr,"split_signals_from_input alloc err\n");
				return NULL;
		}
      	
    signal->bit_count = 1;
    signal->max_index = bit;
    signal->min_index = bit;
    signal->source_mask = 1ULL << bit;
    signal->name = input_file->name;
		signal->input_file=input_file;
    if (! first_new_signal)
			first_new_signal = signal;
  }
  return first_new_signal;
}

/***************************************************************************/
/** .
\n\b Arguments:
\n\b Returns:
****************************************************************************/
signal_info_t *_vcd_add_signal (struct signal_info **first, struct signal_info **last, struct input_file_info *f,char *name, int type, int lsb, int msb) 
{
	signal_info_t *signal;
	if(lsb>63 || lsb <0){
		fprintf(stderr,"Start (%d) out of range (0-63)\n",lsb);
		return NULL;
	}
	if(msb>63 || msb <0){
		fprintf(stderr,"Stop (%d) out of range (0-63)\n",msb);
		return NULL;
	}
	/*fprintf(stderr,"preallco %p %p %p %p ",first,last, *first, *last); */
	signal = alloc_new_signal (first,last);
	/*fprintf(stderr,"Alloc sig\n"); */
  signal->name = name;
	signal->var_type=type;
  if(V_WIRE == type){
    if(lsb==msb){
  		signal->source_mask |= (1ULL << lsb);
  	}else{
  		signal->source_mask |= (((1ULL << (msb+1)) - 1ULL) & ~ ((1ULL << lsb) - 1ULL));
  	}
  	signal->bit_count = bits_set (signal->source_mask);
    signal->max_index = signal->bit_count - 1;
    signal->min_index = 0;  
  }else if(V_REAL == type){
    signal->source_mask=0;
    signal->bit_count = 0;
    signal->max_index = 0;
    signal->min_index = 0;  
  } else{  
    fprintf(stderr,"Can't handle type %d\n",type);
    return NULL;
  }
	
	signal->input_file = f;
	/*printf("%s 0x%16lx\n",signal->name,signal->source_mask); */

  return (signal);
}


/***************************************************************************/
/** .
\n\b Arguments: type is real or logic
\n\b Returns: -1 on err, 0 on success.
****************************************************************************/
int vcd_add_signal (struct la2vcd *l, int type, int msb, int lsb, char *name)
{
  if(NULL ==_vcd_add_signal (&l->first_signal,&l->last_signal, l->last_input_file,name, type, lsb, msb))
    return -1;
  return 0;
}

/************************************************************************/
signal_info_t *split_field (struct signal_info **first, struct signal_info **last,struct input_file_info *f,char *bits, char *name)
{
  signal_info_t *signal;
  unsigned long i, j;

	/*printf("Name= '%s' Bits='%s'\n",name,bits); */
  while (*bits)
    {
      i = strtoul (bits, & bits, 10);
      if ((i < 0) || (i > 63)){
				fprintf(stderr,"bit out of range\n");
				return NULL;
			}
				
      switch (*bits)
			{
			case '-':
			  j = strtoul (bits + 1, & bits, 10);
			  if ((j < 0) || (j > 63)){
					fprintf(stderr,"bit out of range\n");
					return NULL;
				}
			  if (j > i){
					fprintf(stderr,"bit ranges must be from larger index to smaller index\n");
					return NULL;
				}
			    
				signal=_vcd_add_signal(first,last,f,name,V_WIRE,j,i);
				/*printf("%016lx= %ld - %ld\n",signal->source_mask,i,j); */
			  break;
			case ',':
			  bits++;
			  // FALL INTO NEXT CASE
			case '\0':
				signal=_vcd_add_signal(first,last,f,name,V_WIRE,i,i);
				/*printf("Adding %ld  -> %016lx\n",i,signal->source_mask); */
			  break;
			default:
			  fprintf(stderr,"bad character '%c' in bit definition\n", *bits);
				return NULL;
			}
    }


  return (signal);
}
/************************************************************************/
double parse_time_la2vcd (char *p, double *numeric, char **unit)
{
  double t;
  double sc;
  char *p2;

  t = strtod (p, & p2);
  if (t == 0)
    t = 1.0;

  if (strcmp (p2, "fs") == 0)
    sc = 1.0e-15;
  else if (strcmp (p2, "ps") == 0)
    sc = 1.0e-12;
  else if (strcmp (p2, "ns") == 0)
    sc = 1.0e-9;
  else if (strcmp (p2, "us") == 0)
    sc = 1.0e-6;
  else if (strcmp (p2, "ms") == 0)
    sc = 1.0e-3;
  else if (strcmp (p2, "s") != 0){
		fprintf(stderr,"invalid time specification\n");
		return -1;
	}
    

  if (numeric)
    *numeric = t;
  if (unit)
    *unit = p2;

  return (t * sc);
}
/************************************************************************/
double parse_time_scale_la2vcd (char *p)
{
	char *unit;
  double result;
	double numeric;

  result = parse_time_la2vcd (p, &numeric, &unit);
  if ((numeric != 1.0) && (numeric != 10.0) && (numeric != 100.0)){
		fprintf(stderr,"time scale numeric part must be 1, 10, or 100\n");
		return -1;
	}
    
  return (result);
}
 
/***************************************************************************/
/** .
\n\b Arguments:
\n\b Returns:
****************************************************************************/
void fix_name(char *n)
{
	while(*n){
		if(' '== *n)
			*n='_';
		++n;
	}
}
/***************************************************************************/
/** .
\n\b Arguments:
out - out put file name, required. Use - for stdout??

\n\b Returns:
****************************************************************************/
struct la2vcd *open_la2vcd (char *out, char *timefile, double timedelta, double timescale, char *mname)
{
	struct la2vcd *l;
	if(NULL == (l=malloc(sizeof(struct la2vcd)))){
		fprintf(stderr,"Out of Mem for la2vcd struct\n");
		return NULL;
	}
	memset(l,0,sizeof(struct la2vcd));
	
	if (0 == timescale)
    l->time_scale = 1.0e-9;	
	else
		l->time_scale=timescale;
	l->time_delta=timedelta;
	if(NULL == out){ /**use *fdopen(1, "w"); for stdout? */
		l->outf = stdout;
		fprintf(stderr,"Using StdOut\n");
	}else{
		l->outf = fopen (out, "w");
     if (! l->outf){
			fprintf(stderr,"can't open output file '%s'\n",out);	
			goto err1;
		}
	}
	if(NULL != timefile){
		if(l->time_delta){
			fprintf(stderr,"time delta and time file are mutually exclusive\n");
		  goto err2;
		}
		l->timef = fopen (timefile, "r");
    if (NULL == l->timef){
			fprintf(stderr,"can't open timestamp file '%s'\n",timefile);
			goto err2;
		}	else
    	if(read_timestamp_file_header (l))
    		goto err2;
	} 
	if (NULL == timefile && 0 == timedelta){
		fprintf(stderr,"either -ta or -td option must be given (no timefile & timedelta=0)\n");
		goto err3;
	}
    
	if(NULL == mname)
		l->module_name="logic_analyzer";
	else{
		
		l->module_name=strdup(mname);
		fix_name(l->module_name);
	}
		
	return l;	
	
err3:
	if(NULL != l->timef)
		fclose(l->timef);
err2:
	if(NULL != l->outf)
		fclose(l->outf);
err1:
	free(l);
	return NULL;
	
}
/***************************************************************************/
/** .
\n\b Arguments:
\n\b Returns:
****************************************************************************/
int add_input_file(struct input_file_info **first, struct input_file_info **last, char *in, int radix, int bits, int type)
{
	int err=0;
	input_file_info_t *file_info=NULL;
	
	if ((radix != 2) && (radix != 8) && (radix != 16)){
		fprintf(stderr,"Invalid radix (%d). Must be 2,8,or 16\n",radix);
		++err;
	}
	if ((bits < 1) || (bits > 64)){
		fprintf(stderr,"bits must be between 1 and 64 inclusive\n");
		++err;
	}
	if(err)
		return err;
	if(NULL == (file_info=malloc(sizeof (input_file_info_t)))){
		fprintf(stderr,"Out of Mem for file info\n");
		return -1;
	}
	memset(file_info,0,sizeof (input_file_info_t));
	file_info->radix=radix?radix:2;
	file_info->bit_count =bits;
  file_info->var_type=type;
	
	if(NULL != in){
		file_info->inf = fopen (in, "r");
		if (! file_info->inf){
			fprintf(stderr,"Can't open input file %s\n",in);
			free (file_info);
			return -1;
		}
		file_info->buf = alloc (INPUT_FILE_BUF_SIZE);
		if(read_input_file_header (file_info)){
			fprintf(stderr,"Err reading input file header\n");
			goto err;
		}
		if (! read_input_file_sample (file_info)){
			fprintf(stderr,"No data in input file '%s'\n",in);
			goto err;
		}
	  if (! file_info->bit_count)
	    file_info->bit_count = strlen (file_info->buf) * significant_bits (file_info->radix - 1);
	}
	if (*first)
		((struct input_file_info *)*last)->next = file_info;
	else
		*first = file_info;
	*last = file_info;
	return 0;
err:
	free(file_info->buf);
	free (file_info);
	return -1;
}

/***************************************************************************/
/** .
\n\b Arguments:
\n\b Returns:
****************************************************************************/
int vcd_add_file(struct la2vcd *l, char *in, int radix, int bits, int type)
{
	static int first=1;
	
	if(add_input_file(&l->first_input_file, &l->last_input_file,in,radix,bits,type))
		return 1;
	/*fprintf(stderr,"Added input file\n"); */
	if(0 && first){
		split_signals_from_input (NULL,NULL,l->last_input_file);
		first=0;
		/*fprintf(stderr,"Split sigs\n"); */
	}
		
	return 0;
}
/***************************************************************************/
/** .
\n\b Arguments:
\n\b Returns:
****************************************************************************/
void close_la2vcd(struct la2vcd *l)
{
  write_vcd_trailer (l);
	close_all_files (l);
}

