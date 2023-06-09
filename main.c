#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
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
typedef double   f64;

u32 program;
u32 u_time;
u32 u_mouse;
u32 u_resolution;
u32 vertex;
u32 vertex_array;
u32 vertex_buffer;
f32 time;
u32 prv_fragment_file_time;
b8 canvas_exists;
b8 window_closed;
b8 glsl_error;
GLFWwindow  *window;
i32 current_window_height;

#define UNUSED __attribute__((unused))

const i8 vertex_src[] =
"#version 460 core\n"
"layout(location = 0) in vec2 a_position;\n"
"void\n"
"main() {\n"
"  gl_Position = vec4(a_position, 0, 1);\n"
"}\n";

const i32 vertex_size = sizeof (vertex_src);
void window_close(void);

void
key_handler(GLFWwindow *window, i32 key, i32 scancode UNUSED, i32 action, i32 mods UNUSED) {
  if (key == GLFW_KEY_Q && action == GLFW_PRESS) glfwSetWindowShouldClose(window, 1);
}

void
cursor_pos_handler(GLFWwindow *window UNUSED, f64 x, f64 y) {
  glProgramUniform2f(program, u_mouse, x, current_window_height - y);
}

void
resize_handler(GLFWwindow *window UNUSED, i32 width, i32 height) {
  glViewport(0, 0, width, height);
  glProgramUniform2f(program, u_resolution, width, height);
  current_window_height = height;
}

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
  /* make window */
  window = glfwCreateWindow(640, 480, "shader-template", NULL, NULL);
  if (!window) {
    fprintf(stderr, "glfwCreateWindow\n");
    glfwTerminate();
    return 0;
  }
  /* opengl context and load */
  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  /* callback */
  glfwSetKeyCallback(window, key_handler);
  glfwSetFramebufferSizeCallback(window, resize_handler);
  glfwSetCursorPosCallback(window, cursor_pos_handler);
  return 1;
}

void
shader_make(u32 type, const i8 *source, const i32 length, u32 *output) {
  *output = glCreateShader(type);
  glShaderSource(*output, 1, &source, &length);
  glCompileShader(*output);
  i32 status;
  glGetShaderiv(*output, GL_COMPILE_STATUS, &status);
  if (!status) {
    i8 log[512];
    glGetShaderInfoLog(*output, 512, NULL, log);
    fprintf(stderr, "%s: %s\n", type == GL_VERTEX_SHADER ? "vertex" : "fragment", log);
  }
}

b8
shader_program_make(void) {
  FILE *fragment_file = fopen("shader.glsl", "r");
  if (!fragment_file) {
    fprintf(stderr, "fopen(\"shader.glsl\", \"r\")\n");
    glfwTerminate();
    return 0;
  }
  fseek(fragment_file, 0, SEEK_END);
  i32 fragment_size = ftell(fragment_file);
  rewind(fragment_file);
  i8 *fragment_src  = malloc(fragment_size + 1);
  fread(fragment_src, 1, fragment_size, fragment_file);
  fclose(fragment_file);
  fragment_src[fragment_size] = '\0';
  u32 fragment;
  shader_make(GL_FRAGMENT_SHADER, fragment_src, fragment_size, &fragment);
  free(fragment_src);
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
    glsl_error = 1;
  }
  glDeleteShader(fragment);
  glUseProgram(program);
  u_resolution = glGetUniformLocation(program, "resolution");
  u_mouse      = glGetUniformLocation(program, "mouse");
  u_time       = glGetUniformLocation(program, "time");
  i32 width, height;
  glfwGetWindowSize(window, &width, &height);
  glProgramUniform2f(program, u_resolution, width, height);
  current_window_height = height;
  return 1;
}

b8
shader_program_get_state(u32 *fragment_file_time) {
  struct stat fragment_status;
  if (stat("./shader.glsl", &fragment_status) != 0) {
    perror("error: couldn't retrieve status of shader.glsl\n");
    return 0;
  }
  *fragment_file_time = fragment_status.st_mtim.tv_sec * 1e9 + fragment_status.st_mtim.tv_nsec;
  return 1;
}

b8
shader_program_startup(void) {
  shader_make(GL_VERTEX_SHADER, vertex_src, vertex_size, &vertex);
  if (!shader_program_make() || !shader_program_get_state(&prv_fragment_file_time)) {
    glfwTerminate();
    return 0;
  }
  return 1;
}

b8
shader_program_update(void) {
  u32 cur_fragment_file_time;
  if (!shader_program_get_state(&cur_fragment_file_time)) return 0;
  if (cur_fragment_file_time != prv_fragment_file_time) {
    glDeleteShader(program);
    prv_fragment_file_time = cur_fragment_file_time;
    glsl_error = 0;
    shader_program_make();
  }
  return 1;
}

b8
window_is_running(void) {
  f32 prv_time = glfwGetTime();
  if (!shader_program_update()) return 0;
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  if (!glsl_error) {
    glProgramUniform1f(program, u_time, time);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }
  glfwSwapBuffers(window);
  glfwPollEvents();
  time += glfwGetTime() - prv_time;
  return !glfwWindowShouldClose(window);
}

void
window_close(void) {
  if (window_closed) return;
  window_closed = 1;
  glDeleteShader(vertex);
  glDeleteProgram(program);
  if (canvas_exists) {
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteVertexArrays(1, &vertex_array);
  }
  glfwTerminate();
}

void
canvas_make(void) {
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
  canvas_exists = 1;
}

i32
main(void) {
  if (!window_open())            return 1;
  if (!shader_program_startup()) return 1;
  canvas_make();
  while (window_is_running());
  window_close();
  return 0;
}
