#include "vg.h"
#ifdef NANOVG_GLEW
#  include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include "nanovg.h"
#define NANOVG_GL2_IMPLEMENTATION
#include "nanovg_gl.h"

typedef struct vg_data {
    runt_uint callback_id;
    NVGcontext *vg;
    GLFWwindow *window;
    runt_vm *vm;
    pthread_t thread;
    runt_uint run;
    runt_int lock;
    runt_int win_width;
    runt_int win_height;
} vg_data;

void errorcb(int error, const char* desc)
{
	printf("GLFW error %d: %s\n", error, desc);
}

static void key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	NVG_NOTUSED(scancode);
	NVG_NOTUSED(mods);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

static void * run_loop(void *ud)
{
    vg_data *data = (vg_data *)ud;
    runt_vm *vm = data->vm;
    NVGcolor bgcolor =  nvgRGBAf(252, 251, 227, 255);

	if (!glfwInit()) {
		printf("Failed to init GLFW.");
		return;
	}

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    data->window = glfwCreateWindow(800, 600, "NanoVG", NULL, NULL);

	if (!data->window) {
		glfwTerminate();
        return RUNT_NOT_OK;
	}

	glfwSetKeyCallback(data->window, key);
	glfwSetErrorCallback(errorcb);
	glfwMakeContextCurrent(data->window);

#ifdef NANOVG_GLEW
    if(glewInit() != GLEW_OK) {
		printf("Could not init glew.\n");
	}
#endif

#ifdef DEMO_MSAA
	data->vg = nvgCreateGL2(NVG_STENCIL_STROKES | NVG_DEBUG);
#else
	data->vg = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
#endif

	if (data->vg == NULL) {
		printf("Could not init nanovg.\n");
	}

	glfwSwapInterval(0);
	glfwSetTime(0);
    while(data->run) {
        while(runt_get_state(vm, RUNT_MODE_LOCK) == RUNT_ON);
		double mx, my, t, dt;
		int fbWidth, fbHeight;
		float pxRatio;
        int x, y;
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

		glfwGetCursorPos(data->window, &mx, &my);
		glfwGetWindowSize(data->window, &data->win_width, &data->win_height);
		glfwGetFramebufferSize(data->window, &fbWidth, &fbHeight);
		pxRatio = (float)fbWidth / (float)data->win_width;

		glViewport(0, 0, fbWidth, fbHeight);

		nvgBeginFrame(data->vg, data->win_width, data->win_height, pxRatio);
        runt_set_state(vm, RUNT_MODE_LOCK, RUNT_ON);
            runt_cell_id_exec(vm, data->callback_id);
        runt_set_state(vm, RUNT_MODE_LOCK, RUNT_OFF);
        nvgEndFrame(data->vg);

		glfwSwapBuffers(data->window);
		glfwPollEvents();
        usleep(1000);
    }

	nvgDeleteGL2(data->vg);

	glfwTerminate();
    pthread_exit(NULL);
}

runt_int vg_circle(runt_vm *vm, runt_ptr p)
{
    vg_data *data = (vg_data *)runt_to_cptr(p);
    runt_int i;
    runt_stacklet *s[3];
    float x, y, rad;
    for(i = 0; i < 3; i++) s[i] = runt_pop(vm);

    rad = s[0]->f;
    y = s[1]->f;
    x = s[2]->f;

    nvgBeginPath(data->vg);
    nvgArc(data->vg, x, y, rad, 0, 2 * M_PI, NVG_CCW);
    nvgClosePath(data->vg);
    return RUNT_OK;    
}

runt_int vg_fill(runt_vm *vm, runt_ptr p)
{
    vg_data *data = (vg_data *)runt_to_cptr(p);
    nvgFill(data->vg);
}

runt_int vg_fillcolor(runt_vm *vm, runt_ptr p)
{
    vg_data *data = (vg_data *)runt_to_cptr(p);
    runt_stacklet *s;
    runt_float rgba[4];
    runt_uint i;
    for(i = 0; i < 4; i++) {
        s = runt_pop(vm);
        rgba[3 - i] = s->f;
    }
    nvgFillColor(data->vg, nvgRGBA(rgba[0], rgba[1], rgba[2], rgba[3]));
}

