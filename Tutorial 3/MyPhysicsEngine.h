#pragma once

#include "BasicActors.h"
#include <iostream>
#include <iomanip>

namespace PhysicsEngine
{
	using namespace std;

	//a list of colours: Circus Palette
	static const PxVec3 color_palette[] = { PxVec3(46.f / 255.f,9.f / 255.f,39.f / 255.f),PxVec3(217.f / 255.f,0.f / 255.f,0.f / 255.f),
		PxVec3(255.f / 255.f,45.f / 255.f,0.f / 255.f),PxVec3(255.f / 255.f,140.f / 255.f,54.f / 255.f),PxVec3(4.f / 255.f,117.f / 255.f,111.f / 255.f) };

	struct FilterGroup
	{
		enum Enum
		{
			Sphere = (1 << 0),
			GoalPlayer1 = (1 << 1),
			GoalPlayer2 = (1 << 2)
		};
	};

	///An example class showing the use of springs (distance joints).
	class Trampoline
	{
		vector<DistanceJoint*> springs;
		Boxes *bottom, *top;
		Box* box;

	public:
			bool x, y, z;
		Trampoline(const PxVec3& dimensions = PxVec3(3.f, 1.f, 3.f), PxReal stiffness = 20.f, PxReal damping = 20.f)
		{
			PxVec3 position(-60.f, 0.5f, -20.f);
			PxReal thickness = .1f;
			top = new Boxes(PxTransform(PxVec3(position), PxQuat((PxPi / 2), PxVec3(0.f, 0.f, 1.f))), PxVec3(dimensions.x, thickness, dimensions.z));
			bottom = new Boxes(PxTransform(PxVec3(position), PxQuat((PxPi / 2), PxVec3(0.f, 0.f, 1.f))), PxVec3(dimensions.x, thickness, dimensions.z));
			box = new Box();
			springs.resize(4);
			springs[0] = new DistanceJoint(bottom, PxTransform(PxVec3(dimensions.x, thickness, dimensions.z)), top, PxTransform(PxVec3(dimensions.x, -dimensions.y, dimensions.z)));
			springs[1] = new DistanceJoint(bottom, PxTransform(PxVec3(dimensions.x, thickness, -dimensions.z)), top, PxTransform(PxVec3(dimensions.x, -dimensions.y, -dimensions.z)));
			springs[2] = new DistanceJoint(bottom, PxTransform(PxVec3(-dimensions.x, thickness, dimensions.z)), top, PxTransform(PxVec3(-dimensions.x, -dimensions.y, dimensions.z)));
			springs[3] = new DistanceJoint(bottom, PxTransform(PxVec3(-dimensions.x, thickness, -dimensions.z)), top, PxTransform(PxVec3(-dimensions.x, -dimensions.y, -dimensions.z)));

			for (unsigned int i = 0; i < springs.size(); i++)
			{
				springs[i]->Stiffness(stiffness);
				springs[i]->Damping(damping);
			}
		}

		void AddToScene(Scene* scene)
		{
			scene->Add(bottom);
			scene->Add(top);
			scene->Add(box);
			top->GetShape(0)->setLocalPose(PxTransform(PxVec3(2.f, 0.5f, 0.0f), PxQuat((PxPi / 2), PxVec3(0.f, 0.f, 1.f))));
			bottom->GetShape(0)->setLocalPose(PxTransform(PxVec3(-0.f, 1.5f, 0.0f), PxQuat((PxPi / 2), PxVec3(0.f, 0.f, 1.f))));
			box->GetShape(0)->getActor()->setGlobalPose(PxTransform(PxVec3(-63.f, 0.5f, -20.5f)));
			D6Joint* trampolineJoint = new D6Joint(box, PxTransform(PxVec3(2.f, 0.5f, 0.0f)), bottom, PxTransform(PxVec3(-2.f, 0.0f, 0.0f), PxQuat(PxPi/4.25, PxVec3(0.f, 1.f, 0.f))), x = false, y = false, z = false);
		}

