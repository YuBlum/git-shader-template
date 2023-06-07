#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef _Bool    b8;
typedef char     i8;
typedef uint8_t  u8;
typedef short    i16;
typedef uint16_t u16;
typedef int      i32;
typedef uint32_t u32;
typedef long     i64;
typedef uint64_t u64;
typedef float    f32;

u32 program;
u32 u_time;
u32 vertex_array;
u32 vertex_buffer;
f32 time;
void *window;
#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480
#define WINDOW_NAME   "shader-template"

const i8 vertex_src[] =
"#version 460 core\n"
"layout(location = 0) in vec2 a_position;\n"
"void\n"
"main() {\n"
"  gl_Position = vec4(a_position, 0, 1);\n"
"}\n";

const i32 vertex_size = sizeof (vertex_src);

b8
window_open(void) {
  if (!glfwInit()) {
    fprintf(stderr, "glfwInit\n");
    return 0;
  }
  /* window hints */
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, 0);
  /* make window */
  window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, NULL, NULL);
  if (!window) {
    fprintf(stderr, "glfwCreateWindow\n");
    glfwTerminate();
    return 0;
  }
  /* opengl context and load */
  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  return 1;
}

b8
window_is_running(void) {
  f32 prv_time = glfwGetTime();
  glClearColor(1, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glProgramUniform1f(program, u_time, time);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glfwSwapBuffers(window);
  glfwPollEvents();
  time += glfwGetTime() - prv_time;
  return !glfwWindowShouldClose(window);
}

void
window_close(void) {
  glDeleteProgram(program);
  glDeleteBuffers(1, &vertex_buffer);
  glDeleteVertexArrays(1, &vertex_array);
  glfwTerminate();
}

b8
shader_make(u32 type, const i8 *source, const i32 length, u32 *output) {
  *output = glCreateShader(type);
  glShaderSource(*output, 1, &source, &length);
  glCompileShader(*output);
  i32 status;
  glGetShaderiv(*output, GL_COMPILE_STATUS, &status);
  if (!status) {
    i8 log[128];
    glGetShaderInfoLog(*output, 128, NULL, log);
    fprintf(stderr, "%s: %s\n", type == GL_VERTEX_SHADER ? "vertex" : "fragment", log);
    return 0;
  }
  return 1;
}

b8
program_make(void) {
  FILE *fragment_file = fopen("shader.glsl", "r");
  if (!fragment_file) {
    fprintf(stderr, "fopen(\"shader.glsl\", \"r\")\n");
    exit(1);
  }
  fseek(fragment_file, 0, SEEK_END);
  i32 fragment_size = ftell(fragment_file);
  rewind(fragment_file);
  i8 *fragment_src  = malloc(fragment_size + 1);
  fread(fragment_src, 1, fragment_size, fragment_file);
  fragment_src[fragment_size] = '\0';
  fclose(fragment_file);
  u32 vertex, fragment;
  if (!shader_make(GL_FRAGMENT_SHADER, fragment_src, fragment_size, &fragment)) {
    glDeleteProgram(program);
    glfwTerminate();
    free(fragment_src);
    return 0;
  }
  free(fragment_src);
  if (!shader_make(GL_VERTEX_SHADER, vertex_src, vertex_size, &vertex)) {
    glDeleteShader(fragment);
    glDeleteProgram(program);
    glfwTerminate();
    return 0;
  }
  program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);
  i32 status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (!status) {
    i8 log[128];
    glGetProgramInfoLog(program, 128, NULL, log);
    fprintf(stderr, "%s\n", log);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    glDeleteProgram(program);
    glfwTerminate();
    return 0;
  }
  glDeleteShader(vertex);
  glDeleteShader(fragment);
  glUseProgram(program);
  glProgramUniform2f(program, glGetUniformLocation(program, "resolution"), WINDOW_WIDTH, WINDOW_HEIGHT);
  u_time = glGetUniformLocation(program, "time");
  return 1;
}

void
screen_make(void) {
  f32 vertices[] = {
    -1.0f, -1.0f,
     1.0f, -1.0f,
     1.0f,  1.0f,
     1.0f,  1.0f,
    -1.0f,  1.0f,
    -1.0f, -1.0f,
  };
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, 0, sizeof (float) * 2, (void *)0);
  glEnableVertexAttribArray(0);
}

i32
main(void) {
  if (!window_open()) return 1;
  if (!program_make()) return 1;
  screen_make();
  while (window_is_running());
  window_close();
  return 0;
}
