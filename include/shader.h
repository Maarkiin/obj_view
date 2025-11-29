#ifndef SHADER_H
#define SHADER_H

char* openShader( char* filePath );

GLuint createShader( char* vertexFilePath, char* fragmentFilePath );

#endif