		~Trampoline()
		{
			for (unsigned int i = 0; i < springs.size(); i++)
				delete springs[i];
		}
	};

	///A customised collision class, implemneting various callbacks
	class MySimulationEventCallback : public PxSimulationEventCallback
	{
	public:
		//MyScene* myscene;
		//an example variable that will be checked in the main simulation loop
		bool isOver = false, trigger, gameOver = false, direction = false; 
		int scorePlayer1 = 0, scorePlayer2 = 0;

		MySimulationEventCallback() : trigger(false) {}

		///Method called when the contact with the trigger object is detected.
		virtual void onTrigger(PxTriggerPair* pairs, PxU32 count)
		{
			string obstacleTrigger = "Obstacle Trigger Top", obstacleTrigger2 = "Obstacle Trigger Bottom";
			//string goalTrigger1 = "Goal1", goalTrigger2 = "Goal2";
			//you can read the trigger information here
			for (PxU32 i = 0; i < count; i++)
			{
				//filter out contact with the planes
				if (pairs[i].otherShape->getGeometryType() != PxGeometryType::ePLANE)
				{
					//check if eNOTIFY_TOUCH_FOUND trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
					{
						if (pairs[i].otherShape->getGeometryType() == PxGeometryType::eBOX)
						{
							if (pairs[i].triggerActor->getName() == obstacleTrigger)
							{
								direction = true;
								cout << "Obstacle collided with trigger, changing direction" << endl;
							}
							if (pairs[i].triggerActor->getName() == obstacleTrigger2)
							{
								direction = false;
								cout << "Obstacle collided with trigger, changing direction" << endl;
							}
						}
					}
				}
			}
		}

		///Method called when the contact by the filter shader is detected.
		virtual void onContact(const PxContactPairHeader &pairHeader, const PxContactPair *pairs, PxU32 nbPairs)
		{
			string goal1 = "Goal1", goal2 = "Goal2";
			cerr << "Contact found between " << pairHeader.actors[0]->getName() << " " << pairHeader.actors[1]->getName() << endl;

			//check all pairs
			for (PxU32 i = 0; i < nbPairs; i++)
			{
				//check eNOTIFY_TOUCH_FOUND
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
				{
					if (pairHeader.actors[1]->getName() == goal1)
					{
						scorePlayer2++;
						//cerr << "onContact::eNOTIFY_TOUCH_FOUND TRIGGER 1" << endl;
					}
					if (pairHeader.actors[1]->getName() == goal2)
					{
						scorePlayer1++;
						//cerr << "onContact::eNOTIFY_TOUCH_FOUND TRIGGER 2" << endl;
					}
				}
				//check eNOTIFY_TOUCH_LOST
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_LOST)
				{
					cerr << "onContact::eNOTIFY_TOUCH_LOST" << endl;
				}
				if (scorePlayer1 >= 1 || scorePlayer2 >= 1)
				{
					isOver = true;
				}
			}
			endGame();
		}

		void endGame()
		{
			if (scorePlayer1 >= 5 || scorePlayer2 >= 5)
			{
				gameOver = true;
			}
		}

