/*
 * (c) 2012 Daniel Burke
 * This file should be accompanied by a LICENSE file.
 * If not, go to https://github.com/burito/ocl
 */


#include <CL/cl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_LOG_SIZE 1000000


void usage(char *path)
{
	printf("OpenCL Compiler Test Tool\n");
	printf("(c) 2012 Daniel Burke - dan.p.burke@gmail.com\n");
	printf("USAGE:\n");
	printf("\t%s\n\t\tdisplay arguments and devices\n", path);
	printf("\t%s filename\n\t\tcompile filename with first device\n", path);
	printf("\t%s N filename\n\t\tcompile filename with Nth device\n", path);
}

int device_list(void)
{
	void* tmp;
	int i, j, n=0;
	cl_uint num_platforms = 0, old_num_platforms;
	cl_uint num_devices = 0, old_num_devices;
	cl_int ret;
	cl_platform_id* cl_pid = NULL;
	cl_device_id* cl_cid = NULL;
	size_t actual;

	ret = clGetPlatformIDs(0, 0, &num_platforms);
	if(ret)
	{
		printf("clGetPlatformIDs() Failed\n");
		return 1;
	}

	old_num_platforms = num_platforms;

	if(!num_platforms)
	{
		printf("So we can't come out to play.\n");
		return 1;
	}

	cl_pid = malloc(sizeof(cl_platform_id) * num_platforms);
	if(!cl_pid)
	{
		printf("malloc() failed\n");
		return 1;
	}
	memset(cl_pid, 0, sizeof(cl_platform_id) * num_platforms);

	ret = clGetPlatformIDs(num_platforms, cl_pid, &num_platforms);
	if(ret)
	{
		printf("clGetPlatformIDs() Failed the second time - %d\n", ret);
		goto dev_malloc;
	}

	if(num_platforms != old_num_platforms)
	{
		printf("There are %d platforms. That's bad.\n", num_platforms);
		goto dev_malloc;
	}

	tmp = malloc( 1024 );

	printf("\nThe Device list is...\n-----------------------\n");

	for( i=0; i<num_platforms; i++ )
	{
		memset(tmp, 0, 1024);
		clGetPlatformInfo( cl_pid[i], CL_PLATFORM_NAME, 1024, tmp, &actual);
		printf("%s\n", (char*)tmp);
		ret = clGetDeviceIDs(cl_pid[i], CL_DEVICE_TYPE_ALL, 0, 0, &num_devices);
		if(ret)
		{
			printf("\tclGetDeviceIDs() failed\n");
			goto dev_tmp;
		}

		if(!num_devices)
		{
			printf("\tThis platform has no devices.\n");
			continue;
		}

		cl_cid = malloc(sizeof(cl_device_id) * num_devices);
		if(!cl_cid)
		{
			printf("\tmalloc(cl_cid) failed\n");
			goto dev_tmp;
		}

		old_num_devices = num_devices;

		ret = clGetDeviceIDs(cl_pid[i], CL_DEVICE_TYPE_ALL, num_devices, cl_cid, &num_devices);
		if(ret)
		{
			printf("\tclGetDeviceIDs() failed the second time\n");
			goto dev_tmp;
		}

		for( j=0; j<num_devices; j++ )
		{
			clGetDeviceInfo(cl_cid[j], CL_DEVICE_NAME, 1024, tmp, &actual);
			printf("   %d\t%s\n", n, (char*)tmp);
			n++;
		}
		free(cl_cid);
	}

dev_tmp:
	free(tmp);
dev_malloc:
	free(cl_pid);
	return 0;
}

char* loadTextFile(char *filename)
{
	FILE *fptr;
	int size;
	struct stat stbuf;
	char *buf;

	fptr = fopen(filename, "rb");
	stat(filename, &stbuf);
	size = stbuf.st_size;
	buf = malloc(size+1);
	fread(buf, size, 1, fptr);
	buf[size] = 0;
	fclose(fptr);
	return buf;
}

