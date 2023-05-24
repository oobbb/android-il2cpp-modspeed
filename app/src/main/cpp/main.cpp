#include <jni.h>
#include <thread>
#include <unistd.h>
#include "log.h"
#include "xdl.h"
#include "dobby.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_android.h"



int glHeight, glWidth;
bool setupimg;
static float timeScale = 1.00f;
void*(*il2cpp_resolve_icall)(const char *) = nullptr;


void(*_set_timeScale)(float);
void set_timeScale(float Time)
{
    Time = timeScale;
    _set_timeScale(Time);
}
void (*origInput)(void*,void*,void*) = nullptr;
void Input(void *thiz, void *ex_ab, void *ex_ac) {
    origInput(thiz, ex_ab, ex_ac);
    ImGui_ImplAndroid_HandleInputEvent((AInputEvent *)thiz);
    return;
}

void SetupImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float) glWidth, (float) glHeight);
    ImGui_ImplOpenGL3_Init("#version 100");
    ImGui::StyleColorsLight();

    ImGui::GetStyle().ScaleAllSizes(3.0f);
}

EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &glWidth);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &glHeight);

    if (!setupimg) {
        SetupImgui();
        setupimg = true;
    }

    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();


    ImGui::Begin("test!");
    ImGui::SetWindowSize(ImVec2(800, 500));
    if (ImGui::SliderFloat("Speed", &timeScale, 1.00f, 10.00f)) {
        _set_timeScale(timeScale);
    }
    ImGui::End();


    ImGui::EndFrame();
    ImGui::Render();
    glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return old_eglSwapBuffers(dpy, surface);
}


void hack_thread() {
    sleep(3);
    LOGD("Hack App Done!!! %d", getpid());

    auto handle = xdl_open("libil2cpp.so", 0);
    LOGD("Handle %p", handle);
    il2cpp_resolve_icall = (void *(*)(const char *)) xdl_sym(handle, "il2cpp_resolve_icall",
                                                             nullptr);
    LOGD("il2cpp_resolve_icall %p", il2cpp_resolve_icall);
    auto TimeHack = il2cpp_resolve_icall("UnityEngine.Time::set_timeScale(System.Single)");
    LOGD("TimeHack %p", TimeHack);
    DobbyHook(TimeHack, (dobby_dummy_func_t) set_timeScale, (dobby_dummy_func_t *) &_set_timeScale);


    auto eglSwapBuffers = dlsym(nullptr, "eglSwapBuffers");
    DobbyHook((void*)eglSwapBuffers,(dobby_dummy_func_t)hook_eglSwapBuffers,
              (dobby_dummy_func_t*)&old_eglSwapBuffers);
    auto sym_input = xdl_sym(xdl_open("libinput.so",0), "_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE",nullptr);
    if (NULL != sym_input) {
        DobbyHook(sym_input,(dobby_dummy_func_t)Input,(dobby_dummy_func_t*)&origInput);
    }
}
__attribute__((constructor))
void lib_main() {
    std::thread(hack_thread).detach();
}