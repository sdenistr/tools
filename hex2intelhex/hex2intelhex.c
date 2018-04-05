/*
 *	The protocol taken from:
 *	https://en.wikipedia.org/wiki/Intel_HEX
 */

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <stdint.h>


char* default_file_path = "hex.txt";

char* default_output_file = "intel_hex.HEX";

int main(int argc, char **argv)
{
	char* path_to_file;
	char* path_to_output_file;
	FILE* fptr;
	FILE* fptr_temp;
	char c;
	long pos, past_pos=0;

	long record_length = 0;
	long record_length_bytes = 0;
	long record_address = 0;
	char* record_data;
	long record_type = 0x00;	// Record Type: Data
	int64_t record_check_sum = 0;

	printf("Welcome to hex -> intel_hex script\n\n");
	printf("Note: Input file could be transferred to script as first argument\n");
	printf("Note: The default path for input file: source_directory/hex.txt\n\n");
	printf("Note: Path to output file could be transferred to script as second argument\n");
	printf("Note: The default path for output file: source_directory/intel_hex.HEX\n\n\n");

	// Retrieve the path to input file
	if (argc == 1)
	{
		path_to_file = default_file_path;
	}
	else 
	{
		path_to_file = argv[1];
	}

	// Retrieve the path to output file
	if (argc == 1 || argc == 2)
	{
		path_to_output_file = default_output_file;
	}
	else 
	{
		path_to_output_file = argv[2];
	}


	// open input file
	fopen_s(&fptr, path_to_file,"r+");
	if (fptr == NULL)
	{
		printf("Can't open file");
		_getch();
		exit(2);
	}

	// create output file
	fopen_s(&fptr_temp, path_to_output_file, "w+");
	if(fptr == NULL)
	{
      printf("Can't create file"); 
	  _getch();
      exit(3);             
	}

	// start converting
	pos = ftell(fptr);
	record_length = 0;

	while ((c = getc(fptr)) != EOF)
	{
		// if same record(same line) -> length++
		if (c != '\r' && c != '\n')
		{
			record_length++;
		}
		// if we reached the end of current record
		if (c == '\n')
		{
			// check the record contains legal number of bytes
			if (record_length % 2 != 0)
			{
				printf("Record error: every record must contain even number of digits (every 2 digits represent one byte)");
				_getch();
				exit(4);
			}

			// calculate length in bytes and positions
			record_length_bytes = record_length / 2;
			past_pos = pos;
			pos = ftell(fptr);

			// go to the start of current record
			fseek(fptr, past_pos, SEEK_SET);

			// get the record data
			record_data = (char*)malloc(sizeof(char) * record_length);
			fread(record_data, sizeof(char), record_length, fptr);

			// prepare record format string to write to output file
			char format_str[100];
			sprintf_s(format_str, 100, "%s%d%s", ":%02x%04x%02x%.", record_length, "s%02x\n");

			// calculate checksum
			record_check_sum = record_length_bytes + (record_address & 0xFF) + ((record_address & 0xFF00) >> 8) + record_type;
			for (int i = 0; i < record_length; i+=2)
			{
				int msb = 0;
				if (record_data[i] >= '0' && record_data[i] <= '9')
				{
					msb = (record_data[i] - '0');
				}
				else if (record_data[i] >= 'a' && record_data[i] <= 'f')
				{
					msb = (record_data[i] - 'a' + 10);
				}
				else if (record_data[i] >= 'A' && record_data[i] <= 'F')
				{
					msb = (record_data[i] - 'A' + 10);
				}
				else
				{
					printf("Record error: some hex record seems to have illegal characters");
					_getch();
					exit(4);
				}
				int lsb = 0;
				if (record_data[i+1] >= '0' && record_data[i+1] <= '9')
				{
					lsb = (record_data[i+1] - '0');
				}
				else if (record_data[i+1] >= 'a' && record_data[i+1] <= 'f')
				{
					lsb = (record_data[i+1] - 'a' + 10);
				}
				else if (record_data[i+1] >= 'A' && record_data[i+1] <= 'F')
				{
					lsb = (record_data[i+1] - 'A' + 10);
				}
				else
				{
					printf("Record error: some hex record seems to have illegal characters");
					_getch();
					exit(4);
				}
				record_check_sum += ((msb*16) + lsb);
			}
			record_check_sum = (0x01 + (record_check_sum ^ 0xFF)) % 256;

			// write the record to the output file
			fprintf(fptr_temp, format_str, record_length_bytes, record_address, record_type, record_data, record_check_sum);

			// go to the next record in input file (put cursor to the write place)
			fseek(fptr, pos, SEEK_SET);

			// update variables 
			pos = ftell(fptr);
			record_address += record_length_bytes;
			record_length = 0;
			
			// free allocated space
			free(record_data);
		}
	}

	// Add the terminating record
	fprintf(fptr_temp, ":00000001ff");

	// close the files
	fclose(fptr);
	fclose(fptr_temp);

	// bye bye
	printf("\nScript has been executed successfully\n");
	_getch();
	return 0;
}