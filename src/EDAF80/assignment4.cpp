#include "assignment4.hpp"
#include "parametric_shapes.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/node.hpp"
#include "core/ShaderProgramManager.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <tinyfiledialogs.h>

#include <stdexcept>

edaf80::Assignment4::Assignment4(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
	        static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	        0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 4", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}
}

void
edaf80::Assignment4::run()
{
	// Set up the camera
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = 3.0f; // 3 m/s => 10.8 km/h
	auto camera_position = mCamera.mWorld.GetTranslation();

	glm::vec3 deep_color = glm::vec3(0.0f, 0.0f, 0.1f);
	glm::vec3 shallow_color = glm::vec3(0.0f, 0.5f, 0.5f);

	float amplitudes[2] = {1.0, 0.5};
	float frequencies[2] = {0.2, 0.4};
	float phases[2] = {0.5, 1.3};
	float sharpness[2] = {2.0, 2.0};
	glm::vec2 directions[2] = {glm::vec2(-1.0,0.0),glm::vec2(-0.7,0.7)};
	int nbr_of_waves = 2;

	// Create the shader programs
	ShaderProgramManager program_manager;
	GLuint fallback_shader = 0u;
	program_manager.CreateAndRegisterProgram("Fallback",
	                                         { { ShaderType::vertex, "EDAF80/fallback.vert" },
	                                           { ShaderType::fragment, "EDAF80/fallback.frag" } },
	                                         fallback_shader);
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}

	GLuint diffuse_shader = 0u;
	program_manager.CreateAndRegisterProgram("Diffuse",
	                                         { { ShaderType::vertex, "EDAF80/diffuse.vert" },
	                                           { ShaderType::fragment, "EDAF80/diffuse.frag" } },
	                                         diffuse_shader);
	if (diffuse_shader == 0u)
		LogError("Failed to load diffuse shader");

	GLuint normal_shader = 0u;
	program_manager.CreateAndRegisterProgram("Normal",
	                                         { { ShaderType::vertex, "EDAF80/normal.vert" },
	                                           { ShaderType::fragment, "EDAF80/normal.frag" } },
	                                         normal_shader);
	if (normal_shader == 0u)
		LogError("Failed to load normal shader");

	GLuint texcoord_shader = 0u;
	program_manager.CreateAndRegisterProgram("Texture coords",
	                                         { { ShaderType::vertex, "EDAF80/texcoord.vert" },
	                                           { ShaderType::fragment, "EDAF80/texcoord.frag" } },
	                                         texcoord_shader);
	if (texcoord_shader == 0u)
		LogError("Failed to load texcoord shader");

	GLuint cube_map = 0u;
	program_manager.CreateAndRegisterProgram("Skybox",
	                                         { { ShaderType::vertex, "EDAF80/cube_map.vert" },
	                                           { ShaderType::fragment, "EDAF80/cube_map.frag" } },
	                                         cube_map);
	if (cube_map == 0u)
		LogError("Failed to load texcoord shader");

	GLuint water_shader = 0u;
	program_manager.CreateAndRegisterProgram("Waves",
	                                         { { ShaderType::vertex, "EDAF80/water.vert" },
	                                           { ShaderType::fragment, "EDAF80/water.frag" } },
	                                         water_shader);
	if (water_shader == 0u)
		LogError("Failed to load texcoord shader");


	//
	// Todo: Insert the creation of other shader programs.
	//       (Check how it was done in assignment 3.)
	//
	auto light_position = glm::vec3(-2.0f, 4.0f, 2.0f);
	auto const set_uniforms = [&light_position](GLuint program){
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
	};


	float ellapsed_time_s = 0.0f;

	//
	// Todo: Load your geometry
	//
	auto const water_set_uniforms = [&amplitudes,&frequencies,&sharpness,&phases,&directions,
		&ellapsed_time_s,&nbr_of_waves,&deep_color,&shallow_color,&camera_position](GLuint program){
		glUniform1fv(glGetUniformLocation(program, "amplitudes"), 2, amplitudes);
		glUniform1fv(glGetUniformLocation(program, "frequencies"), 2,frequencies);
		glUniform1fv(glGetUniformLocation(program, "sharpness"), 2, sharpness);
		glUniform1fv(glGetUniformLocation(program, "phases"), 2, phases);
		glUniform2fv(glGetUniformLocation(program, "directions"), 2, glm::value_ptr(directions[0]));
		glUniform1f(glGetUniformLocation(program, "time"), ellapsed_time_s);
		glUniform1i(glGetUniformLocation(program, "nbr_of_waves"), nbr_of_waves);
		glUniform3fv(glGetUniformLocation(program, "deep_color"),1, glm::value_ptr(deep_color));
		glUniform3fv(glGetUniformLocation(program, "shallow_color"),1, glm::value_ptr(shallow_color));
		glUniform3fv(glGetUniformLocation(program, "camera_position"),1, glm::value_ptr(camera_position));
	};

	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	auto skybox_id = bonobo::loadTextureCubeMap(
												config::resources_path("cubemaps/NissiBeach2/posx.jpg"), 
												config::resources_path("cubemaps/NissiBeach2/negx.jpg"),
												config::resources_path("cubemaps/NissiBeach2/posy.jpg"), 
												config::resources_path("cubemaps/NissiBeach2/negy.jpg"),
												config::resources_path("cubemaps/NissiBeach2/posz.jpg"), 
												config::resources_path("cubemaps/NissiBeach2/negz.jpg"), 
												true
												);
	auto water_id = bonobo::loadTexture2D(config::resources_path("textures/waves.png"));

	auto sphere = parametric_shapes::createSphere(25,100,100);
	auto demo_shape = parametric_shapes::createSphere(1.5f, 40u, 40u);
	auto plane  = parametric_shapes::createQuadNew(20, 20, 50, 50);

	auto water  = Node();
	water.set_geometry(plane);
	water.set_program(&water_shader, water_set_uniforms);
	water.get_transform().SetTranslate(glm::vec3(-5.0f,-5.0f,-20.0f));
	water.add_texture("skybox", skybox_id, GL_TEXTURE_CUBE_MAP);
	water.add_texture("water", water_id, GL_TEXTURE_2D);

	auto skybox = Node();
	skybox.set_geometry(sphere);
	skybox.set_program(&cube_map, set_uniforms);
	skybox.add_texture("skybox", skybox_id, GL_TEXTURE_CUBE_MAP);

	// Enable face culling to improve performance:
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glCullFace(GL_BACK);

	auto lastTime = std::chrono::high_resolution_clock::now();

	auto polygon_mode = bonobo::polygon_mode_t::fill;
	std::int32_t water_program_index = 0;
	std::int32_t skybox_program_index = 0;
	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;

	while (!glfwWindowShouldClose(window)) {
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;
		ellapsed_time_s += std::chrono::duration<float>(deltaTimeUs).count();

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(deltaTimeUs, inputHandler);
		camera_position = mCamera.mWorld.GetTranslation();

		if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			shader_reload_failed = !program_manager.ReloadAllPrograms();
			if (shader_reload_failed)
				tinyfd_notifyPopup("Shader Program Reload Error",
				                   "An error occurred while reloading shader programs; see the logs for details.\n"
				                   "Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
				                   "error");
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F11) & JUST_RELEASED)
			mWindowManager.ToggleFullscreenStatusForWindow(window);


		// Retrieve the actual framebuffer size: for HiDPI monitors,
		// you might end up with a framebuffer larger than what you
		// actually asked for. For example, if you ask for a 1920x1080
		// framebuffer, you might get a 3840x2160 one instead.
		// Also it might change as the user drags the window between
		// monitors with different DPIs, or if the fullscreen status is
		// being toggled.
		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		glViewport(0, 0, framebuffer_width, framebuffer_height);

		//
		// Todo: If you need to handle inputs, you can do it here
		//


		mWindowManager.NewImGuiFrame();

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		bonobo::changePolygonMode(polygon_mode);


		if (!shader_reload_failed) {
			//
			// Todo: Render all your geometry here.
			//
			skybox.render(mCamera.GetWorldToClipMatrix());
			water.render(mCamera.GetWorldToClipMatrix());
		}


		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//
		// Todo: If you want a custom ImGUI window, you can set it up
		//       here
		//


		bool opened = ImGui::Begin("Scene Control", nullptr, ImGuiWindowFlags_None);
		ImGui::Separator();
		if (opened) {
			bonobo::uiSelectPolygonMode("Polygon mode", polygon_mode);
			auto water_selection_result = program_manager.SelectProgram("Water", water_program_index);
			if (water_selection_result.was_selection_changed) {
				water.set_program(water_selection_result.program, water_set_uniforms);
			}
			ImGui::Separator();
			auto skybox_selection_result = program_manager.SelectProgram("Skybox", skybox_program_index);
			if (skybox_selection_result.was_selection_changed) {
				skybox.set_program(skybox_selection_result.program, set_uniforms);
			}
		}
		ImGui::Separator();
		ImGui::ColorEdit3("Deep", glm::value_ptr(deep_color));
		ImGui::ColorEdit3("Shallow", glm::value_ptr(shallow_color));
		ImGui::Separator();
		float* p = amplitudes;
		for(size_t i = 0; i < nbr_of_waves; ++i){

			ImGui::SliderFloat((std::string("Amplitude ") + std::to_string(i+1)).c_str(), &amplitudes[i], 0.0f, 10.0f);
			ImGui::SliderFloat((std::string("Frequency ") + std::to_string(i+1)).c_str(), &frequencies[i], 0.0f, 10.0f);
			ImGui::SliderFloat((std::string("Phase ") + std::to_string(i+1)).c_str(), &phases[i], 0.0f, 10.0f);
			ImGui::SliderFloat((std::string("Sharpness ") + std::to_string(i+1)).c_str(), &sharpness[i], 0.0f, 10.0f);
			ImGui::SliderFloat2((std::string("Direction ") + std::to_string(i+1)).c_str(), glm::value_ptr(directions[i]), -1.0, 1.0);
		}
		ImGui::End();

		if (show_logs)
			Log::View::Render();
		if (show_gui)
			mWindowManager.RenderImGuiFrame();

		glfwSwapBuffers(window);
	}
}

int main()
{
	Bonobo framework;

	try {
		edaf80::Assignment4 assignment4(framework.GetWindowManager());
		assignment4.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}
