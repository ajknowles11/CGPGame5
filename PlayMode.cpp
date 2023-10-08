#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"
#include "VFXProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <random>

GLuint mountain_meshes_for_lit_color_texture_program = 0;
GLuint mountain_meshes_for_vfx_program = 0;
Load< MeshBuffer > mountain_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("mountain.pnct"));
	mountain_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	mountain_meshes_for_vfx_program = ret->make_vao_for_program(vfx_program->program);
	return ret;
});

Load< Scene::Texture > player_texture(LoadTagDefault, []() -> Scene::Texture const * {
	return new Scene::Texture(data_path("player_texture.png"));
});

Load< Scene::Texture > mountain_texture(LoadTagDefault, []() -> Scene::Texture const * {
	return new Scene::Texture(data_path("mountain_texture.png"));
});

Load< Scene::Texture > flame_texture(LoadTagDefault, []() -> Scene::Texture const * {
	return new Scene::Texture(data_path("fire.png"));
});

Load< Scene > mountain_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("mountain.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = mountain_meshes->lookup(mesh_name);
		
		if (mesh_name.substr(0,5) == "Flame") {
			scene.transparents.emplace_back(transform);
			Scene::Drawable &drawable = scene.transparents.back();

			drawable.pipeline = vfx_program_pipeline;

			drawable.pipeline.vao = mountain_meshes_for_vfx_program;
			drawable.pipeline.type = mesh.type;
			drawable.pipeline.start = mesh.start;
			drawable.pipeline.count = mesh.count;
		}
		else {
			scene.drawables.emplace_back(transform);
			Scene::Drawable &drawable = scene.drawables.back();

			drawable.pipeline = lit_color_texture_program_pipeline;

			drawable.pipeline.vao = mountain_meshes_for_lit_color_texture_program;
			drawable.pipeline.type = mesh.type;
			drawable.pipeline.start = mesh.start;
			drawable.pipeline.count = mesh.count;
		}

	});
});

WalkMesh const *walkmesh = nullptr;
Load< WalkMeshes > mountain_walkmeshes(LoadTagDefault, []() -> WalkMeshes const * {
	WalkMeshes *ret = new WalkMeshes(data_path("mountain.w"));
	walkmesh = &ret->lookup("WalkMesh");
	return ret;
});

PlayMode::PlayMode() : scene(*mountain_scene) {

	for (auto &transform : scene.transforms) {
		if (transform.name == "Player") {
			player.transform = &transform;
		}
		else if (transform.name == "CamBase") {
			player.camera_base = &transform;
		}
		else if (transform.name == "LFoot") {
			player.left_foot = &transform;
		}
		else if (transform.name == "RFoot") {
			player.right_foot = &transform;
		}
		else if (transform.name.substr(0, 4) == "Wood") {
			woods.emplace_back(&transform);
		}
		else if (transform.name.substr(0, 4) == "Fire") {
			fires.emplace_back(&transform);
			transform.scale = glm::vec3(0);
		}

	}

	for (auto &drawable : scene.drawables) {
		if (drawable.transform->name == "Player") {
			player.base_mesh = &drawable;
		}
		else if (drawable.transform->name == "Landscape") {
			mountain_mesh = &drawable;
		}
	}
	for (auto &vfx : scene.transparents) {
		if (vfx.transform->name.substr(0, 5) == "Flame") { // I actually know this is always true since nothing else is transparent, but whatever
			flames.emplace_back(&vfx);
		}
	}

	{ //player tex
		GLuint tex = 0;
		glGenTextures(1, &tex);

		glBindTexture(GL_TEXTURE_2D, tex);
		player.base_mesh->pipeline.textures->texture = tex;
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, player_texture->size.x, player_texture->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, player_texture->pixels.data());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	{ //mountain tex
		GLuint tex = 0;
		glGenTextures(1, &tex);

		glBindTexture(GL_TEXTURE_2D, tex);
		mountain_mesh->pipeline.textures->texture = tex;
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mountain_texture->size.x, mountain_texture->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, mountain_texture->pixels.data());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	{ //fire tex
		for (auto &flame : flames) {	
			GLuint tex = 0;
			glGenTextures(1, &tex);

			glBindTexture(GL_TEXTURE_2D, tex);
			flame->pipeline.textures->texture = tex;
			
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, flame_texture->size.x, flame_texture->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, flame_texture->pixels.data());
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);

			flame->Frame_Offset_vec2 = vfx_program->FrameOffset_vec2;
			flame->start_loop_frame = 1;
			flame->num_frames = 6;
			flame->frame_time = 0.1f;
			flame->per_frame_offset = glm::vec2(1.0f/6.0f, 0.0f);
		}
	}

	//create a player camera attached to a child of the player transform:
	assert(scene.cameras.size() > 0);
	player.camera = &scene.cameras.back();

	//start player walking at nearest walk point:
	player.at = walkmesh->nearest_walk_point(flames[0]->transform->make_local_to_world() * glm::vec4(flames[0]->transform->position, 1));

}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		return true;
	} 

	return false;
}

