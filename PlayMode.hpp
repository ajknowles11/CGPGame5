#include "Mode.hpp"

#include "Scene.hpp"
#include "WalkMesh.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	Scene::Drawable *mountain_mesh = nullptr;

	//player info:
	struct Player {
		WalkPoint at;
		//transform is at player's feet and will be yawed by mouse left/right motion:
		Scene::Transform *transform = nullptr;
		//ortho camera following player
		Scene::Camera *camera = nullptr;
		//camera base moves camera with player, allowing camera to ignore player rotation, 
		// and player to move without camera vertical rotation
		Scene::Transform *camera_base = nullptr;

		Scene::Transform *left_foot = nullptr;
		Scene::Transform *right_foot = nullptr;

		Scene::Drawable *base_mesh = nullptr;

		enum MountainZone {
			Front,
			Side,
			Top,
			Back
		} zone = Front;
	} player;

	std::vector<Scene::Transform*> fires;
	std::vector<Scene::Transform*> woods;

	std::vector<Scene::AnimatedDrawable *> flames;

	int num_placed = 0;

	float const walk_anim_time = 0.8f;
	float walk_anim_acc = 0;
	glm::quat const back_foot_rot = glm::angleAxis(-1.5f, glm::vec3(0,1,0));
	glm::quat const front_foot_rot = glm::angleAxis(1.0f, glm::vec3(0,1,0));
};
