#include "context.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void OnFramebufferSizeChange(GLFWwindow *window, int width, int height)
{
	SPDLOG_INFO("framebuffer size changed: ({} x {})", width, height);

	auto context = static_cast<Context*>(glfwGetWindowUserPointer(window));
	context->Reshape(width, height);
}

void OnCursorPos(GLFWwindow* window, double x, double y)
{
	auto context = (Context *)glfwGetWindowUserPointer(window);
	context->MouseMove(x, y);
}

void OnMouseButton(GLFWwindow* window, int button, int action, int modifier)
{
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, modifier);
	auto context = (Context*)glfwGetWindowUserPointer(window);
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	context->MouseButton(button, action, x, y);
}

void OnKeyEvent(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	SPDLOG_INFO("key: {}, scancode: {}, action: {}, mods: {}{}{}",
			key, scancode,
			action == GLFW_PRESS ? "Pressed" : 
			action == GLFW_RELEASE ? "Released" : 
			action == GLFW_REPEAT ? "Repeat" : "Unknown",
			mods & GLFW_MOD_CONTROL ? "C" : "-",
			mods & GLFW_MOD_SHIFT ? "S" : "-",
			mods & GLFW_MOD_ALT ? "A" : "-");
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

void OnCharEvent(GLFWwindow * window, unsigned int ch)
{
	ImGui_ImplGlfw_CharCallback(window, ch);
}

void OnScroll(GLFWwindow* window, double xoffset, double yoffset)
{
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

int main()
{
	SPDLOG_INFO("Start program");

	SPDLOG_INFO("Initialize glfw");
	if (!glfwInit())
	{
		const char * description = nullptr;
		glfwGetError(&description);
		SPDLOG_ERROR("failed to initialize glfw: {}", description); 
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
	glfwWindowHint(GLFW_SAMPLES, 4); // sample 5개를 쓰는 멀티샘플

	SPDLOG_INFO("Create glfw window");
	auto window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, nullptr, nullptr); 
	if (!window)
	{
		SPDLOG_ERROR("failed to create glfw window");
		const char * description = nullptr;
		glfwGetError(&description); 
		SPDLOG_ERROR("failed to initialize glfw: {}", description); 
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		SPDLOG_ERROR("failed to initialize glad");
		glfwTerminate();
		return -1;
	}
	auto glVersion = glGetString(GL_VERSION);
	SPDLOG_INFO("OpenGL context version: {}", reinterpret_cast<const char*>(glVersion));

	auto imguiContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(imguiContext);
	ImGui_ImplGlfw_InitForOpenGL(window, false);
	ImGui_ImplOpenGL3_Init();
	ImGui_ImplOpenGL3_CreateFontsTexture();
	ImGui_ImplOpenGL3_CreateDeviceObjects();

	auto context = Context::Create();
	if (!context)
	{
		SPDLOG_ERROR("failed to create context");
		glfwTerminate();
		return -1;
	}
	
	glfwSetWindowUserPointer(window, context.get());

	OnFramebufferSizeChange(window, WINDOW_WIDTH, WINDOW_HEIGHT);

	glfwSetFramebufferSizeCallback(window, OnFramebufferSizeChange);
	glfwSetKeyCallback(window, OnKeyEvent);
	glfwSetCharCallback(window, OnCharEvent);
	glfwSetCursorPosCallback(window, OnCursorPos);
	glfwSetMouseButtonCallback(window, OnMouseButton);
	glfwSetScrollCallback(window, OnScroll);

	SPDLOG_INFO("Start main loop");

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		context->ProcessInput(window);
		context->Render();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}
	context.reset();
	
	ImGui_ImplOpenGL3_DestroyFontsTexture();
	ImGui_ImplOpenGL3_DestroyDeviceObjects();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(imguiContext);

	glfwTerminate();
	return 0;
}