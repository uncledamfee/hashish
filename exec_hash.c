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
#include <getopt.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#define BUF_SIZE 256
#define USAGE_INFO "OPTIONS:\n \
-h, --help       Prints help message and exits \n \
-v, --version    Prints version info and exits \n \
-t, --test       Used to test in directories (TEMPORARY) \n \
"
//-S, --SHA1       Creates SHA1 hash of executables found \n \
//-M, --MD5        Creates MD5 hash of executables found \


#define VERSION_INFO "hashish 0.0.0 \n \
Copyright (C) 2016 uncledamfee \n \
\n \
License AGPLv3: GNU AGPL version 3 only <http://gnu.org/licenses/agpl.html>. \n \
This is libre software: you are free to change and redistribute it. \n \
here is NO WARRANTY, to the extent permitted by law. \n"

// TODO: 
// - Add function for SHA and MD5 hash sums
// - Add dynamic fopen file naming (ex. <working directory>_sums)
// - FIX VALGRIND ERRORS

void
logfile(char *sum)
{
	FILE *logfile;

	char *work_dir;
	char *rc;
	char buffer[BUF_SIZE+1];
	char *directory;
	size_t length;

	work_dir = getcwd(buffer, BUF_SIZE+1);
	rc = strrchr(work_dir,'/');

	if (rc == NULL || work_dir == NULL)
	{
		puts("Unable to fetch directory path");

		exit(EXIT_FAILURE);
	}

	length = strlen(rc);
	directory = malloc(length);
	memcpy(directory, rc + 1, length);
 	
	//puts(directory);
	free(directory);

	logfile = fopen("hash_sums", "a"); // create file (append if exists)
	if (logfile == NULL)	// check if created
	{
		printf ("Unable to create log file");
	}
	
	fprintf(logfile, "%s\n", sum);
} 

void
*sha1(const char *file)
{
	int file_len = sizeof(file);
	SHA_CTX c;
	unsigned char digest[16];
	char *sum = malloc(BUF_SIZE * sizeof(char *));

	SHA1_Init(&c);

	while (file_len > 0)
	{
		if (file_len > BUF_SIZE)
			SHA1_Update(&c, file, BUF_SIZE);
		else
			SHA1_Update(&c, file, file_len);

		file_len -= BUF_SIZE;
		file += BUF_SIZE;
	}

	SHA1_Final(digest, &c);

	for (int n = 0; n < 16; ++n)
		snprintf(&(sum[n*2]), 16*2, "%02x", (unsigned int) digest[n]);

  logfile(sum);
	free(sum);
}
	
int
get_executables (void)
{
	DIR *dir;
	dir = opendir("./");

	size_t buflen = BUF_SIZE;

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

		if (access((entry->d_name), F_OK|X_OK) == 0) // checks if executable
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

	for (int d = 0; d < buflen + 1; ++d)
	{
		if (out[d] != NULL)
		{
			printf("%s\n", out[d]);
			sha1(out[d]);
		}
	}

free(out);

fail:
	exit(EXIT_FAILURE);
	free(out);
}

int
main (int argc, char *argv[])
{
	int c;

	while (1)
	{
		struct option long_opts[] =
		{
			{"help",		no_argument, 0, 'h'},
			{"version",	no_argument, 0, 'v'},
			{"test",	no_argument, 0, 't'},
			{0, 0, 0, 0}
		};

		int long_index = 0;

		c = getopt_long (argc, argv, "hvt", 
										long_opts, &long_index);

		if (c == -1)
		{
			puts (USAGE_INFO);
			puts (VERSION_INFO);

			return 1;
		}

		switch (c)
		{
			case 't':
				get_executables();
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
