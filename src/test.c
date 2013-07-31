#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>

#include "process.h"
#include "vec.h"
#include "list.h"
#include "var.h"
#include "sen-data.h"
#include "proof.h"
#include "aio.h"
#include "rules.h"
#include "config.h"
#include "interop-isar.h"
#include "sexpr-process.h"
#include "menu.h"


static char *
safe_read (int fd)
{
  /*
  int size = 0;
  char * ret = (char *) malloc (sizeof (char));
  CHECK_ALLOC (ret, NULL);
  size_t read_chk;
  read_chk = read (fd, ret + size, 1);
  //fprintf (stderr, "%c", *(ret + size));
  size++;
  ret[size] = '\0';
  while (read_chk)
    {
      printf ("%i\n", size);
      ret = (char *) realloc (ret, sizeof(char) * (size + 1));
      CHECK_ALLOC (ret, NULL);
      read_chk = read (fd, ret + size, 1);
      //fprintf (stderr, "%c", *(ret + size));
      size++;
      ret[size] = '\0';
    }
  */
  off_t offset, current;
  current = lseek (fd, 0, SEEK_CUR);
  offset = lseek (fd, 0, SEEK_END);
  printf ("%li\n", offset);
  lseek (fd, current, SEEK_SET);

  char * buffer;
  buffer = (char *) calloc (offset + 1, sizeof (char));
  CHECK_ALLOC (buffer, NULL);

  read (fd, buffer, offset);
  buffer[offset] = '\0';
  
  return buffer;
}

int
start_parse (unsigned char * in_str)
{
  unsigned char * car;

  car = sexpr_car (in_str);
  if (!car)
    return -1;

  // func = process_entry (car);
  // func(cdr(in_str));

  return 0;
}

int
main (int argc, char * argv[])
{
  /*
  unsigned char * test;

  test = argv[1];

  unsigned char * car, * cdr;

  car = sexpr_car (test);
  cdr = sexpr_cdr (test);

  printf ("'%s','%s'\n", car, cdr);

  free (car);
  free (cdr);
  */

  /*
  GPid new_pid;
  int sout, sin, serr;
  GError * g_err;

  g_spawn_async_with_pipes (ISABELLE_PATH,
			    (gchar **) is_args,
			    NULL,
			    0,
			    NULL, NULL,
			    &new_pid,
			    &sin, &sout, &serr,
			    &g_err);

  fd_set read_set, write_set;
  FD_ZERO (&read_set);
  FD_ZERO (&write_set);
  FD_SET (sin, &write_set);
  FD_SET (sout, &read_set);
  */

  // Use a similar manner to most servers to check when this is ready to be written to.
  // Once ready, write commands to sin, and read the response from sout.

  /*
  int i;
  int have_written = 0;

  FILE * in_stream, * out_stream;
  in_stream = fdopen (sin, "w");
  fprintf (in_stream, "help;\r\n");
  fclose (in_stream);

  out_stream = fdopen (sout, "r");
  char c;
  while ((c = fgetc(out_stream)) != EOF)
    putchar(c);
  fclose (out_stream);
  */
  

  while (1)
    {
      break;
      /*
      select (FD_SETSIZE, &read_set, &write_set, NULL, NULL);
      if (FD_ISSET (sin, &write_set) && !have_written)
	{
	  write (sin, "help;\r\n", 7);
	  sleep (5);
	  char buffer[10001];
	  size_t ret_chk;
	  ret_chk = read (sout, buffer, 10000);
	  buffer[ret_chk] = '\0';

	  char * buffer;
	  buffer = safe_read (sout);

	  printf ("%s\n", buffer);

	  break;
	}
      */

      /*
      if (FD_ISSET (sout, &read_set) && have_written)
	{
	  char buffer[8];
	  size_t ret_chk;
	  ret_chk = read (sout, buffer, 7);
	  buffer[ret_chk] = '\0';
	  printf ("%s\n", buffer);
	  break;
	}
      */
      /*
      if (FD_ISSET (sout, &file_set))
	{
	  break;
	}
      */
    }

  unsigned char * output;
  //unsigned char * inputs[] = {"help", "quit"};
  unsigned char * inputs[] = {"theory Basic_Logic imports Main begin",
			      "lemma I: \"A --> A\"",
			      "proof",
			      "assume A",
			      "show A by fact",
			      "qed",
			      "print_theorems",
			      "end",
			      "quit"};
			      

  isar_run_cmds (inputs, &output);
  printf ("%s\n", output);

  /*
  int init_pos, fin_pos;

  fin_pos = parse_tags (test, 0, &car, "(", ")");

  printf ("'%i' - '%s'\n", fin_pos, car);

  free (car);
  */

  return 0;
}