runt_int vg_clear(runt_vm *vm, runt_ptr p)
{
    runt_stacklet *s;
    runt_float rgba[4];
    runt_uint i;
    for(i = 0; i < 4; i++) {
        s = runt_pop(vm);
        rgba[3 - i] = s->f;
    }
    NVGcolor bgcolor = nvgRGBA(rgba[0], rgba[1], rgba[2], rgba[3]);
    glClearColor(bgcolor.r, bgcolor.g, bgcolor.b, bgcolor.a);
}

static runt_int start_nanovg(runt_vm *vm, runt_ptr p)
{
    vg_data *data = (vg_data *)runt_to_cptr(p);
    
    runt_stacklet *s = runt_pop(vm);
    data->callback_id = s->f;

    if(data->callback_id == 0) {
        runt_print(vm, "nvg: Could not find callback\n");
        return RUNT_NOT_OK;
    }
    data->run = 1;
    pthread_create(&data->thread, NULL, run_loop, data);
    return RUNT_OK;
}

runt_int stop_nanovg(runt_vm *vm, runt_ptr p)
{
    vg_data *data = (vg_data *)runt_to_cptr(p);
    data->run = 0;
    pthread_join(data->thread, NULL);
    return RUNT_OK;
}

static runt_int vg_swap(runt_vm *vm, runt_ptr p)
{
    vg_data *data = (vg_data *)runt_to_cptr(p);
    runt_stacklet *s = runt_pop(vm);
    if(s->f != 0) {
        data->callback_id = s->f;
    }
    return RUNT_OK;
}

static runt_int vg_width(runt_vm *vm, runt_ptr p)
{
    vg_data *data = (vg_data *)runt_to_cptr(p);
    runt_stacklet *s = runt_push(vm);
    s->f = data->win_width;
    return RUNT_OK;
}

static runt_int vg_height(runt_vm *vm, runt_ptr p)
{
    vg_data *data = (vg_data *)runt_to_cptr(p);
    runt_stacklet *s = runt_push(vm);
    s->f = data->win_height;
    return RUNT_OK;
}

static runt_int vg_rect(runt_vm *vm, runt_ptr p)
{
    float v[4];
    runt_stacklet *s;
    runt_uint i;
    vg_data *data = (vg_data *)runt_to_cptr(p);

    for(i = 0; i < 4; i++) {
        s = runt_pop(vm);
        v[3 - i] = s->f;
    }

    nvgBeginPath(data->vg);
    nvgRect(data->vg, v[0], v[1], v[2], v[3]);
    nvgClosePath(data->vg);

    return RUNT_OK;
}

static void vg_define(vg_data *data, 
        runt_vm *vm, 
        const char *word,
        runt_uint size,
        runt_proc proc,
        runt_ptr p) 
{
    runt_uint word_id;
    word_id = runt_word_define(vm, word, size, proc);
    runt_word_bind_ptr(vm, word_id, p);
}

void vg_load_dict(runt_vm *vm)
{
    vg_data *data;
    runt_malloc(vm, sizeof(vg_data), (void *)&data);

    data->vm = vm;
    data->lock = 0;
    runt_ptr p = runt_mk_cptr(vm, data);

    vg_define(data, vm, "vgcirc", 6, vg_circle, p);
    vg_define(data, vm, "vgrect", 6, vg_rect, p);
    vg_define(data, vm, "vgfc", 4, vg_fillcolor, p);
    vg_define(data, vm, "vgf", 3, vg_fill, p);
    vg_define(data, vm, "vgstrt", 6, start_nanovg, p);
    vg_define(data, vm, "vgclr", 5, vg_clear, p);
    vg_define(data, vm, "vgstop", 6, stop_nanovg, p);
    vg_define(data, vm, "vgswp", 5, vg_swap, p);
    vg_define(data, vm, "vgw", 3, vg_width, p);
    vg_define(data, vm, "vgh", 3, vg_height, p);
}

void runt_plugin_init(runt_vm *vm)
{
    runt_print(vm, "Loading nanovg plugin\n");
    vg_load_dict(vm);
}
