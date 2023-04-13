#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// tmp
#include <math.h>

#if defined(_WIN32)
#include <Windows.h>
#undef max
#undef RGB
#else //< #elif defined(__linux__)
#include <sys/time.h>
#endif

#include <arguments_mini.h>
#include <lcg_mini.h>
#include <turbojpeg.h>


float ceil$float(float a)
{
	// tmp
	return (float)ceil((double)a);
}

void usage()
{
	char a[] = "usage..\n"
		"lcg(.exe)\n"
		" ^\n"
		" print to stdout\n"
		"OR\n"
		"lcg(.exe) <filename>.jpg\n"
		" ^\n"
		" export to <filename>.jpg\n"
		"parameters..\n"
		".. -numIterations=<int>\n"
		"   ^\n"
		"   optional, defaults to 1\n"
		".. -seed=<0 and 2^63 both result in the seed 2^63, uint64_t>\n"
		"   ^\n"
		"   optional, defaults to the current timestamp which is guaranteed to change atleast every microsecond\n"
		"parameters for \"lcg.exe <filename>.jpg\"..\n"
		".. --width=<in pixels, int>\n"
		"   ^\n"
		"   optional, defaults to 1\n";
	fputs(a, stdout);
}

// TODO: void my_on_print(char* a, FILE* b)
//                                 ^
//                                 stdout/stderr
//       ..?
void my_on_print(char* a)
{
	// "error:"
	if((strlen(a) >= 6) && (memcmp("error:", a, 6) == 0))
	{
		fputs(a, stderr);
	}
	else
	{
		fputs(a, stdout);
	}
}

int bUsage = 0;
char* jpgfilename = NULL;
// TODO: add note to arguments_mini.h that argument is char* into argv thus..
//       .. won't be deallocated..?
int my_on_argument_parsed(char* argument)
{
	if(strcmp("usage", argument) == 0)
	{
		bUsage = 1; //< duplicate argument not allowed by arguments-mini thus this works
	}
	else
	{
		int a = strlen(argument);
		if(a >= 4 && memcmp(&argument[a-4], ".jpg", 4) == 0)
		{
			if(jpgfilename != NULL)
			{
				fputs("error: more than <filename>.jpg supplied\n", stderr);
				return 0;
			}
			
			jpgfilename = argument; //< works as argument is pointer into argv
		}
		else
		{
			fprintf(stderr, "error: invalid argument \"%s\"\n", argument);
			return 0;
		}
	}
	
	return 1;
}

int numIterations = 1;
int bUseCustomSeed = 0;
uint64_t customSeed;
int bUseCustomWidth = 0;
int customWidth;
int my_on_parameterwithvalue_parsed(char* parametername, char* value)
{
	if(strcmp(parametername, "numIterations") == 0)
	{
		numIterations = atoi(value);
	}
	else if(strcmp(parametername, "seed") == 0)
	{
		bUseCustomSeed = 1;
		customSeed = strtoull(value, NULL, 10);
	}
	else if(strcmp(parametername, "width") == 0)
	{
		bUseCustomWidth = 1;
		customWidth = atoi(value);
	}
	else
	{
		fprintf(stderr, "error: unknown parameter -%s=%s\n", parametername, value);
		return 0;
	}
	
	return 1;
}