int build(int dev, char *filename)
{
	int i, j=0, n=-1;
	cl_uint num_platforms = 0, old_num_platforms;
	cl_uint num_devices = 0, old_num_devices;
	cl_int ret;
	cl_platform_id* cl_pid = NULL;
	cl_device_id* cl_cid = NULL;
	cl_context context;
	cl_program program;
	const char * source;
	char * build_log;
	size_t src_len, binary_size;
	cl_context_properties props[] =
		{
			CL_CONTEXT_PLATFORM, 0,
//			CL_CONTEXT_INTEROP_USER_SYNC, CL_FALSE,
			0
		};


	ret = clGetPlatformIDs(0, 0, &num_platforms);
	if(ret)
	{
		printf("clGetPlatformIDs() Failed\n");
		return 1;
	}

	old_num_platforms = num_platforms;

	if(!num_platforms)
	{
		printf("So we can't come out to play.\n");
		return 1;
	}

	cl_pid = malloc(sizeof(cl_platform_id) * num_platforms);
	if(!cl_pid)
	{
		printf("malloc() failed\n");
		return 1;
	}
	memset(cl_pid, 0, sizeof(cl_platform_id) * num_platforms);

	ret = clGetPlatformIDs(num_platforms, cl_pid, &num_platforms);
	if(ret)
	{
		printf("clGetPlatformIDs() Failed the second time - %d\n", ret);
		goto build_pid;
	}

	if(num_platforms != old_num_platforms)
	{
		printf("There are %d platforms. That's bad.\n", num_platforms);
		goto build_pid;
	}

	for( i=0; i<num_platforms; i++ )
	{
		ret = clGetDeviceIDs(cl_pid[i], CL_DEVICE_TYPE_ALL, 0, 0, &num_devices);
		if(ret)
		{
			printf("\tclGetDeviceIDs() failed\n");
			goto build_pid;
		}

		if(!num_devices)
		{
			printf("\tThis platform has no devices.\n");
			continue;
		}

		cl_cid = malloc(sizeof(cl_device_id) * num_devices);
		if(!cl_cid)
		{
			printf("\tmalloc(cl_cid) failed\n");
			goto build_pid;
		}

		old_num_devices = num_devices;

		ret = clGetDeviceIDs(cl_pid[i], CL_DEVICE_TYPE_ALL, num_devices, cl_cid, &num_devices);
		if(ret)
		{
			printf("\tclGetDeviceIDs() failed the second time\n");
			goto build_cid;
		}


		for( j=0; j<num_devices; j++ )
		{
			n++;
			if(dev == n) break;
		}
		if(dev == n) break;
		free(cl_cid);
	}


	if(dev > n)
	{
		printf("I could not find device number %d.\n", dev);
		device_list();
		goto build_pid;
	}

	props[1] = (cl_context_properties)cl_pid[i];

	context = clCreateContext( props, 1, &cl_cid[j], NULL, NULL, &ret);
	
	if(!context)
	{
		printf("clCreateContext() failed\n");
		goto build_cid;
	}

	source = loadTextFile(filename);
	src_len = strlen(source);

	program = clCreateProgramWithSource(context, 1, &source, &src_len, &ret);

	if(ret != CL_SUCCESS)
	{
		printf("clCreateProgramWithSource() failed\n");
		goto build_context;
	}

	ret = clBuildProgram(program, 1, &cl_cid[j], NULL, NULL, NULL);

	if(ret != CL_SUCCESS)
	{
		build_log = malloc(MAX_LOG_SIZE);
		if(!build_log)
		{
			printf("Failed to malloc() build log.\n");
			goto build_program;
		}
		memset(build_log, 0, MAX_LOG_SIZE);
		binary_size = MAX_LOG_SIZE;
		ret = clGetProgramBuildInfo(program, cl_cid[j], CL_PROGRAM_BUILD_LOG,
				binary_size, build_log, NULL);
		printf("Building \"%s\" failed:\n%s", filename, build_log);
		free(build_log);

	}
	else
	{
		clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t),
				&binary_size, NULL);
		printf("Success: %zu bytes.\n", binary_size);

	}


build_program:
	clReleaseProgram(program);
build_context:
	clReleaseContext(context);
	free((char*)source);
build_cid:
	free(cl_cid);
build_pid:
	free(cl_pid);
	return 0;
}

int main(int argc, char *argv[])
{
	switch(argc) {
	case 1:
		usage(argv[0]);
		device_list();
		break;
	case 2:
		build(0, argv[1]);
		break;
	case 3:
		build(atoi(argv[1]), argv[2]);
		break;
	default:
		usage(argv[0]);
		return 1;
	}

	return 0;
}
