#include "VisualDebugger.h"
#include <vector>
#include "Extras\Camera.h"
#include "Extras\Renderer.h"
#include "Extras\HUD.h"

namespace VisualDebugger
{
	using namespace physx;

	enum RenderMode
	{
		DEBUG,
		NORMAL,
		BOTH
	};

	enum HUDState
	{
		HELP = 0,
		EMPTY = 1,
		PAUSE = 2,
		GAMEOVER = 3
	};

	enum cameraPlacement
	{
		OVERHEAD = 0,
		PLAYER1 = 1,
		PLAYER2 = 2
	};

	//function declarations
	void KeyHold();
	void KeySpecial(int key, int x, int y);
	void KeyRelease(unsigned char key, int x, int y);
	void KeyPress(unsigned char key, int x, int y);

	void motionCallback(int x, int y);
	void mouseCallback(int button, int state, int x, int y);
	void exitCallback(void);

	void RenderScene();
	void ToggleRenderMode();
	void HUDInit();
	void cameraMove(enum cameraPlacement);

	///simulation objects
	Camera* camera;
	PhysicsEngine::MyScene* scene;
	PxReal delta_time = 1.f / 120.f;
	PxReal gForceStrength = 200;
	RenderMode render_mode = NORMAL;

	const int MAX_KEYS = 256;
	bool key_state[MAX_KEYS];

	bool hud_show = true;
	HUD hud;
	int score1 = 0, score2 = 0;
	bool gameOver, direction;
	cameraPlacement cameraState = cameraPlacement::OVERHEAD;


	//Init the debugger
	void Init(const char *window_name, int width, int height)
	{
		///Init PhysX
		PhysicsEngine::PxInit();
		scene = new PhysicsEngine::MyScene();
		scene->Init();


		///Init renderer
		Renderer::BackgroundColor(PxVec3(150.f / 255.f, 0.f / 255.f, 150.f / 255.f));
		Renderer::SetRenderDetail(40);
		Renderer::InitWindow(window_name, width, height);
		Renderer::Init();

		camera = new Camera(PxVec3(-15.0f, 100.0f, 15.0f), PxVec3(0.f, -1.f, -.1f), 25.f);


		//initialise HUD
		HUDInit();

		///Assign callbacks
		//render
		glutDisplayFunc(RenderScene);

		//keyboard
		glutKeyboardFunc(KeyPress);
		glutSpecialFunc(KeySpecial);
		glutKeyboardUpFunc(KeyRelease);

		//mouse
		glutMouseFunc(mouseCallback);
		glutMotionFunc(motionCallback);

		//exit
		atexit(exitCallback);

		//init motion callback
		motionCallback(0, 0);


		//making sure the player is the first and only actor that can be controlled
		string firstActor = scene->GetSelectedActor()->getName();
		srand((unsigned)time(0));
		int direction1 = (rand() % 2) - 1;
		int direction2 = (rand() % 2) - 1;
		if (firstActor == "Ball")
		{
			scene->GetSelectedActor()->addForce(PxVec3(direction1, 0, direction2)*gForceStrength);
		}

	}

