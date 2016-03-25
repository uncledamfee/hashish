/* Reads current directory for executables and creates
	 hashes of executable files */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
//#include <openssl/md5.h>
//#include <openssl/sha.h>

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

int
get_executables (void)
{
	DIR *dir;
	dir = opendir("./");

	size_t buflen = 512;
	long bufsize;

	struct stat fc;
	char **out = malloc (buflen * sizeof (char *));
	if (!out)
		exit(EXIT_FAILURE); // failed to allocate

	out[0] = NULL; // terminator
	
	
	size_t i = 0;
	struct dirent *entry;
	while ((entry = readdir (dir)) != NULL)
	{
		stat(entry->d_name, &fc);

		if (strcmp(entry->d_name, ".") == 0 ||
				strcmp(entry->d_name, "..") == 0 ||
				S_ISREG(fc.st_mode) == 0)
				continue;
			
		if (access((entry->d_name), F_OK|X_OK) == 0 & entry->d_name != NULL) // implement that later
		{
			if (++i <= buflen)
			{
				buflen *= 2;
				printf("%ld", buflen);

				char **old = out;
				out = realloc (out, buflen * sizeof (char *));
				if (!out)
				{
					out = old;
					goto fail;
				}

				bufsize = buflen;
			}

			out[i + 1] = NULL;
			out[i] = malloc (256 * sizeof (char *));
			if (!out[i])
			{
				for (char **p = out; *p != NULL; ++p)
					free (*p);

				free (out);
			}

			strcpy (out[i], entry->d_name);
		}
	}

	//int p = 0;
	
	for (int d = 0; d <= buflen + 1;)
	{
		if (out[d] != NULL)
		{
			printf("\n|%s|%d|%ld", out[d], d, bufsize);

			++d;
		//	++p;
		}
		else if (out[d] == NULL)
		{
			++d;
		//	++p;
		}
		else
			d = bufsize + 1;

	}
	
	free(out);

fail:
	exit(EXIT_FAILURE);
}

int
main (int argc, char *argv[])
{
	int c;

	while (1)
	{
		struct option long_opts[] =
		{
			{"help",	no_argument, 0, 'h'},
			{"version", no_argument, 0, 'v'},
			{0, 0, 0, 0}
		};

		int long_index = 0;

		c = getopt_long (argc, argv, "hv", 
							long_opts, &long_index);

		if (c == -1)
		{
			puts (USAGE_INFO);
			puts (VERSION_INFO);

			return 1;
		}

		switch (c)
		{
			case 'h':
				get_executables();
				return 0;
			case 'v':
				puts (VERSION_INFO);
				return 0;
			case '?':
				return 0;
		}
	}
}







