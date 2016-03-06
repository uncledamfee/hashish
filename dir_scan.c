/* Reads current directory for executables and creates
	 hashes of executable files */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#define USAGE_INFO "OPTIONS:\n \
-h, --help       Prints help message and exits \n \
-v, --version    Prints version info and exits \n \
-S, --SHA1       Creates SHA1 hash of executables found \n \
-M, --MD5        Creates MD5 hash of executables found \
"

#define VERSION_INFO "hashish 0.0.0 \n \
Copyright (C) 2016 uncledamfee \n \
\n \
License AGPLv3: GNU AGPL version 3 only <http://gnu.org/licenses/agpl.html>. \n \
This is libre software: you are free to change and redistribute it. \n \
here is NO WARRANTY, to the extent permitted by law. \n"

char **
get_executables (void)
{
	DIR *dir;
	size_t buflen = 64;
	dir = opendir("./");
	char **out = malloc (buflen * sizeof (char *));
	if (!out)
		return NULL; // failed to allocate

	out[0] = NULL; // terminator

	size_t i = 0;
	struct dirent *entry;
	while ((entry = readdir (dir)))
	{
		if (access((entry->d_name), F_OK|X_OK) == 0) // implement that later
		{
			if (++i >= buflen)
			{
				buflen *= 2;

				char **old = out;
				out = realloc (out, buflen * sizeof (char *));
				if (!out)
				{
					out = old;
					goto fail;
				}
			}

			out[i + 1] = NULL;
			out[i] = malloc (256 * sizeof (char));
			if (!out[i])
				goto fail;

			strcpy (out[i], entry->d_name);
		}
	}

	return out;

fail:
	for (char **p = out; *p != NULL; ++p)
		free (*p);
	free (out);
	return NULL;
}

char
*sha1_sum(const char *file_name, int file_len)
{
	int n;
	SHA_CTX c;
	unsigned char digest[16];
	char *sum = (char*)malloc(33);

	SHA1_Init(&c);

	while (file_len > 0)
	{
		if (file_len > 512)
		{
			SHA1_Update(&c, file_name, 512);
		}
		else
		{
			SHA1_Update(&c, file_name, file_len);
		}

		file_len  -= 512;
		file_name += 512;

	}
	
	SHA1_Final(digest, &c);

	for (n = 0; n < 16; ++n)
	{
		snprintf(&(sum[n*2]), 16*2, "%02x", (unsigned int)digest[n]);
	}
  
	return sum;
}

char
*md5_sum(const char *file_name, int file_len)
{
	int n;
	MD5_CTX c;
	unsigned char digest[16];
	char *sum = (char*)malloc(33);

	MD5_Init(&c);

	while (file_len > 0)
	{
		if (file_len > 512)
		{
			MD5_Update(&c, file_name, 512);
		}	
		else
		{
			MD5_Update(&c, file_name, file_len);
		}
		
		file_len  -= 512;
		file_name += 512;

	}

	MD5_Final(digest, &c);

	for (n = 0; n < 16; ++n)
	{
		snprintf(&(sum[n*2]), 16*2, "%02x", (unsigned int)digest[n]);
	}

	return sum;
}
/*
void
logfile(char *log)
{
	FILE *logfile;

	logfile = fopen("hash_sums.txt", "w");
	if (logfile == NULL)
	{
		printf ("Unable to create log file");
	}

	fprintf(logfile, "%s", log);
}
*/
int
main(int argc, char *argv[])
{ 
	int i;
	int c;
  char **val;
	 
  val = get_executables();
  
	while (1)
	{
		struct option long_options[] =
		{
			{"help",      no_argument,       0, 'h'},
			{"version",   no_argument,       0, 'v'},
			{"SHA1",      no_argument,       0, 'S'},
			{"MD5" ,      no_argument,       0, 'M'},
			{0, 0, 0, 0} 
		};

		int long_index = 0;

		c = getopt_long (argc, argv, "hvSM",
				long_options, &long_index);

		if (c == -1)
		{ 
			for (char **p = val; *p != NULL; ++p) 
				{
					printf("%s\n", p[i]);
					free(*p);

					++i;
	  		/* *p is each string in the array */ };		
		/*	puts (USAGE_INFO);
			puts (VERSION_INFO);*/
		  return 1;
		}

		switch (c)
		{
			case 'h':
				puts (USAGE_INFO);
				return 0;
			case 'v':
				puts (VERSION_INFO);
				return 0;
			case 'S':
				puts ("SHA1");
				return 0;
			case 'M':
				puts ("MD5");
				return 0;
			case '?':
				return 1;
		}
	}

	free(val); 
}
