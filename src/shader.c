#include <glad/glad.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shader.h"

char *openShader(char *filePath)
{
	FILE *fptr;

	fptr = fopen(filePath, "r");
	if (fptr == NULL)
	{
		printf("error opening file %s\n", filePath);
		return NULL;
	}

	char *currLine = (char *)malloc(100 * sizeof(char));

	unsigned int numLines = 0;
	while (fgets(currLine, 100, fptr))
	{
		numLines++;
	}
	fseek(fptr, 0, SEEK_SET);

	char *shaderStr = (char *)calloc(numLines * 100, sizeof(char));
	while (fgets(currLine, 100, fptr))
	{
		strcat(shaderStr, currLine);
	}
	// printf("\n%s", shaderStr );

	free(currLine);
	fclose(fptr);
	return shaderStr;
}

GLuint createShader(char *vertexFilePath, char *fragmentFilePath)
{
	const char *vertexShaderSource = openShader(vertexFilePath);
	const char *fragmentShaderSource = openShader(fragmentFilePath);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}