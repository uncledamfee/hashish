/* Reads current directory for executables and creates
	 hashes of executable files found*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <getopt.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#define BUF_SIZE 512

#define USAGE_INFO "OPTIONS:\n \
-h, --help       Prints help message and exits \n \
-v, --version    Prints version info and exits \n \
-l, --list       Prints information of all found binaries to stdout \n \
-S, --SHA1       Creates SHA1 hash of executables found \n \
-M, --MD5        Creates MD5 hash of executables found \
"

#define VERSION_INFO "hashish 0.0.0 \n \
Copyright (C) 2016 uncledamfee \n \
\n \
License AGPLv3: GNU AGPL version 3 only <http://gnu.org/licenses/agpl.html>. \n \
This is libre software: you are free to change and redistribute it. \n \
here is NO WARRANTY, to the extent permitted by law. \n"

// TODO:
// - FIX VALGRIND ERRORS

void
print_file_stat(const char *file)
{
	struct stat s_file;

	stat(file, &s_file); 
		
	printf ((s_file.st_mode & S_IRUSR) ? "r" : "-");
	printf ((s_file.st_mode & S_IWUSR) ? "w" : "-");
	printf ((s_file.st_mode & S_IXUSR) ? "x" : "-");
	printf ((s_file.st_mode & S_IRGRP) ? "r" : "-");
	printf ((s_file.st_mode & S_IWGRP) ? "w" : "-");
	printf ((s_file.st_mode & S_IXGRP) ? "x" : "-");
	printf ((s_file.st_mode & S_IROTH) ? "r" : "-");
	printf ((s_file.st_mode & S_IWOTH) ? "w" : "-");
	printf ((s_file.st_mode & S_IXOTH) ? "x" : "-");
	printf (" %zu ", s_file.st_size);
	printf (" %zu ", s_file.st_nlink);
	printf (" %zu ", s_file.st_ino);
} 


void
logfile (char *exe)
{
  FILE *logfile;

  char *work_dir;
  char *rc;
  char buffer[BUF_SIZE + 1];
  char *directory;
  size_t length;

  work_dir = getcwd (buffer, BUF_SIZE + 1);
  rc = strrchr (work_dir, '/');

  if (rc == NULL || work_dir == NULL)
    {
      puts ("Unable to fetch directory path");

      exit (EXIT_FAILURE);
    }

  length = strlen (rc);
  directory = malloc (length);
  memcpy (directory, rc + 1, length);

  snprintf (buffer, sizeof (buffer), "%s_sums", directory);
  logfile = fopen (buffer, "a");        // create file (append if exists)
  if (logfile == NULL)          // check if created
    {
      printf ("Unable to create log file");
    }

  fprintf (logfile, "%s\n", exe);
  fclose (logfile);
  free (directory);
}

void *
file_hash (const char *file, int opt)
{
  MD5_CTX c;
  SHA_CTX z;
  unsigned char digest[BUF_SIZE];
  char *sum = malloc (BUF_SIZE * sizeof (char *));

  if (opt == 'M')
    {
      logfile ("\nMD5 Hash Sums");
      logfile ("================");
      MD5_Init (&c);
      MD5_Update (&c, file, BUF_SIZE);
      MD5_Final (digest, &c);
    }
  else
    {
      logfile ("\nSHA1 Hash Sums");
      logfile ("=================");
      SHA1_Init (&z);
      SHA1_Update (&z, file, BUF_SIZE);
      SHA1_Final (digest, &z);
    }

  for (int n = 0; n < 16; ++n)
    snprintf (&(sum[n * 2]), 16 * 2, "%02x", (unsigned int) digest[n]);

  logfile (sum);
  free (sum);
}

char **
get_executables (void)
{
  DIR *dir;
  dir = opendir ("./");

  size_t buflen = BUF_SIZE;

  struct stat fc;
  char **out = malloc (buflen * sizeof (char *));
  if (!out)
    exit (EXIT_FAILURE);        // failed to allocate

  out[0] = NULL;                // terminator

  size_t i = 0;
  struct dirent *entry;
  while ((entry = readdir (dir)) != NULL)
    {
      stat (entry->d_name, &fc);

			// only files from working directory plz
      if (strcmp (entry->d_name, ".") == 0 ||
          strcmp (entry->d_name, "..") == 0 || S_ISREG (fc.st_mode) == 0)
        continue;

      if (access ((entry->d_name), F_OK | X_OK) == 0)   // checks if executable
        {
          if (++i >= buflen)
            {
              buflen *= 2;

              char **old = out;
              out = realloc (out, buflen * sizeof (char *));
              if (!out)
                {
                  out = old;
                  free (out);
                  goto fail;
                }
            }

          out[i + 1] = NULL;
          out[i] = malloc (BUF_SIZE * sizeof (char *));
          if (!out[i])
            {
              for (char **p = out; *p != NULL; ++p)
                free (*p);

              free (out);
            }

          strcpy (out[i], entry->d_name);
        }
    }

  return out;
  free (out);

fail:
  exit (EXIT_FAILURE);
}

int
main (int argc, char *argv[])
{
  int c;
  char **val = get_executables ();

  while (1)
    {
      struct option long_opts[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"SHA1", no_argument, 0, 'S'},
        {"MD5", no_argument, 0, 'M'},
        {"list", no_argument, 0, 'l'},
        {0, 0, 0, 0}
      };

      int long_index = 0;

      c = getopt_long (argc, argv, "hvtSMl", long_opts, &long_index);

      if (c == -1)
        {
          puts (USAGE_INFO);
          puts (VERSION_INFO);

          return 1;
        }

      switch (c)
        {
        case 'S':
          for (int d = 0; d < BUF_SIZE + 1; ++d)
            {
              if (val[d] != NULL)
                {
                  file_hash (val[d], c);
                }
            }
          free (val);
          return 0;
        case 'M':
          for (int d = 0; d < BUF_SIZE + 1; ++d)
            {
              if (val[d] != NULL)
                {
                  file_hash (val[d], c);
                }
            }
          free (val);
          return 0;
        case 'l':
          for (int d = 0; d < BUF_SIZE + 1; ++d)
            {
              if (val[d] != NULL)
							{
									print_file_stat(val[d]);
                  printf ("%s\n", val[d]);
							}
            }
          free (val);
          return 0;
        case 'h':
          puts (USAGE_INFO);
          return 0;
        case 'v':
          puts (VERSION_INFO);
          return 0;
        case '?':
          return 0;
        }
    }
}
