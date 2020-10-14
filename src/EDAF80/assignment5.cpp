#include "assignment5.hpp"
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

edaf80::Assignment5::Assignment5(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
	        static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	        0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 5", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}
}

bool testSphereSphere(glm::vec3 p1, float r1, glm::vec3 p2, float r2){
	float dist = sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) + pow(p1.z - p2.z, 2));
	return dist < (r1 + r2);
}

void moveShip(std::vector<Node> ship, float shipSpeed, glm::vec3 dir){
	for (std::size_t i = 0; i < ship.size(); ++i) {
		glm::vec3 shipPartPos = ship[i].get_transform().GetTranslation() + dir*shipSpeed;
		ship[i].get_transform().SetTranslate(shipPartPos);
	}
	std::cout << "moveShip called" << std::endl;
}


void
edaf80::Assignment5::run()
{
	// Set up the camera
	mCamera.mWorld.ResetTransform();
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 20.0f, 0.0f));
	mCamera.mWorld.RotateX(-1.57);
	mCamera.mWorld.RotateY(3.14);
	// mCamera.mWorld.LookTowards(glm::vec3(0, -1, 0));
	// mCamera.mWorld.LookAt(glm::vec3(0.0f, -1000.0f, 0.0f));
	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = 3.0f; // 3 m/s => 10.8 km/h

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
	program_manager.CreateAndRegisterProgram("Texture coords",
	                                         { { ShaderType::vertex, "EDAF80/diffuse.vert" },
	                                           { ShaderType::fragment, "EDAF80/diffuse.frag" } },
	                                         diffuse_shader);
	if (diffuse_shader == 0u)
	{
		LogError("Failed to load texcoord shader");
	}

	GLuint normal_shader = 0u;
	program_manager.CreateAndRegisterProgram("Normal",
	                                         { { ShaderType::vertex, "EDAF80/normal.vert" },
	                                           { ShaderType::fragment, "EDAF80/normal.frag" } },
	                                         normal_shader);
	if (normal_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}

	GLuint background_shader = 0u;
	program_manager.CreateAndRegisterProgram("Background",
	                                         { { ShaderType::vertex, "EDAF80/background.vert" },
	                                           { ShaderType::fragment, "EDAF80/background.frag" } },
	                                         background_shader);
	if (background_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}

	GLuint cube_map = 0u;
	program_manager.CreateAndRegisterProgram("Skybox",
	                                         { { ShaderType::vertex, "EDAF80/cube_map.vert" },
	                                           { ShaderType::fragment, "EDAF80/cube_map.frag" } },
	                                         cube_map);
	if (cube_map == 0u)
		LogError("Failed to load cubemap shader");

	GLuint phong_shader = 0u;
	program_manager.CreateAndRegisterProgram("Phong",
	                                         { { ShaderType::vertex, "EDAF80/phong.vert" },
	                                           { ShaderType::fragment, "EDAF80/phong.frag" } },
	                                         phong_shader);
	if (cube_map == 0u)
		LogError("Failed to load phong shader");

	GLuint ship_shader = 0u;
	program_manager.CreateAndRegisterProgram("Phong",
	                                         { { ShaderType::vertex, "EDAF80/ship.vert" },
	                                           { ShaderType::fragment, "EDAF80/ship.frag" } },
	                                         ship_shader);
	if (ship_shader == 0u)
		LogError("Failed to load phong shader");

	GLuint default_shader = 0u;
	program_manager.CreateAndRegisterProgram("Phong",
	                                         { { ShaderType::vertex, "EDAF80/default.vert" },
	                                           { ShaderType::fragment, "EDAF80/default.frag" } },
	                                         default_shader);
	if (default_shader == 0u)
		LogError("Failed to load phong shader");


	// Uniforms
	float ellapsed_time_s = 0.0f;
	// auto light_position = glm::vec3(-2.0f, 4.0f, 2.0f);
	auto light_position = glm::vec3(0.5f, 2.0f, -0.8f);
	auto const set_uniforms = [&light_position, &ellapsed_time_s](GLuint program){
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform1f(glGetUniformLocation(program, "ellapsed_time_s"), ellapsed_time_s);

	};


	//
	// Todo: Insert the creation of other shader programs.
	//       (Check how it was done in assignment 3.)
	//
	auto background_shape = parametric_shapes::createQuadNew(20, 20, 10, 10);
	if (background_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the background");
		return;
	}

	GLuint const nebula_texture = bonobo::loadTexture2D(config::resources_path("nebula3.jpg"));
	//GLuint const nebula_texture = bonobo::loadTexture2D(config::resources_path("planets/2k_jupiter.jpg"));


	Node background;
	background.set_geometry(background_shape);
	background.set_program(&background_shader, set_uniforms);
	background.get_transform().SetTranslate(glm::vec3(-10, 0, -10));
	background.add_texture("nebula", nebula_texture, GL_TEXTURE_2D);


	bool use_normal_mapping = true;
	auto camera_position = mCamera.mWorld.GetTranslation();
	auto ambient = glm::vec3(0.1f, 0.1f, 0.1f);
	auto diffuse = glm::vec3(0.7f, 0.2f, 0.4f);
	auto specular = glm::vec3(1.0f, 1.0f, 1.0f);
	auto shininess = 10.0f;
	auto const phong_set_uniforms = [&use_normal_mapping, &light_position, &camera_position,&ambient,&diffuse,&specular,&shininess](GLuint program){
		glUniform1i(glGetUniformLocation(program, "use_normal_mapping"), use_normal_mapping ? 1 : 0);
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform3fv(glGetUniformLocation(program, "ambient"), 1, glm::value_ptr(ambient));
		glUniform3fv(glGetUniformLocation(program, "diffuse"), 1, glm::value_ptr(diffuse));
		glUniform3fv(glGetUniformLocation(program, "specular"), 1, glm::value_ptr(specular));
		glUniform1f(glGetUniformLocation(program, "shininess"), shininess);
	};

	float astRadius = 1.0f;
	auto sphere_shape = parametric_shapes::createSphere(astRadius, 20, 20);

	// Setting up asteroids
	int N = 10;
	Node asteroids[N];
	glm::vec3 astRots[N];

	auto ast_normal_map_id = bonobo::loadTexture2D(config::resources_path("asteroid/ground_0010_normal_2k.jpg"));
	auto ast_diffuse_map_id = bonobo::loadTexture2D(config::resources_path("asteroid/ground_0010_base_color_2k.jpg"));
	for(int i = 0; i < N; i++){
		Node ast;
		ast.set_geometry(sphere_shape);
		ast.set_program(&phong_shader, phong_set_uniforms);

		int randNum = rand() % (10 + 1) + (-4);
		ast.get_transform().SetTranslate(glm::vec3(randNum, 2, 6 + i*3));

		ast.add_texture("normal_map", ast_normal_map_id, GL_TEXTURE_2D);
		ast.add_texture("diffuse_map", ast_diffuse_map_id, GL_TEXTURE_2D);

		asteroids[i] = ast;

		astRots[i] = glm::vec3(rand() % 3, rand() % 3, rand() % 3);
	}

	// Setting up ship
	float shipScale = 0.25f;
	std::vector<bonobo::mesh_data> const shipGeometry = bonobo::loadObjects(config::resources_path("asteroid/Intergalactic_Spaceship-(Wavefront).obj"));
	
	if (shipGeometry.empty()) {
		LogError("Failed to load the ship geometry: exiting.");
		throw std::runtime_error("EXIT_FAILURE");
	}

	std::vector<Node> ship(shipGeometry.size());
	glm::vec3 shipPos = glm::vec3(0, 2, -3);
	for (std::size_t i = 0; i < ship.size(); ++i) {
		ship[i].get_transform().SetScale(shipScale);
		ship[i].get_transform().SetTranslate(shipPos);
		ship[i].set_geometry(shipGeometry[i]);
		ship[i].set_program(&default_shader);
	}
	float shipRadius = 0.7f;

	//
	// Todo: Load your geometry
	//

	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	// Enable face culling to improve performance:
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glCullFace(GL_BACK);


	auto lastTime = std::chrono::high_resolution_clock::now();

	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;

	// Game params
	float speed = 0.05f;
	float shipSpeed = 0.05f;
	float rotSpeed = 0.02f;
	bool dead = false;


	auto polygon_mode = bonobo::polygon_mode_t::fill;
	std::int32_t background_program_index = 0;
	std::int32_t asteroid_program_index = 0;


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
		glm::vec3 dir = glm::vec3(0,0,0);
		if(inputHandler.GetKeycodeState(GLFW_KEY_W) & PRESSED || inputHandler.GetKeycodeState(GLFW_KEY_UP) & PRESSED){
			dir += glm::vec3(0,0,1);
		}
		if(inputHandler.GetKeycodeState(GLFW_KEY_A) & PRESSED || inputHandler.GetKeycodeState(GLFW_KEY_LEFT) & PRESSED){
			dir += glm::vec3(1,0,0);
		}
		if(inputHandler.GetKeycodeState(GLFW_KEY_S) & PRESSED || inputHandler.GetKeycodeState(GLFW_KEY_DOWN) & PRESSED){
			dir += glm::vec3(0,0,-1);
		}
		if(inputHandler.GetKeycodeState(GLFW_KEY_D) & PRESSED || inputHandler.GetKeycodeState(GLFW_KEY_RIGHT) & PRESSED){
			dir += glm::vec3(-1,0,0);		
		}
		for (std::size_t i = 0; i < ship.size(); ++i) {
			glm::vec3 shipPartPos = ship[i].get_transform().GetTranslation() + dir*shipSpeed;
			ship[i].get_transform().SetTranslate(shipPartPos);
		}
		shipPos = ship[0].get_transform().GetTranslation();
		// std::cout << shipPos << std::endl;


		mWindowManager.NewImGuiFrame();

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		bonobo::changePolygonMode(polygon_mode);

		// Move + rotate asteroids and check collision
		for(int i = 0; i < N; i++){
			asteroids[i].get_transform().Translate(glm::vec3(0, 0, -speed));
			asteroids[i].get_transform().Rotate(rotSpeed, astRots[i]);
			glm::vec3 astPos = asteroids[i].get_transform().GetTranslation();

			// Collision check
			if(testSphereSphere(shipPos, shipRadius, astPos, astRadius)){
				dead = true;
			}
			// Reset asteroid position
			if(asteroids[i].get_transform().GetTranslation().z < -10)
			{
				float randNum = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 17 - 8;
				// std::cout << randNum << std::endl;
				asteroids[i].get_transform().SetTranslate(glm::vec3(randNum, 2, N*2));
			}
		}

		if(dead) {
			for(int i = 0 ; i < N ; i++){
				float randNum = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 17 - 8;
				// std::cout << randNum << std::endl;
				asteroids[i].get_transform().SetTranslate(glm::vec3(randNum, 2, 15 + 3*i));
			}
		}


		if (!shader_reload_failed) {
			background.render(mCamera.GetWorldToClipMatrix());

			for(int i = 0; i < N; i++){
				asteroids[i].render(mCamera.GetWorldToClipMatrix());
			}

			if(!dead) {
				for(int i = 0; i < ship.size(); i++) {
					ship[i].render(mCamera.GetWorldToClipMatrix());
				}
			} else {
				glm::vec3 shipPos = glm::vec3(0, 2, -3);
				for (std::size_t i = 0; i < ship.size(); ++i) {
					// glm::vec3 shipPartPos = ship[i].get_transform().GetTranslation() + dir*shipSpeed;
					ship[i].get_transform().SetTranslate(shipPos);
					ship[i].render(mCamera.GetWorldToClipMatrix());
				}
				dead = false;
			}
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
			auto background_selection_result = program_manager.SelectProgram("Background", background_program_index);
			if (background_selection_result.was_selection_changed) {
				background.set_program(background_selection_result.program, set_uniforms);
			}
			ImGui::Separator();
			auto asteroid_selection_result = program_manager.SelectProgram("Asteroid", asteroid_program_index);
			if (asteroid_selection_result.was_selection_changed) {
				// asteroid.set_program(asteoid_selection_result.program, phong_set_uniforms);
				for(auto &ast : asteroids){
					ast.set_program(asteroid_selection_result.program, phong_set_uniforms);
				}
			}
			ImGui::Separator();
			ImGui::Checkbox("Use normal mapping", &use_normal_mapping);
			ImGui::ColorEdit3("Ambient", glm::value_ptr(ambient));
			ImGui::ColorEdit3("Diffuse", glm::value_ptr(diffuse));
			ImGui::ColorEdit3("Specular", glm::value_ptr(specular));
			ImGui::SliderFloat("Shininess", &shininess, 1.0f, 1000.0f);
			ImGui::SliderFloat3("Light Position", glm::value_ptr(light_position), -20.0f, 20.0f);
		}

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
		edaf80::Assignment5 assignment5(framework.GetWindowManager());
		assignment5.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}