enum
{
	EJpgfileFormat_RGB = 0,
	EJpgfileFormat_Grayscale
};
int add_jpgfile$char$$unsignedchar$$int$int$int(char* filename, unsigned char* pixels, int width, int height, int format)
{
	tjhandle a = tjInitCompress();
	if(a == NULL)
	{
		fprintf(stderr, "error: %s\n", tjGetErrorStr2(NULL));
		return 0;
	}
	
	struct
	{
		int a;
	} tjsamp;
	struct
	{
		int a;
	} tjpf;
	if(format == EJpgfileFormat_Grayscale)
	{
		tjsamp.a = TJSAMP_GRAY;
		tjpf.a = TJPF_GRAY;
	}
	else
	{
		tjsamp.a = TJSAMP_444;
		tjpf.a = TJPF_RGB;
	}

	unsigned long b = tjBufSize(width, height, tjsamp.a);
	if(b == -1)
	{
		fprintf(stderr, "error: tjBufSize(width, height, %s) == -1\n", format == EJpgfileFormat_Grayscale ? "TJSAMP_GRAY" : "TJSAMP_444");
		return 0;
	}
	//unsigned char c[b];
	// NOTE: tjCompress2 failed w. "Buffer passed to JPEG library is too..
	//       .. small" if using e directly
	//       v
	//unsigned char* d = c;
	// ^
	// possible but don't to prevent stack overflow..?
	//unsigned char c = new char[b];
	unsigned char* c = malloc(b);
	
	if(tjCompress2(a, pixels, width, 0, height, tjpf.a, &c, &b, tjsamp.a, 100, TJFLAG_NOREALLOC) == -1)
	{
		fprintf(stderr, "error: %s\n", tjGetErrorStr2(a));
		return 0;
	}
	
	if(tjDestroy(a) == -1)
	{
		printf("warning %s\n", tjGetErrorStr2(a));
	}
	
	FILE* d = fopen(filename, "wb");
	if(d == NULL)
	{
		fprintf(stderr, "error: fopen(\"%s\", \"w\") == NULL", filename);
		return 0;
	}

	fwrite(c, 1, b, d);
	
	free(c);
	
	fclose(d);
	
	return 1;
}
int add_jpgfile$char$$int$unsignedchar$$int$int$int(char* filename, int numPixels, unsigned char* pixels, int width, int height, int format)
{
	unsigned char* a = pixels;
	if(numPixels < width * height)
	{
		//a = format == EJpgfileFormat_Grayscale ? new unsigned char[width * height] : new unsigned char[width * height * 3];
		a = (unsigned char*)(format == EJpgfileFormat_RGB ? malloc(width * height) : malloc(width * height * 3));
		
		for(int i = 0; i < width * height; ++i)
		{
			int index = format == EJpgfileFormat_Grayscale ? i : i * 3;
			if(i >= numPixels)
			{
				switch(format)
				{
				case EJpgfileFormat_Grayscale:
					a[index] = pixels[index];
					break;
				case EJpgfileFormat_RGB:
					a[index + 0] = pixels[index + 0];
					a[index + 1] = pixels[index + 1];
					a[index + 2] = pixels[index + 2];
					break;
				}
			}
			else
			{
				switch(format)
				{
				case EJpgfileFormat_Grayscale:
					a[index] = pixels[index];
					break;
				case EJpgfileFormat_RGB:
					a[index + 0] = 0xff;
					a[index + 1] = 0xff;
					a[index + 2] = 0xff;
					break;
				}
			}
		}
	}

	int b = add_jpgfile$char$$unsignedchar$$int$int$int(filename, a, width, height, format);
	if(numPixels < width * height)
	{
		free(a);
	}
	if(b != 1)
	{
		return b;
	}

	return 1;
}
int add_jpgfile$char$$int$double$$int$int$int(char*, int, double*, int, int, int);
int add_jpgfile$char$$double$$int$int$int(char* filename, double* pixels, int width, int height, int format)
{
	return add_jpgfile$char$$int$double$$int$int$int(filename, width * height, pixels, width, height, format);
}
int add_jpgfile$char$$int$double$$int$int$int(char* filename, int numPixels, double* pixels, int width, int height, int format)
{
	//unsigned char* a = format == EJpgfileFormat_Grayscale ? new char[height][width] : new	char[height][width][3];
	unsigned char* a = format == EJpgfileFormat_Grayscale ? malloc(height * width) : malloc(height * width * 3);
	for(int i = 0; i < height; ++i)
	{
		for(int j = 0; j < width; ++j)
		{
			int sourceindex = i * width + j;
			int targetindex = format == EJpgfileFormat_Grayscale ? sourceindex : i * width * 3 + j * 3; 
			if(sourceindex >= numIterations)
			{
				switch(format)
				{
				case EJpgfileFormat_Grayscale:
					a[targetindex] = 0xff;
					break;
				case EJpgfileFormat_RGB:
					a[targetindex + 0] = 0xff;
					a[targetindex + 1] = 0xff;
					a[targetindex + 2] = 0xff;
					break;
				}

				continue;
			}
			
			unsigned char pixel = pixels[sourceindex] * 0xff;
			
			switch(format)
			{
			case EJpgfileFormat_Grayscale:
				a[targetindex] = pixel;
				break;
			case EJpgfileFormat_RGB:
				a[targetindex + 0] = pixel;
				a[targetindex + 1] = pixel;
				a[targetindex + 2] = pixel;
				break;
			}
		}
	}
	
	int b = add_jpgfile$char$$unsignedchar$$int$int$int(filename, a, width, height, format);
	free(a);
	if(b != 1)
	{
		return b;
	}
	
	return 1;
}