		virtual void onConstraintBreak(PxConstraintInfo *constraints, PxU32 count) {}
		virtual void onWake(PxActor **actors, PxU32 count) {}
		virtual void onSleep(PxActor **actors, PxU32 count) {}
	};

	//A simple filter shader based on PxDefaultSimulationFilterShader - without group filtering
	static PxFilterFlags CustomFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		// let triggers through
		if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlags();
		}

		pairFlags = PxPairFlag::eCONTACT_DEFAULT;


			// trigger the contact callback for pairs (A,B) where 
			// the filtermask of A contains the ID of B and vice versa.
		if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		{
			//trigger onContact callback for this pair of objects
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
			//			pairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;
		}

		return PxFilterFlags();
	};

	///Custom scene class
	class MyScene : public Scene
	{
		MySimulationEventCallback* my_callback;
		//actor classes instantiation
		Plane* plane;
		Box* box, *box2, *obstacleTrigger1, *obstacleTrigger2;
		Obstacle* obstacle;
		OutSkirtsSides* outskirts;
		PxMaterial* playerMaterial;
		Flipper* flipper, *flipper2;
		Goals* goalTrigger1, *goalTrigger2;
		MotorArms* motorArms;
		bool x, y, z;

	public:

		MyScene() : Scene() { filter_shader = CustomFilterShader; };

		Player* player1, *player2;
		Sphere* sphere;
		int scorePlayer1, scorePlayer2; 
		bool gameOver, direction;

		void SetVisualisation()
		{
			px_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eBODY_AXES, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.f);
		}

		//scene initialisation (only called once unless called again elsewhere)
		virtual void CustomInit()
		{
			SetVisualisation();
			///Initialise and set the customised event callback
			my_callback = new MySimulationEventCallback();
			px_scene->setSimulationEventCallback(my_callback);
			

			//Initialise scores when the scene is created
			scorePlayer1 = 0, scorePlayer2 = 0;


			/*--------------------------------------------------Plane-----------------------------------------------------
			--------------------------Adds plane to scene and colours it blue to indicate ice---------------------------*/
			plane = new Plane();
			plane->Color(PxVec3(50.f / 255.f, 211.f / 255.f, 255.f / 255.f));
			Add(plane);
			

			/*---------------------------------------------------Ball-----------------------------------------------------
			-----------Dynamic flipper object which will cause the ball to rebound when it collides with this-------------
			---------------------Set material of shape so it keeps the same speed whilst rolling--------------------------
			------------Slightly increase the speed of any actor that comes into contact with the boundaries--------------
			---------------------Set name of shape so it can be compared in filter group contact--------------------------
			-------------------Set filtergroup of shape so it can be trigger contact with both goals----------------------
			----------------------------------------------Add shape to scene---------------------------------------------*/
			sphere = new Sphere(PxTransform(PxVec3(-20.f, 1.f, 0.f)));
			PxMaterial* ballMaterial = CreateMaterial(0.f, 0.f, 1.f);
			sphere->Material(ballMaterial);
			sphere->Name("Ball");
			sphere->Color(PxVec3(0 / 255, 255 / 255, 255 / 255));
			sphere->SetupFiltering(FilterGroup::Sphere, FilterGroup::GoalPlayer1 | FilterGroup::GoalPlayer2);
			//sphere->SetKinematic(true);
			Add(sphere);


			/*---------------------------------------------------Flipper--------------------------------------------------
			-----------Dynamic flipper object which will cause the ball to rebound when it collides with this-------------
			-------------Slightly increase the speed of any actor that comes into contact with the boundaries-------------
			----------------------------------------------Add shape to scene---------------------------------------------*/
			PxMaterial* flipperMaterial = CreateMaterial(0.2f, 0.f, 1.45f);
			flipper = new Flipper(PxTransform(PxVec3(0.f, 0.f, 0.5f), PxQuat(PxPi / 2, PxVec3(1.f, 0.f, 0.f))));
			flipper->Material(flipperMaterial);
			flipper->Name("Flipper");
			Add(flipper);


			/*-------------------------------------------------Anchor Box------------------------------------------------
			---------------------Adds a box as an anchor for the flipper joint inside the boundaries--------------------*/
			box = new Box(PxTransform(PxVec3(2.f, 3.f, -25.0f)));
			Add(box);


			/*-------------------------------------------------Boundaries-------------------------------------------------
			------------COMPOUND STATIC OBJECT using 4 roatated cuboids where opposite objects are identical--------------
			------------Creates boundaries for the game, to stop any objects going outside of the playable area-----------
			--Decreases the speed of any actor that comes into contact with the boundaries, to dull down fast movements---
			------------------------------Shape has a unique and distinguishable colour (green)---------------------------
			----------------------------------------------Add shape to scene---------------------------------------------*/
			outskirts = new OutSkirtsSides();
			PxMaterial* boundariesMaterial = CreateMaterial(0.0f, 0.f, 1.f);
			outskirts->Material(boundariesMaterial);
			outskirts->Color(PxVec3(0 / 255, 255 / 255, 0 / 255));
			Add(outskirts);
			

			/*--------------------------------------------------Player 1--------------------------------------------------
			--------------------Creates new player, translates it to the right hand side of the screen--------------------
			--Makes sure the shape DOESN'T have a material which speeds up the actor when it collides (material setting)--
			------------------------------Shape has a unique and distinguishable colour (blue)----------------------------
			-------Sets the name of shape which can be used to check trigger events and movement in Visual Debugger-------
			----------------------------------------------Add shape to scene---------------------------------------------*/
			player1 = new Player(PxTransform(PxVec3(30.f, 0.f, 0.15f)));
			playerMaterial = CreateMaterial(0.f, 0.f, 0.f);
			player1->Material(playerMaterial);
			player1->Name("Player 1");
			player1->GetShape()->getActor()->setName("Player 1");
			player1->Color(PxVec3(0 / 255.f, 0 / 255.f, 255 / 255.f));
			Add(player1);


			/*--------------------------------------------------Player 2--------------------------------------------------
			--------------------Creates new player, translates it to the left hand side of the screen---------------------
			--Makes sure the shape doesn't have a material which speeds up the actor when it collides (material setting)--
			------------------------------Shape has a unique and distinguishable colour (red)-----------------------------
			-------Sets the name of shape which can be used to check trigger events and movement in Visual Debugger-------
			----------------------------------------------Add shape to scene---------------------------------------------*/
			player2 = new Player(PxTransform(PxVec3(-60.0f, 0.0f, 0.15f)));
			player2->Material(playerMaterial);
			player2->Color(PxVec3(255 / 255.f, 0 / 255.f, 0 / 255.f));
			player2->Name("Player 2");
			player2->GetShape()->getActor()->setName("Player 2");
			Add(player2);


			/*---------------------------------------------------Goal 1---------------------------------------------------
			--------------------Creates new goal trigger, translates it to the right hand side of the screen--------------
			-------------Sets object to be a goal filter group that interacts with the ball to increase score-------------
			--Sets the name of shape and actor which can be used to check trigger events and movement in Visual Debugger--
			----------------------------------------------Add shape to scene---------------------------------------------*/
			goalTrigger1 = new Goals();
			//goalTrigger1->GetShape()->setFlag(PxShapeFlag::)
			//goalTrigger1->GetShape()->getActor()->isRigidDynamic()->putToSleep();
			goalTrigger1->SetupFiltering(FilterGroup::GoalPlayer1, FilterGroup::Sphere);
			goalTrigger1->Color(PxVec3(255 / 255, 255 / 255, 255 / 255));
			goalTrigger1->Name("Goal1");
			goalTrigger1->GetShape()->getActor()->setName("Goal1");
			Add(goalTrigger1);


			/*---------------------------------------------------Goal 2---------------------------------------------------
			--------------------Creates new goal trigger, translates it to the right hand side of the screen--------------
			-------------Sets object to be a goal filter group that interacts with the ball to increase score-------------
			--Sets the name of shape and actor which can be used to check trigger events and movement in Visual Debugger--
			----------------------------------------------Add shape to scene---------------------------------------------*/
			goalTrigger2 = new Goals();
			goalTrigger2->GetShape()->getActor()->setGlobalPose(PxTransform(PxVec3(99.f, 0.f, .0f)));
			//goalTrigger2->SetTrigger(true);
			goalTrigger2->SetupFiltering(FilterGroup::GoalPlayer2, FilterGroup::Sphere);
			goalTrigger2->Color(PxVec3(255 / 255, 255 / 255, 255 / 255));
			goalTrigger2->Name("Goal2");
			goalTrigger2->GetShape()->getActor()->setName("Goal2");
			Add(goalTrigger2);


			/*-------------------------------------------------Motor Arms-------------------------------------------------
			--------------------------Adds 2 motor arms into the scene to be used as a motor/spinner----------------------
			----------------------------------------Sets object to a unique colour----------------------------------------
			-------Sets the name of shape which can be used to check trigger events and movement in Visual Debugger-------
			-----------------------Disabled gravity to stop unnecessary forces acting upon the arms-----------------------
			-----------------------Has a material which speeds up any actors that come into contact-----------------------
			----------------------------------------------Add shape to scene---------------------------------------------*/
			motorArms = new MotorArms();
			motorArms->Color(color_palette[3]);
			motorArms->GetShape()->getActor()->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
			motorArms->Material(CreateMaterial(0.f, 0.f, 1.75f));
			motorArms->Name("Motor");
			Add(motorArms);

			
			/*------------------------Added instance of springs using the trampoline class provided-----------------------
			----------------------All translations and rotations are handled within the class itself----------------------
			----------------A D6 joint was added to the bottom box to secure it to an anchor and lock axes----------------
			------------------------Used custom method to add to scene rather than standard Add()-------------------------*/
			Trampoline* tramp = new Trampoline();
			tramp->AddToScene(this);
			

			/*------------------------------------------Obstacle Trigger 1 and 2------------------------------------------
			----Reuses and repurposes static box actor box to change the direction of the moving obstacle in the game-----
			------------Trigger shapes utilise onTrigger method, rather than filtergroups and onContact method------------
			----------------------------Actor and shape names are defined for triggers checks-----------------------------
			-------------Triggers set to same material as other surrounding to avoid interruptions in speeds--------------
			------Also set as same colour as boundaries to blend in and show the shape is not important to the game-------
			----------------------------------------------Add trigger to scene--------------------------------------------*/
			obstacleTrigger1 = new Box(PxTransform(PxVec3(-15.f, 4.f, 23.65f)), PxVec3(3.f, 8.f, .25f));
			obstacleTrigger1->SetTrigger(true);
			obstacleTrigger1->Color(PxVec3(0 / 255, 255 / 255, 0 / 255));
			obstacleTrigger1->Material(boundariesMaterial);
			obstacleTrigger1->Name("Obstacle Trigger Bottom");
			obstacleTrigger1->GetShape()->getActor()->setName("Obstacle Trigger Bottom");
			Add(obstacleTrigger1);

			obstacleTrigger2 = new Box(PxTransform(PxVec3(-15.f, 4.f, -23.65f)), PxVec3(3.f, 8.f, .25f));
			obstacleTrigger2->SetTrigger(true);
			obstacleTrigger2->Color(PxVec3(0 / 255, 255 / 255, 0 / 255));
			obstacleTrigger2->Material(boundariesMaterial);
			obstacleTrigger2->Name("Obstacle Trigger Top");
			obstacleTrigger2->GetShape()->getActor()->setName("Obstacle Trigger Top");
			Add(obstacleTrigger2);

			obstacle = new Obstacle(PxTransform(PxVec3(-15.5f, 4.f, 3.5f)), PxVec3(0.5, 3.5, 7.5f), 1.f);
			obstacle->Name("Obstacle");
			obstacle->GetShape()->getActor()->setName("Obstacle");
			Add(obstacle);

			
			/*-------------------------------------------------Joints-----------------------------------------------------
			-------------------Revolute joint to allow movement around the Y axis for flipper movement--------------------
			---------D6 Joints for all controllable actors to prevent any chance of the shape flying off on Y axis--------
			-D6 Joints are all constrained on the Y axis and allow for free movement on both X and Z, decided by booleans-
			-------------------------------------------------------------------------------------------------------------*/
			RevoluteJoint* joint = new RevoluteJoint(box, PxTransform(PxVec3(0.f, 0.f, 0.0f), PxQuat(PxPi / 2, PxVec3(0.f, 0.f, 1.f))), flipper, PxTransform(PxVec3(0.f, 0.0f, -10.0f), PxQuat(PxPi / 2, PxVec3(1.0f, 0.f, 0.f)))); //cant add drive velocity due to static actor
			D6Joint* d6jointPlayer1 = new D6Joint(NULL, PxTransform(PxVec3(0.f, 3.f, 0.f)), player1, PxTransform(PxVec3(0.f, 0.5f, 0.f)), x = true, y = false, z = true);
			D6Joint* d6jointPlayer2 = new D6Joint(NULL, PxTransform(PxVec3(0.f, 3.f, 0.f)), player2, PxTransform(PxVec3(0.f, 0.5f, 0.f)), x = true, y = false, z = true);
			D6Joint* d6jointBall = new D6Joint(NULL, PxTransform(PxVec3(-30.f, .75f, 0.f)), sphere, PxTransform(PxVec3(0.f, 0.5f, 0.f)), x = true, y = false, z = true);
			D6Joint* d6jointObstacle = new D6Joint(NULL, PxTransform(PxVec3(-15.5f, 4.f, 3.5f)), obstacle, PxTransform(PxVec3(0.f, 0.f, 0.f)), x = false, y = false, z = true);
			
			//Motor joint with drive velocity applied to make a spiral of arms spin slightly offset from the centre to take up a larger area
			RevoluteJoint* motorJoint = new RevoluteJoint(NULL, PxTransform(PxVec3(-40.f, 0.55f, 12.0f), PxQuat(PxPi / 2, PxVec3(0.f, 0.f, 1.f))), motorArms, PxTransform(PxVec3(4.75f, 0.f, -0.0f), PxQuat(PxPi / 2, PxVec3(0.0f, 0.f, 1.f))));
			motorJoint->DriveVelocity(PxReal(-1));
		}

		//Add all players (interactable actors) to a vector - for use with movement in VisualDebugger
		PxActor* GetPlayerActors()
		{
			vector<PxActor*> playerList = GetAllActors();
		}

		//Custom udpate function (this function is called every frame and can be used to call other frequently-required methods
		virtual void CustomUpdate()
		{

			direction = my_callback->direction;

			//Frequently update score by checking the values in simulation callback
			scorePlayer1 = my_callback->scorePlayer1;
			scorePlayer2 = my_callback->scorePlayer2;
			gameOver = my_callback->gameOver;

			//check if game is over from values in simulation callback
			if (my_callback->isOver == true)
			{
				resetScene();
				my_callback->isOver = false;
			}
		}
		
		//reset actor; disable simulation, set location relative to world and re-enabling - removing all forces and acceleration
		void resetScene()
		{
			/*--could use Reset(), but that reinitialises score as 0, even though a player has just scored, so have to forcefully reset positions--*/
			
			//reset sphere position position
			sphere->Get()->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, true);
			sphere->GetShape()->getActor()->setGlobalPose(PxTransform(PxVec3(-20.f, 1.f, 0.f)));
			sphere->Get()->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, false);

			//reset player1 box position
			player1->Get()->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, true);
			player1->GetShape()->getActor()->setGlobalPose(PxTransform(PxVec3(30.f, 2.5f, 0.15f)));
			player1->Get()->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, false);

			//reset player2 box position
			player2->Get()->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, true);
			player2->GetShape()->getActor()->setGlobalPose(PxTransform(PxVec3(-60.5f, 2.5f, .0f)));
			player2->Get()->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, false);
		}

		//set score to 0 for a new game and change bool which changes game state
		void newGame()
		{
			my_callback->scorePlayer1 = 0;
			my_callback->scorePlayer2 = 0;
			my_callback->gameOver = false;
			resetScene();
		}
	};
}