	void HUDInit()
	{

		score1 = scene->scorePlayer1;
		score2 = scene->scorePlayer2;
		gameOver = scene->gameOver;
		hud.ActiveScreen(HELP);
		string score = "Player 1 Score: " + std::to_string(score1) + "                ICE HOCKEY                   Player 2 Score: " + std::to_string(score2);
		//initialise HUD
		//add an empty screen
		hud.Clear();
		hud.AddLine(EMPTY, "");
		//add a help screen
		hud.AddLine(EMPTY, " Simulation");
		hud.AddLine(EMPTY, "    F9 - select next actor");
		hud.AddLine(EMPTY, "    F10 - pause");
		hud.AddLine(EMPTY, "    F12 - reset");
		hud.AddLine(EMPTY, "");
		hud.AddLine(EMPTY, " Display");
		hud.AddLine(EMPTY, "    F5 - help on/off");
		hud.AddLine(EMPTY, "    F6 - shadows on/off");
		hud.AddLine(EMPTY, "    F7 - render mode");
		hud.AddLine(EMPTY, "");
		hud.AddLine(EMPTY, " Camera");
		hud.AddLine(EMPTY, "    W,S,A,D,Q,Z - forward,backward,left,right,up,down");
		hud.AddLine(EMPTY, "    mouse + click - change orientation");
		hud.AddLine(EMPTY, "    F8 - reset view");
		hud.AddLine(EMPTY, "");
		hud.AddLine(EMPTY, " Force (applied to the selected actor)");
		hud.AddLine(EMPTY, "    I,K,J,L,U,M - forward,backward,left,right,up,down");
		hud.AddLine(HELP, score);
		hud.AddLine(HELP, "");
		hud.AddLine(HELP, "");
		hud.AddLine(HELP, "");
		hud.AddLine(HELP, "F6 - Toggle Shadows, F9 - New Game");
		hud.AddLine(HELP, "F10 - Pause Game, F11 - Reset positions");
		//add a pause screen
		hud.AddLine(PAUSE, "   Simulation paused. Press F10 to continue.");

		//set font size for all screens
		hud.FontSize(0.025f);
		//set font color for all screens
		if (gameOver == true)
		{
			//scene->Pause(!scene->Pause());
			hud.ActiveScreen(GAMEOVER);
			scene->GetSelectedActor()->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, true);
			//hud.Clear();
			hud.AddLine(GAMEOVER, "");
			hud.AddLine(GAMEOVER, "         GAME OVER");
			if (score1 >= 5)
			{
				hud.AddLine(GAMEOVER, "       PLAYER 1 WINS");
			}
			if (score2 >= 5)
			{
				hud.AddLine(GAMEOVER, "       PLAYER 2 WINS");
			}
			else
			{

			}
			hud.FontSize(0.075f);

		}
		else
		{
			hud.ActiveScreen(HELP);
		}
		hud.Color(PxVec3(0.f, 0.f, 0.f));
	}

	//Start the main loop
	void Start()
	{
		glutMainLoop();
	}

	//Render the scene and perform a single simulation step
	void RenderScene()
	{
		//handle pressed keys
		KeyHold();

		//start rendering
		Renderer::Start(camera->getEye(), camera->getDir());

		if ((render_mode == DEBUG) || (render_mode == BOTH))
		{
			Renderer::Render(scene->Get()->getRenderBuffer());
		}

		if ((render_mode == NORMAL) || (render_mode == BOTH))
		{
			std::vector<PxActor*> actors = scene->GetAllActors();
			if (actors.size())
				Renderer::Render(&actors[0], (PxU32)actors.size());
		}

		//adjust the HUD state
		if (hud_show)
		{
			if (scene->Pause())
				hud.ActiveScreen(PAUSE);
			else
				hud.ActiveScreen(HELP);
		}
		else
			hud.ActiveScreen(EMPTY);

		direction = scene->direction;

		//render HUD
		HUDInit();
		hud.Render();

		cameraMove(cameraState);

		//finish rendering
		Renderer::Finish();

		//set forces to obstacle dpendant on what direction the object should be heading
		if (scene->direction == true)
		{
			scene->GetPlayer("Obstacle")->addForce(PxVec3(0.f, 0.f, 1.f)*gForceStrength / 10);
		}
		else if (scene->direction == false)
		{
			scene->GetPlayer("Obstacle")->addForce(PxVec3(0.f, 0.f, -1.f)*gForceStrength / 10);
		}

		//perform a single simulation step
		scene->Update(delta_time);
	}

	//user defined keyboard handlers
	void UserKeyPress(int key)
	{
		switch (toupper(key))
		{
			//implement your own
		case 49:
			cameraState = cameraPlacement::PLAYER1;
			cout << "Changing camera position to player 1" << endl;
			break;
		case 50:
			cameraState = cameraPlacement::PLAYER2;
			cout << "Changing camera position to player 2" << endl;
			break;
		case 32:
			cameraState = cameraPlacement::OVERHEAD;
			cout << "Changing camera position to Vverhead" << endl;
			break;
		default:
			break;
		}
	}

	//Changes camera positioning to be either overhead or attached to player 1 or 2 depending on key presses
	void cameraMove(enum cameraPlacement cameraState)
	{
		if (cameraState == cameraPlacement::PLAYER1)
		{
			camera->setEye(scene->GetPlayer("Player 2")->getGlobalPose().p, 3.f);
			camera->setDir(PxVec3(1.f, 0.f, .1f), cameraPlacement::PLAYER1);
		}
		if (cameraState == cameraPlacement::PLAYER2)
		{
			camera->setEye(scene->GetPlayer("Player 1")->getGlobalPose().p, 3.f);
			camera->setDir(PxVec3(-1.f, 0.f, -.1f), cameraPlacement::PLAYER2);
		}
		if (cameraState == cameraPlacement::OVERHEAD)
		{
			camera->setEye(PxVec3(-15.0f, 100.0f, 15.0f), 0.f);
			camera->setDir(PxVec3(0.f, -1.f, -.1f), cameraPlacement::OVERHEAD);
		}
	}

	void UserKeyHold(int key)
	{
	}

	//obsolete function due to change/removal of camera control
	void CameraInput(int key)
	{
		switch (toupper(key))
		{
		case 'I':
			camera->MoveForward(delta_time);
			break;
		case 'K':
			camera->MoveBackward(delta_time);
			break;
		case 'J':
			camera->MoveLeft(delta_time);
			break;
		case 'L':
			camera->MoveRight(delta_time);
			break;
		case 'Q':
			camera->MoveUp(delta_time);
			break;
		case 'Z':
			camera->MoveDown(delta_time);
			break;
		default:
			break;
		}
	}

	//handle force control keys
	void ForceInput(int key)
	{
		string playerChoice1, playerChoice2;

		if (score1 < 5 && score2 < 5)
		{
			switch (key)
			{
				//right hand side player (actually player 2)
			case 54: //KEYPAD 6
				//change movement axis dependant on whether the camera is first person view or not
				if (cameraState == cameraPlacement::PLAYER1 || cameraState == cameraPlacement::PLAYER2)
				{
					scene->GetPlayer("Player 1")->addForce(PxVec3(0, 0, -1)*gForceStrength);
				}
				else
				{
					scene->GetPlayer("Player 1")->addForce(PxVec3(1, 0, 0)*gForceStrength);
				}
				break;
				//move left
			case 52: //KEYPAD 4
				//change movement axis dependant on whether the camera is first person view or not
				if (cameraState == cameraPlacement::PLAYER1 || cameraState == cameraPlacement::PLAYER2)
				{
					scene->GetPlayer("Player 1")->addForce(PxVec3(0, 0, 1)*gForceStrength);
				}
				else
				{
					scene->GetPlayer("Player 1")->addForce(PxVec3(-1, 0, 0)*gForceStrength);
				}
				break;
				//down
			case 56:
				//change movement axis dependant on whether the camera is first person view or not
				if (cameraState == cameraPlacement::PLAYER1 || cameraState == cameraPlacement::PLAYER2)
				{
					scene->GetPlayer("Player 1")->addForce(PxVec3(-1, 0, 0)*gForceStrength); // KEYPAD 8 - forward
				}
				else
				{
					scene->GetPlayer("Player 1")->addForce(PxVec3(0, 0, -1)*gForceStrength);
				}
				break;
				//up
			case 53: // KEYPAD 5
				//change movement axis dependant on whether the camera is first person view or not
				if (cameraState == cameraPlacement::PLAYER1 || cameraState == cameraPlacement::PLAYER2)
				{
					scene->GetPlayer("Player 1")->addForce(PxVec3(1, 0, 0)*gForceStrength);
				}
				else
				{
					scene->GetPlayer("Player 1")->addForce(PxVec3(0, 0, 1)*gForceStrength);
				}
				break;

				/*-----------------Player 1 (left hand side player)-----------------*/
				// Force controls on the selected actor
			case 119: //forward
				//change movement axis dependant on whether the camera is first person view or not
				if (cameraState == cameraPlacement::PLAYER1 || cameraState == cameraPlacement::PLAYER2)
				{
					scene->GetPlayer("Player 2")->addForce(PxVec3(1, 0, 0)*gForceStrength);
				}
				else
				{
					scene->GetPlayer("Player 2")->addForce(PxVec3(0, 0, -1)*gForceStrength);
				}
				break;
			case 115: //backward
				//change movement axis dependant on whether the camera is first person view or not
				if (cameraState == cameraPlacement::PLAYER1 || cameraState == cameraPlacement::PLAYER2)
				{
					scene->GetPlayer("Player 2")->addForce(PxVec3(-1, 0, 0)*gForceStrength);
				}
				else
				{
					scene->GetPlayer("Player 2")->addForce(PxVec3(0, 0, 1)*gForceStrength);
				}
				break;
			case 97: //left
				//change movement axis dependant on whether the camera is first person view or not
				if (cameraState == cameraPlacement::PLAYER1 || cameraState == cameraPlacement::PLAYER2)
				{
					scene->GetPlayer("Player 2")->addForce(PxVec3(0, 0, -1)*gForceStrength);
				}
				else
				{
					scene->GetPlayer("Player 2")->addForce(PxVec3(-1, 0, 0)*gForceStrength);
				}
				break;
			case 100: //right
				//change movement axis dependant on whether the camera is first person view or not
				if (cameraState == cameraPlacement::PLAYER1 || cameraState == cameraPlacement::PLAYER2)
				{
					scene->GetPlayer("Player 2")->addForce(PxVec3(0, 0, 1)*gForceStrength);
				}
				else
				{
					scene->GetPlayer("Player 2")->addForce(PxVec3(1, 0, 0)*gForceStrength);
				}
				break;
			default:
				break;
			}
		}
	}


	///handle special keys
	void KeySpecial(int key, int x, int y)
	{
		string objectName;
		//simulation control
		switch (key)
		{
			//display control
		case GLUT_KEY_F5:
			//hud on/off
			hud_show = !hud_show;
			break;
		case GLUT_KEY_F6:
			//shadows on/off
			Renderer::ShowShadows(!Renderer::ShowShadows());
			break;
		case GLUT_KEY_F7:
			//toggle render mode
			//ToggleRenderMode();
			break;
		case GLUT_KEY_F8:
			//reset camera view
			camera->Reset();
			break;
			//simulation control
		case GLUT_KEY_F10:
			//toggle scene pause
			scene->Pause(!scene->Pause());
			break;
		case GLUT_KEY_F9:
			scene->Reset();
			//scene->newGame();
			HUDInit();
			break;
		case GLUT_KEY_F11:
			scene->newGame();
			break;
		default:
			break;
		}
	}

	//handle single key presses
	void KeyPress(unsigned char key, int x, int y)
	{
		//do it only once
		if (key_state[key] == true)
			return;

		key_state[key] = true;

		//exit
		if (key == 27)
			exit(0);

		UserKeyPress(key);
	}

	//handle key release
	void KeyRelease(unsigned char key, int x, int y)
	{
		key_state[key] = false;
	}

	//handle holded keys
	void KeyHold()
	{
		for (int i = 0; i < MAX_KEYS; i++)
		{
			if (key_state[i]) // if key down
			{
				CameraInput(i);
				ForceInput(i);
				UserKeyHold(i);
			}
		}
	}

	///mouse handling
	int mMouseX = 0;
	int mMouseY = 0;

	void motionCallback(int x, int y)
	{
		int dx = mMouseX - x;
		int dy = mMouseY - y;

		camera->Motion(dx, dy, delta_time);

		mMouseX = x;
		mMouseY = y;
	}

	void mouseCallback(int button, int state, int x, int y)
	{
		mMouseX = x;
		mMouseY = y;
	}

	void ToggleRenderMode()
	{
		if (render_mode == NORMAL)
			render_mode = DEBUG;
		else if (render_mode == DEBUG)
			render_mode = BOTH;
		else if (render_mode == BOTH)
			render_mode = NORMAL;
	}

	///exit callback
	void exitCallback(void)
	{
		delete camera;
		delete scene;
		PhysicsEngine::PxRelease();
	}
}