int main(int argc, char** argv)
{
	am_set_on_print(my_on_print);
	am_set_on_argument_parsed(my_on_argument_parsed);
	am_set_on_parameterwithvalue_parsed(my_on_parameterwithvalue_parsed);
	
	int a = am_parse(argc, argv);
	am_unset_on_print();
	am_unset_on_argument_parsed();
	am_unset_on_parameterwithvalue_parsed();
	if(a == 0)
	{
		return 1;
	}
	
	if(bUsage == 1)
	{
		if(argc > 2)
		{
			fputs("error: too many arguments and/or parameter(s) supplied with argument usage\n", stderr);
			return 1;
		}
		
		usage();
		return 0;
	}
	
	if((bUseCustomWidth == 1) & (jpgfilename == NULL))
	{
		fprintf(stderr, "error: unknown parameter -width=%i\n", customWidth);
		return 1;
	}
	
	int seed;
	if(bUseCustomSeed == 1)
	{
		seed = customSeed;
	}
	else
	{
		uint64_t b; //< nanoseconds
		
#if defined(_WIN32)
		FILETIME c;
		GetSystemTimeAsFileTime(&c);
		ULARGE_INTEGER d = { .u.LowPart = c.dwLowDateTime, .u.HighPart = c.dwHighDateTime };
		// ^
		// (d.QuadPart)100nanoseconds
		// ^
		// https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-filetime
		b = d.QuadPart * 100ull;
#else //< #elif defined(__linux__)
		timeval c;
		gettimeofday(&c, NULL);
		b = (((unsigned long long)c.tv_sec) * 1000000ull + ((unsigned long long)c.tv_usec)) * 1000ull;
		//  ^
		//  microseconds
		//  ^
		//  https://man7.org/linux/man-pages/man0/sys_time.h.0p.html
#endif

		seed = b;
	}
	
	int myGenerator;
	lm_add_generator(seed, &myGenerator);
	
	//double samples[numIterations];
	// ^
	// don't to prevent stack overflow
	//double* samples = new double[numIterations];
	double* samples = malloc(sizeof(double) * numIterations);
	for(int i = 0; i < numIterations; ++i)
	{
		uint64_t b;
		lm_generate_next(myGenerator, &b);
		samples[i] = b / (double)0xffffffffffffffffull;
	}
	
	lm_remove_generator(myGenerator);
	
	if(jpgfilename == NULL)
	{
		for(int i = 0; i < numIterations; ++i)
		{
			printf("%f\n", samples[i]);
		}
	}
	else
	{
		int width;
		int height;
		if(bUseCustomWidth == 1)
		{
			width = customWidth;
			height = (int)ceil$float(numIterations / (float)width);
		}
		else
		{
			width = numIterations;
			height = 1;
		}
		
		//printf("width %i, height %i\n", width, height);
		
//#define GRAY
#define RGB
#if defined(GRAYSCALE) & defined(RGB)
//#elif ~(defined(GRAYSCALE) | defined(RGB))
#error GRAYSCALE or RGB may be defined, but not both
#elif !(defined(GRAYSCALE) | defined(RGB))
#error either GRAYSCALE or RGB must be defined
#endif

#if defined(GRAY)
		if(add_jpgfile$char$$int$double$$int$int$int(jpgfilename, numIterations, samples, width, height, EJpgfileFormat_Grayscale) != 1)
#else //< #elif defined(RGB)
		if(add_jpgfile$char$$int$double$$int$int$int(jpgfilename, numIterations, samples, width, height, EJpgfileFormat_RGB) != 1)
#endif
		{
			return 1;
		}
	}
	
	return 0;
}
