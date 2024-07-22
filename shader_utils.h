#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

unsigned int compileShader(unsigned int type, const char *source);
unsigned int createShader(const char *vertexShaderSource,
                          const char *fragmentShaderSource);
unsigned int createShaderFromFiles(const char *vertexPath,
                                   const char *fragmentPath);
char *loadShaderSource(const char *filePath);

#endif // !SHADER_UTILS_H SHADER_UTILS_H