void PlayMode::update(float elapsed) {
	//player walking:
	{
		//combine inputs into a move:
		constexpr float PlayerSpeed = 15.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		//get move in world coordinate system:
		glm::vec3 remain = player.camera_base->make_local_to_world() * glm::vec4(move.x, move.y, 0.0f, 0.0f);

		if (remain != glm::vec3(0.0f)) {

			//rotate player to face movement direction
			{
				constexpr float PlayerRotateSpeed = 10.0f;

				float yaw = glm::roll(player.transform->rotation);
				float yaw_target = glm::roll(glm::rotation(glm::vec3(-1,0,0), glm::normalize(remain)));
				if (glm::abs(yaw_target - yaw) > glm::pi<float>()) {
					if (yaw < 0) yaw_target -= 2 * glm::pi<float>();
					else yaw_target += 2 * glm::pi<float>(); 
				}
				float const &alpha = glm::clamp((PlayerRotateSpeed * elapsed) / glm::abs(yaw_target - yaw), 0.0f, 1.0f);
				if (alpha >= 0 && alpha <= 1) {
					float const &yaw_new = glm::mix(yaw, yaw_target, alpha);
					player.transform->rotation = glm::angleAxis(yaw_new, glm::vec3(0,0,1));
				}
			}

			//advance walk anim
			walk_anim_acc += elapsed;

		}
		else {
			//reset walk anim
			if (walk_anim_acc > 0) {
				if (walk_anim_acc >= 0.75f * walk_anim_time) {
					walk_anim_acc += elapsed;
					if (walk_anim_acc >= walk_anim_time) {
						walk_anim_acc = 0;
					}
				}
				else if (walk_anim_acc >= 0.5f * walk_anim_time) {
					walk_anim_acc -= elapsed;
					if (walk_anim_acc <= 0.5f * walk_anim_time) {
						walk_anim_acc = 0;
					}
				}
				if (walk_anim_acc >= 0.25f * walk_anim_time) {
					walk_anim_acc += elapsed;
					if (walk_anim_acc >= 0.5f * walk_anim_time) {
						walk_anim_acc = 0;
					}
				}
				else if (walk_anim_acc > 0) {
					walk_anim_acc -= elapsed;
					if (walk_anim_acc < 0) {
						walk_anim_acc = 0;
					}
				}
			}
		}

		//move feet
		{
			while (walk_anim_acc > walk_anim_time) {
				walk_anim_acc -= walk_anim_time;
			}

			float alpha = walk_anim_acc / walk_anim_time;
			if (alpha >= 0.75f) {
				alpha = 2.0f * (alpha - 0.75f);
			}
			else if (alpha >= 0.25f) {
				alpha = 1 - 2.0f * (alpha - 0.25f);
			}
			else {
				alpha = 2.0f * alpha + 0.5f;
			}
			player.left_foot->rotation = glm::slerp(back_foot_rot, front_foot_rot, alpha);
			player.right_foot->rotation = glm::slerp(back_foot_rot, front_foot_rot, 1-alpha);
		}
		

		//using a for() instead of a while() here so that if walkpoint gets stuck in
		// some awkward case, code will not infinite loop:
		for (uint32_t iter = 0; iter < 10; ++iter) {
			if (remain == glm::vec3(0.0f)) break;
			WalkPoint end;
			float time;
			walkmesh->walk_in_triangle(player.at, remain, &end, &time);
			player.at = end;
			if (time == 1.0f) {
				//finished within triangle:
				remain = glm::vec3(0.0f);
				break;
			}
			//some step remains:
			remain *= (1.0f - time);
			//try to step over edge:
			glm::quat rotation;
			if (walkmesh->cross_edge(player.at, &end, &rotation)) {
				//stepped to a new triangle:
				player.at = end;
				//rotate step to follow surface:
				remain = rotation * remain;
			} else {
				//ran into a wall, bounce / slide along it:
				glm::vec3 const &a = walkmesh->vertices[player.at.indices.x];
				glm::vec3 const &b = walkmesh->vertices[player.at.indices.y];
				glm::vec3 const &c = walkmesh->vertices[player.at.indices.z];
				glm::vec3 along = glm::normalize(b-a);
				glm::vec3 normal = glm::normalize(glm::cross(b-a, c-a));
				glm::vec3 in = glm::cross(normal, along);

				//check how much 'remain' is pointing out of the triangle:
				float d = glm::dot(remain, in);
				if (d < 0.0f) {
					//bounce off of the wall:
					remain += (-1.25f * d) * in;
				} else {
					//if it's just pointing along the edge, bend slightly away from wall:
					remain += 0.01f * d * in;
				}
			}
		}

		constexpr float CameraRotateSpeed = 4.0f;
		glm::quat target;
		if (player.transform->position.z >= 30 && glm::abs(player.transform->position.x) < 24) {
			player.zone = player.Top;
			target = glm::angleAxis(glm::pi<float>()/4.0f - 0.1f, glm::vec3(0,0,1)) *
											glm::angleAxis(-0.1f, glm::vec3(1,0,0));
		}
		else if (player.transform->position.x < -30 && glm::abs(player.transform->position.y) < 60 ) {
			player.zone = player.Side;
			target = glm::angleAxis(5.0f*glm::pi<float>()/4.0f, glm::vec3(0,0,1));
		}
		else if (player.transform->position.y > 0) {
			player.zone = player.Back;
			target = glm::angleAxis(3.0f*glm::pi<float>()/4.0f, glm::vec3(0,0,1));
		}
		else {
			player.zone = player.Front;
			target = glm::angleAxis(-glm::pi<float>()/4.0f, glm::vec3(0,0,1));
		}

		float const &rot_alpha = glm::clamp(CameraRotateSpeed * elapsed / glm::abs(glm::roll(target) - glm::roll(player.camera_base->rotation)), 0.0f, 1.0f);
		if (rot_alpha > 0) {
			player.camera_base->rotation = glm::slerp(player.camera_base->rotation, target, rot_alpha);
		}

		float const &scale_alpha = glm::clamp((player.transform->position.z - 10.0f) / 20.0f, 0.0f, 1.0f);
		player.camera->scale = glm::mix(30.0f, 70.0f, scale_alpha);

		// if (remain != glm::vec3(0.0f)) {
		// 	std::cout << "NOTE: code used full iteration budget for walking." << std::endl;
		// }

		//update player's position to respect walking:
		player.transform->position = walkmesh->to_world_point(player.at);
		player.camera_base->position = player.transform->position;
	}

	constexpr float fire_radius = 2.0f;
	for (size_t i = 0; i < fires.size(); i++) {
		if (flames[i]->visited) continue;
		if (glm::distance(player.transform->position, fires[i]->make_local_to_world() * glm::vec4(fires[i]->position,1.0f)) < fire_radius) {
			fires[i]->scale = glm::vec3(1.0f);
			flames[i]->visited = true;

			if (num_placed < 6) {
				woods[num_placed]->scale = glm::vec3(0);
				woods[num_placed+1]->scale = glm::vec3(0);
			}
			else {
				woods[num_placed]->scale = glm::vec3(0);
			}

			num_placed += 2;
		}
	}

	{ //animate fire
		for (auto &flame : flames) {
			if (flame->visited) {
				flame->anim_time_acc += elapsed;
				while (flame->anim_time_acc >= flame->num_frames * flame->frame_time) {
					flame->anim_time_acc -= (flame->num_frames - flame->start_loop_frame) * flame->frame_time;
				}

				//rotate to face camera
				flame->transform->rotation = glm::mat4(flame->transform->parent->make_world_to_local()) * glm::mat4(glm::angleAxis(glm::roll(player.camera_base->rotation), glm::vec3(0,0,1)));
			}
		}
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	player.camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(scene.drawables, *player.camera);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	scene.draw(scene.transparents, *player.camera);

	glDisable(GL_BLEND);

	/* In case you are wondering if your walkmesh is lining up with your scene, try:
	{
		glDisable(GL_DEPTH_TEST);
		DrawLines lines(player.camera->make_projection() * glm::mat4(player.camera->transform->make_world_to_local()));
		for (auto const &tri : walkmesh->triangles) {
			lines.draw(walkmesh->vertices[tri.x], walkmesh->vertices[tri.y], glm::u8vec4(0x88, 0x00, 0xff, 0xff));
			lines.draw(walkmesh->vertices[tri.y], walkmesh->vertices[tri.z], glm::u8vec4(0x88, 0x00, 0xff, 0xff));
			lines.draw(walkmesh->vertices[tri.z], walkmesh->vertices[tri.x], glm::u8vec4(0x88, 0x00, 0xff, 0xff));
		}
	}
	*/

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("WASD to move.",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("WASD to move.",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}
