/* Reads current directory for executables and creates
	 hashes of executable files found*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
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
here is NO WARRANTY, to the extent permitted by law. \n
"

void
print_file_stat (const char *file)
{
  struct stat s_file;

  stat (file, &s_file);

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
logfile (char *bin)
{
  FILE *logfile;

  char *rc;
  size_t length;
  char *work_dir;
  char buffer[BUF_SIZE + 1];
  char dir[BUF_SIZE + 1];

  // retrieve current directory to name file
  work_dir = getcwd (buffer, sizeof (buffer));

  if (work_dir == NULL)
    {
      puts ("Unable to fetch directory path");

      exit (EXIT_FAILURE);
    }

  rc = strrchr (work_dir, '/');
  length = strlen (rc);
  memcpy (dir, rc + 1, length);

  snprintf (buffer, sizeof (buffer), "../%s_sums", dir);

  //create file
  logfile = fopen (buffer, "a");
  if (logfile == NULL)          // check if created
    {
      printf ("Unable to create log file");
    }

  fprintf (logfile, "%s\n", bin);
  fclose (logfile);
}

void
file_hash (const char *file, int opt)
{
  unsigned char digest[BUF_SIZE];
  char sum[(BUF_SIZE * 2) + 1];

  if (opt == 'M')
    {
      MD5_CTX c;
      logfile ("\nMD5 Hash Sum");
      logfile ("================");
      MD5_Init (&c);
      MD5_Update (&c, file, BUF_SIZE);
      MD5_Final (digest, &c);
    }
  else
    {
      SHA_CTX c;
      logfile ("\nSHA1 Hash Sum");
      logfile ("=================");
      SHA1_Init (&c);
      SHA1_Update (&c, file, BUF_SIZE);
      SHA1_Final (digest, &c);
    }

  for (int n = 0; n <= 16; ++n)
    snprintf (&(sum[n * 2]), 16 * 3, "%02x", (unsigned int) digest[n]);

  logfile (sum);
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
          if (i + 1 >= buflen)
            {
              buflen *= 2;

              char **old = out;
              out = realloc (out, buflen * sizeof (char));
              if (!out)
                {
                  out = old;
                  free (out);
                  goto fail;
                }
            }

          out[i + 1] = NULL;
          out[i] = malloc (BUF_SIZE * sizeof (char));
          if (!out[i])
            {
              for (char **p = out; *p != NULL; ++p)
                free (*p);

              goto fail;
            }

          strcpy (out[i++], entry->d_name);
        }
    }

  return out;

fail:
  free (out);
  exit (EXIT_FAILURE);
}

int
main (int argc, char *argv[])
{
  int c;
  int long_index = 0;
  char **vals = get_executables ();

  struct option long_opts[] = {
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'v'},
    {"SHA1", no_argument, 0, 'S'},
    {"MD5", no_argument, 0, 'M'},
    {"list", no_argument, 0, 'l'},
    {0, 0, 0, 0}
  };

  c = getopt_long (argc, argv, "hvtSMl", long_opts, &long_index);

  if (c == -1)
    {
      puts (USAGE_INFO);
      puts (VERSION_INFO);

      return 0;
    }

  switch (c)
    {
    case 'S':
    case 'M':
      for (char **v = vals; *v != NULL; ++v)
        {
          file_hash (*v, c);
          free (*v);
        }
      free (vals);
      return 0;
    case 'l':
      for (char **v = vals; *v != NULL; ++v)
        {
          print_file_stat (*v);
          printf ("%s\n", *v);
          free (*v);
        }
      free (vals);
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

  return 0;
}
