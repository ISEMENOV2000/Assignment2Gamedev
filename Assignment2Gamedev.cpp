// Assignment2Gamedev.cpp: A program using the TL-Engine

#include <TL-Engine.h>	// TL-Engine include file and namespace
using namespace tle;
#include <sstream>
using namespace std;

// declaration of constant values

// Value of Pi
const float kPi = 3.1415926f;
// Speed of Hovercraft
const float kSpeed = 50.0f;
// Enemy speed Multiplyer
const float kEnemySpeedMult = 2.0f;
// Reverce Speed
const float kReverceSpeed = -10.0f;

const float kTurnSpeed = 100.0f;

const float kMouseTurnSpeed = 45.0f;
// Radius of Hovercar
const float kRadius = 4.0f;
// Radius of Barrel
const float kBarrelRadius = 2.0f;
// Nuber of Barrels
const int kNumberOfTanks = 35;
// Number of walls
const int kNumberOfWalls = 6;
// Radius of Checkpoint legs
const float kLegRadius = 0.5f;
// Distanse of Checkpoint legs from center
const float kLegOffsetX = 9.0f;
// Number of checkpoints
const int numCheckpoints = 5;

const float kBounceFactor = 0.5;
const float wobbleAmplitude = 0.7f;
const float wobbleFrequency = 2.5f;
const float boostSpeedMultiplier = 2.0f;
const float boostDuration = 4.0f;
const float boostCooldownDuration = 6.0f;
const float boostRecoveryRate = 1.0f;
const int kNumberOfDummy = 24;
const float kCameraOffsetX = 0.0f;
const float kCameraOffsetY = 10.0f;
const float kCameraOffsetZ = -100.0f;
const float waypointRadius = 5.0;



// Hitbox Structure Declaration
struct Hitbox
{
	float xPos;
	float yPos;
	float zPos;
	float width;
	float length;
};

// Wall Structure Declaration
struct Wall
{
	IModel* wallmodel;
	IModel* islemodel1;
	IModel* islemodel2;
	float length;
	float xPos;
	float yPos;
	float zPos;
	Hitbox hitbox;
};

// Tank Structure Declaration
struct Tank
{
	IModel* tankmodel;
	float xPos;
	float yPos;
	float zPos;
};

// Dummy Structure Declaration
struct DummyPoint
{
	IModel* Dummy;
	float xPos;
	float yPos;
	float zPos;
};

// Vector Structure Declaration
struct Vector3 {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

// Facing vector calculation function
void calculateFacingVector(IModel* model, Vector3& facingVector)
{
	float initialPosX = model->GetX();
	float initialPosY = model->GetY();
	float initialPosZ = model->GetZ();
	model->MoveLocalZ(1.0f);
	float finalPosX = model->GetX();
	float finalPosY = model->GetY();
	float finalPosZ = model->GetZ();
	facingVector.x = finalPosX - initialPosX;
	facingVector.y = finalPosY - initialPosY;
	facingVector.z = finalPosZ - initialPosZ;
	model->MoveLocalZ(-1.0f);

}


// Class definition for HoverCar
class HoverCar {
	IModel* model;
	float currentSpeed = 0.0f;
	const float maxSpeed = 150.0f;
	const float minSpeed = -20.0f;
	float friction = 0.80f; // Friction value between 0 and 1, where 1 is no friction
	float wobbleTime = 0.0f;


	float boostTime = 0.0f;
	float boostCooldown = 0.0f;
	bool boostOverheated = false;

	Vector3 facingVector;

public:
	// Constructor to create a new HoverCar with the given mesh and initial position
	HoverCar(IMesh* mesh, float x, float y, float z) {
		model = mesh->CreateModel(x, y, z); // Create a new model at the given position
		updateFacingVector(); // Update the facing vector based on the initial orientation of the model
	}

	// Move the car forward by the given amount of time, speed multiplier, and whether or not to bounce
	void moveForward(float frameTime, float speedMultiplier, bool bounce) {
		currentSpeed += kSpeed * speedMultiplier * frameTime; // Increase speed based on speed multiplier and time
		if (currentSpeed > maxSpeed) { // Cap speed at maximum speed
			currentSpeed = maxSpeed;
		}
		if (bounce == true) { // If bounce flag is true, reverse speed and apply bounce factor
			currentSpeed = -currentSpeed * kBounceFactor;
		}
	}

	// Move the car backward by the given amount of time
	void moveBackward(float frameTime) {
		currentSpeed -= kSpeed * frameTime; // Decrease speed based on time
		if (currentSpeed < minSpeed) { // Cap speed at minimum speed
			currentSpeed = minSpeed;
		}
	}

	// Update the car's position and state based on the given amount of time and boost state
	void update(float frameTime, bool boostActive) {
		// Apply friction to slow down the car
		float frictionFactor = pow(friction, frameTime);
		currentSpeed *= frictionFactor;

		// Move the car based on its current speed and facing vector
		model->Move(facingVector.x * currentSpeed * frameTime, 0.0f, facingVector.z * currentSpeed * frameTime);

		// Apply a wobbling effect to the car's position
		wobbleTime += frameTime;
		float yOffset = wobbleAmplitude * sin(wobbleFrequency * wobbleTime);
		model->SetY(model->GetY() + yOffset * frameTime);

		// Handle the boost cooldown and overheating
		if (boostOverheated) {
			boostCooldown += frameTime;
			friction = 0.1f; // Decrease friction to make the car slide more
			if (boostCooldown >= boostCooldownDuration) { // If cooldown is finished, reset the overheating flag and friction value
				boostOverheated = false;
				boostCooldown = 0.0f;
				friction = 0.80f;
			}
		}
		else if (!boostActive) { // If boost is not active, start recovering the boost over
			boostTime -= frameTime * boostRecoveryRate;
			if (boostTime < 0.0f) {
				boostTime = 0.0f;
			}
		}
	}

	// Function Handles right turn
	void turnRight(float frameTime) {
		model->RotateLocalY(kTurnSpeed * frameTime);
		updateFacingVector();
	}

	// Function Handles left turn
	void turnLeft(float frameTime) {
		model->RotateLocalY(-kTurnSpeed * frameTime);
		updateFacingVector();
	}

	//  getss x position of craft
	float getX() {
		return model->GetX();
	}

	// gets Z position of craft
	float getZ() {
		return model->GetZ();
	}

	// gets model of craft
	IModel* getModel() {
		return model;
	}

	// Returns the current speed of the car
	float getCurrentSpeed() const {
		return currentSpeed;
	}

	// Returns true if the boost is overheated, false otherwise
	bool isBoostOverheated() {
		return boostOverheated;
	}

	// Returns the amount of time remaining for the boost to recharge
	float getBoostTime() {
		return boostTime;
	}

	// Returns the amount of time needed for the boost to recharge
	float getBoostCooldown() {
		return boostCooldown;
	}

	// Sets the amount of time remaining for the boost to recharge
	void setBoostTime(float time) {
		boostTime = time;
	}

	// Returns true if the boost is overheated, false otherwise (const version)
	bool isBoostOverheated() const {
		return boostOverheated;
	}

	// Sets whether the boost is overheated or not
	void setBoostOverheated(bool overheated) {
		boostOverheated = overheated;
	}

	// Sets the x-coordinate of the car's position
	void SetX(float x) {
		model->SetX(x);
	}

	// Sets the z-coordinate of the car's position
	void SetZ(float z) {
		model->SetZ(z);
	}

private:
	// Updates the facing vector of the car
	void updateFacingVector() {
		calculateFacingVector(model, facingVector);
	}
};

// Class definition for Opponent HoverCar
class OpponentCar : public HoverCar {
public:
	// Constructor takes an IMesh pointer, and x, y, and z coordinates to create the car
	OpponentCar(IMesh* mesh, float x, float y, float z) : HoverCar(mesh, x, y, z) {}

	// Makes the opponent car face towards a waypoint specified by a DummyPoint
	void faceWaypoint(const DummyPoint& dummy) {
		getModel()->LookAt(dummy.Dummy);
	}

	// Moves the opponent car forward based on its facing direction, with a speed multiplier applied
	void moveForwardBasedOnFacing(float frameTime, float speedMultiplier) {
		getModel()->MoveLocalZ(kSpeed * speedMultiplier * frameTime * kEnemySpeedMult);
	}

	// Sets the skin of the opponent car
	void setSkin(const std::string& skinFilename) {
		getModel()->SetSkin(skinFilename.c_str());
	}
};

// Creates a wall with the specified parameters and returns it
Wall CreateWall(IMesh* wallMesh, IMesh* isleMesh, float xPos, float yPos, float zPos, float rotation, int length);

// Creates a tank with the specified parameters and returns it
Tank CreateTank(IMesh* tankMesh, float xPos, float yPos, float zPos);

// Determines if two spheres are colliding based on their positions and radii
bool sphere2Sphere(float s1XPos, float s1ZPos, float s1Radius, float s2XPos, float s2ZPos, float s2Radius);

// Determines if a sphere and a box are colliding based on their positions and dimensions
bool sphere2Box(float sphereXPos, float sphereZPos, float sphereRadius, float boxXPos, float boxYPos, float boxZPos, float boxWidth, float boxLength);

// Determines if a hovercar is colliding with a checkpoint
bool isCollidingWithCheckpoint(IModel* hovercar, IModel* checkpoint);

// Determines if any of a hovercar's legs are colliding with any of the checkpoints
bool isCollidingWithCheckpointLegs(IModel* hovercar, IModel* checkpoints[], int numCheckpoints, float checkpointRotations[]);

// Calculates the facing vector of a car based on its Y rotation
Vector3 CalculateFacingVector(float rotationYDegrees);

// Creates a dummy point with the specified parameters and returns it
DummyPoint CreateDummy(IMesh* dummyMesh, float xPos, float yPos, float zPos);

// Handles collision between two hovercars by bouncing them off each other
void handleHovercarCollision(HoverCar& hovercar1, HoverCar& hovercar2, float frameTime, float kBounceFactor);

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine(kTLX);
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder("C:\\ProgramData\\TL-Engine\\Media");
	myEngine->AddMediaFolder("C:\\Users\\Vanja\\OneDrive - UCLan\\Desktop\\Assignment2Gamedev\\media");

	/**** Set up your scene here ****/
	ICamera* myCamera = myEngine->CreateCamera(kManual);

	IFont* myFont = myEngine->LoadFont("Comic Sans MS", 36);
	IFont* smallFont = myEngine->LoadFont("Comic Sans MS", 20);

	ISprite* backdrop = myEngine->CreateSprite("50-Beautiful-and-Minimalist-Presentation-Backgrounds-046.jpg", 0, myEngine->GetHeight() - 90);



	IMesh* skyMesh = myEngine->LoadMesh("Skybox 07.x");
	IModel* sky = skyMesh->CreateModel(0, -960, 0);

	IMesh* floorMesh = myEngine->LoadMesh("Ground.x");
	IModel* floor = floorMesh->CreateModel(0, 0, 0);

	IMesh* tribune1Mesh = myEngine->LoadMesh("Tribune1.x");
	IModel* tribune1 = tribune1Mesh->CreateModel(0, 0, 0);

	IMesh* tribune2Mesh = myEngine->LoadMesh("Tribune2.x");
	IModel* tribune2 = tribune2Mesh->CreateModel(0, 0, 30);


	IMesh* tribune3Mesh = myEngine->LoadMesh("Tribune3.x");
	IModel* tribune3 = tribune3Mesh->CreateModel(0, 0, 100);

	IMesh* walkwayMesh = myEngine->LoadMesh("Walkway.x");
	IModel* walkway = walkwayMesh->CreateModel(0, 0, 150);

	IMesh* garagelMesh = myEngine->LoadMesh("GarageLarge.x");
	IModel* garagel = garagelMesh->CreateModel(0, 0, 250);

	IMesh* garagesMesh = myEngine->LoadMesh("GarageSmall.x");
	IModel* garages = garagesMesh->CreateModel(0, 0, 290);

	IMesh* checkpointMesh = myEngine->LoadMesh("Checkpoint.x");
	IModel* checkpoint1 = checkpointMesh->CreateModel(-200, 0, 0);
	IModel* checkpoint2 = checkpointMesh->CreateModel(-85, 0, -100);
	IModel* checkpoint3 = checkpointMesh->CreateModel(110, 0, 0);
	IModel* checkpoint4 = checkpointMesh->CreateModel(370, 0, -100);
	IModel* checkpoint5 = checkpointMesh->CreateModel(-200, 0, -200);


	IMesh* wallMesh = myEngine->LoadMesh("Wall.x");


	IMesh* isleMesh = myEngine->LoadMesh("IsleStraight.x");


	IMesh* tanksmallMesh = myEngine->LoadMesh("TankSmall1.x");
	IMesh* tankMesh = myEngine->LoadMesh("TankSmall2.x");


	IModel* fallentank1 = tankMesh->CreateModel(0, -5, -160);
	fallentank1->RotateZ(45);
	IModel* fallentank2 = tankMesh->CreateModel(50, -5, -80);
	fallentank2->RotateZ(-45);

	IMesh* hovercarMesh = myEngine->LoadMesh("Race2.x");
	HoverCar hovercar(hovercarMesh, -200, 0, -50);

	OpponentCar opponentCar(hovercarMesh, -200, 0, -20);
	opponentCar.setSkin("td_interstellar.jpg");

	IMesh* dummyM = myEngine->LoadMesh("Dummy.x");

	// creates dummy models usin the createdummy function and stores them in the dummypoint struct
	DummyPoint dummy1 = CreateDummy(dummyM, -200, 0, 0);
	DummyPoint dummy2 = CreateDummy(dummyM, -200, 0, 50);
	DummyPoint dummy3 = CreateDummy(dummyM, -180, 0, 130);
	DummyPoint dummy4 = CreateDummy(dummyM, -150, 0, 160);
	DummyPoint dummy5 = CreateDummy(dummyM, -100, 0, 160);
	DummyPoint dummy6 = CreateDummy(dummyM, -90, 0, 100);

	DummyPoint dummy7 = CreateDummy(dummyM, -90, 0, 50);
	DummyPoint dummy8 = CreateDummy(dummyM, -90, 0, 0);
	DummyPoint dummy9 = CreateDummy(dummyM, -90, 0, -100);
	DummyPoint dummy10 = CreateDummy(dummyM, -90, 0, -140);
	DummyPoint dummy11 = CreateDummy(dummyM, -70, 0, -170);
	DummyPoint dummy12 = CreateDummy(dummyM, -30, 0, -200);
	DummyPoint dummy13 = CreateDummy(dummyM, -10, 0, -200);

	DummyPoint dummy14 = CreateDummy(dummyM, 50, 0, -140);
	DummyPoint dummy15 = CreateDummy(dummyM, 90, 0, -60);
	DummyPoint dummy16 = CreateDummy(dummyM, 120, 0, 30);
	DummyPoint dummy17 = CreateDummy(dummyM, 250, 0, 100);
	DummyPoint dummy18 = CreateDummy(dummyM, 300, 0, 80);
	DummyPoint dummy19 = CreateDummy(dummyM, 370, 0, 50);
	DummyPoint dummy20 = CreateDummy(dummyM, 370, 0, -270);
	DummyPoint dummy21 = CreateDummy(dummyM, 250, 0, -310);
	DummyPoint dummy22 = CreateDummy(dummyM, 100, 0, -390);
	DummyPoint dummy23 = CreateDummy(dummyM, -100, 0, -390);
	DummyPoint dummy24 = CreateDummy(dummyM, -200, 0, -300);

	// stores all dummie strcts in a dummy sarray
	DummyPoint dummies[kNumberOfDummy] = { dummy1, dummy2, dummy3, dummy4, dummy5, dummy6,
											dummy7, dummy8, dummy9, dummy10, dummy11, dummy12, dummy13,
											dummy14, dummy15, dummy16, dummy17, dummy18, dummy19, dummy20,
											dummy21, dummy22, dummy23, dummy24 };

	int currentWaypointIndex = 0;



	// creates tank models using the createtank function and stores them in the tank struct
	Tank tank1 = CreateTank(tanksmallMesh, -215, 0, 80);
	Tank tank2 = CreateTank(tankMesh, -180, 0, 180);
	Tank tank3 = CreateTank(tanksmallMesh, -140, 0, 200);
	Tank tank4 = CreateTank(tankMesh, -100, 0, 180);
	Tank tank5 = CreateTank(tanksmallMesh, -180, 0, 80);
	Tank tank6 = CreateTank(tankMesh, -150, 0, 120);
	Tank tank7 = CreateTank(tanksmallMesh, -70, 0, 100);
	Tank tank8 = CreateTank(tankMesh, -110, 0, 80);
	Tank tank9 = CreateTank(tanksmallMesh, -105, 0, -150);
	Tank tank10 = CreateTank(tankMesh, -85, 0, -200);
	Tank tank11 = CreateTank(tanksmallMesh, 0, 0, -220);
	Tank tank12 = CreateTank(tankMesh, 60, 0, -150);
	Tank tank13 = CreateTank(tanksmallMesh, 100, 0, -50);
	Tank tank14 = CreateTank(tankMesh, 150, 0, 20);
	Tank tank15 = CreateTank(tanksmallMesh, 250, 0, 80);
	Tank tank16 = CreateTank(tankMesh, -40, 0, -160);
	Tank tank17 = CreateTank(tanksmallMesh, 0, 0, -100);
	Tank tank18 = CreateTank(tankMesh, 60, 0, -20);
	Tank tank19 = CreateTank(tanksmallMesh, 140, 0, 90);
	Tank tank20 = CreateTank(tankMesh, 240, 0, 140);
	Tank tank21 = CreateTank(tanksmallMesh, 320, 0, 130);
	Tank tank22 = CreateTank(tankMesh, 150, 0, 20);
	Tank tank23 = CreateTank(tanksmallMesh, 250, 0, 80);
	Tank tank24 = CreateTank(tanksmallMesh, 370, 0, -320);
	Tank tank25 = CreateTank(tankMesh, 250, 0, -350);
	Tank tank26 = CreateTank(tanksmallMesh, 150, 0, -390);
	Tank tank27 = CreateTank(tanksmallMesh, 50, 0, -400);
	Tank tank28 = CreateTank(tankMesh, -60, 0, -400);
	Tank tank29 = CreateTank(tanksmallMesh, -200, 0, -350);
	Tank tank30 = CreateTank(tanksmallMesh, 300, 0, -280);
	Tank tank31 = CreateTank(tankMesh, 210, 0, -280);
	Tank tank32 = CreateTank(tanksmallMesh, 120, 0, -330);
	Tank tank33 = CreateTank(tanksmallMesh, 20, 0, -350);
	Tank tank34 = CreateTank(tankMesh, -60, 0, -330);
	Tank tank35 = CreateTank(tanksmallMesh, -150, 0, -310);

	// stores all tank structs in a tank array
	Tank tanks[kNumberOfTanks] = { tank1, tank2, tank3, tank4, tank5, tank6, tank7, tank8,
	tank9, tank10, tank11, tank12, tank13, tank14, tank15, tank16,
	tank17, tank18, tank19, tank20, tank21, tank22, tank23, tank24, tank25, tank26, tank27, tank28, tank29,
	tank30, tank31, tank32, tank33, tank34, tank35 };

	// creates wall using create wall function
	Wall wall1 = CreateWall(wallMesh, isleMesh, -215.0f, 0.0f, 46.0f, 0, 40);
	Wall wall2 = CreateWall(wallMesh, isleMesh, -185.0f, 0.0f, 46.0f, 0, 40);
	Wall wall3 = CreateWall(wallMesh, isleMesh, -95.0f, 0.0f, 46.0f, 0, 20);
	Wall wall4 = CreateWall(wallMesh, isleMesh, -75.0f, 0.0f, 46.0f, 0, 20);
	Wall wall5 = CreateWall(wallMesh, isleMesh, 350.0f, 0.0f, 10.0f, 0, 32);
	Wall wall6 = CreateWall(wallMesh, isleMesh, 390.0f, 0.0f, 46.0f, 0, 40);

	// stores walls
	Wall walls[kNumberOfWalls] = { wall1, wall2, wall3, wall4, wall5, wall6 };


	IModel* checkpoints[numCheckpoints] = { checkpoint1, checkpoint2, checkpoint3, checkpoint4, checkpoint5 };
	float checkpointRotations[numCheckpoints] = { 0.0f, 0.0f, 35.0f, 0.0f, 0.0f }; // Replace these values with the actual rotations of your checkpoints

	// rotates checkpoints
	checkpoint1->RotateY(checkpointRotations[0]);
	checkpoint2->RotateY(checkpointRotations[1]);
	checkpoint3->RotateY(checkpointRotations[2]);
	checkpoint4->RotateY(checkpointRotations[3]);
	checkpoint5->RotateY(checkpointRotations[4]);



	Vector3 CheckpointFacingVector; // The facing vector of the current checkpoint
	Vector3 hovercarFacingVector; // The facing vector of the player's hovercar
	Vector3 cameraFacingVector; // The facing vector of the camera

	enum EGameState { Demo, CountDown, Play, GamePaused, GameOver, GameWon }; // Enumeration of possible game states
	EGameState GameState = Demo; // The current game state

	enum ECameraMode { FreeMoving, ThirdPerson, FirstPerson }; // Enumeration of possible camera modes
	ECameraMode CameraMode = FreeMoving; // The current camera mode

	int currentStage = 0; // The current stage of the game (0 = Stage 0, 1 = Stage 1, 2 = Race Complete)

	float last_positionX; // The last recorded x-coordinate of the player's hovercar
	float last_positionZ; // The last recorded z-coordinate of the player's hovercar

	float time = 0; // The current time of the game

	bool connection = false; // A flag indicating whether the game is currently connected to a server

	float initialPositionX = 0.0; // The initial x-coordinate of the player's hovercar
	float initialPositionY = 0.0; // The initial y-coordinate of the player's hovercar
	float initialPositionZ = 0.0; // The initial z-coordinate of the player's hovercar

	// initializes the timer
	myEngine->Timer();
	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();

		// stores the time it took to complete to loop
		float frameTime = myEngine->Timer();

		int frame = 1 / frameTime;
		time = time + frameTime;




		/**** Update your scene each frame here ****/

		stringstream outText;

		// Displays the current FPS on the screen
		outText << "FPS: " << frame;
		myFont->Draw(outText.str(), myEngine->GetWidth() - 180, myEngine->GetHeight() - 700, kWhite);
		outText.str(""); // Clear myStream

		// Displays the current speed of the player's hovercar
		int speedDisplay = static_cast<int>(hovercar.getCurrentSpeed());
		outText << "Speed: " << speedDisplay;
		smallFont->Draw(outText.str(), 10, myEngine->GetHeight() - 90, kWhite);
		outText.str("");

		// Calculates the distance between the opponent car and the current waypoint
		float distanceToWaypoint = static_cast<float>(sqrt(pow(dummies[currentWaypointIndex].xPos - opponentCar.getX(), 2) +
			pow(dummies[currentWaypointIndex].zPos - opponentCar.getZ(), 2)));

		// If the opponent car has reached the current waypoint, move to the next waypoint
		if (distanceToWaypoint < waypointRadius) {
			currentWaypointIndex = (currentWaypointIndex + 1) % kNumberOfDummy;
		}

		// Make the opponent car face towards the current waypoint
		opponentCar.faceWaypoint(dummies[currentWaypointIndex]);

		// If the game is in the "Play" state, move the opponent car forward
		if (GameState == Play) {
			opponentCar.moveForwardBasedOnFacing(frameTime, 1.0f);
		}

		// Display various messages related to the player's boost state
		if (hovercar.getBoostTime() > 0 && hovercar.getBoostTime() < boostDuration - 2.0f) {
			outText << "BOOST";
		}
		if (hovercar.isBoostOverheated()) {
			outText << "OVERHEATED!";
		}
		else if (hovercar.getBoostTime() > boostDuration - 2.0f) {
			outText << "WARNING";
		}
		smallFont->Draw(outText.str(), myEngine->GetWidth() / 2 - 100, myEngine->GetHeight() - 90, kRed);
		outText.str("");

		if (myEngine->KeyHit(Key_Space) && GameState == Demo)
		{
			GameState = CountDown;
			time = 0;
		}

		// Displays a countdown before the race begins
		if (GameState == CountDown) {
			if (time <= 1) {
				outText << "3";
				myFont->Draw(outText.str(), myEngine->GetWidth() / 2, myEngine->GetHeight() / 2, kRed);
				outText.str(""); // Clear myStream
			}
			else if (time <= 2) {
				outText << "2";
				myFont->Draw(outText.str(), myEngine->GetWidth() / 2, myEngine->GetHeight() / 2, kRed);
				outText.str(""); // Clear myStream
			}
			else if (time <= 3) {
				outText << "1";
				myFont->Draw(outText.str(), myEngine->GetWidth() / 2, myEngine->GetHeight() / 2, kRed);
				outText.str(""); // Clear myStream
			}
			else if (time <= 4) {
				outText << "Go, go, go!";
				myFont->Draw(outText.str(), myEngine->GetWidth() / 2 - 50, myEngine->GetHeight() / 2, kRed);
				outText.str(""); // Clear myStream
			}
			else {
				GameState = Play;
			}
		}

		// Displays a message prompting the player to start the game
		if (GameState == Demo) {
			outText << "Press Space to Start";
			myFont->Draw(outText.str(), myEngine->GetWidth() / 2 - 140, myEngine->GetHeight() / 2, kWhite);
			outText.str(""); // Clear myStream
		}

		// Displays a message indicating that the player has won the game
		if (GameState == GameWon) {
			outText << "You Win!!";
			myFont->Draw(outText.str(), myEngine->GetWidth() / 2 - 50, myEngine->GetHeight() / 2, kWhite);
			outText.str(""); // Clear myStream
		}


		// Checkpoint logic
		if (GameState == Play) {
			// Check if player has reached each checkpoint in turn
			if (currentStage == 0 && isCollidingWithCheckpoint(hovercar.getModel(), checkpoint1)) {
				currentStage = 1;
			}
			else if (currentStage == 1 && isCollidingWithCheckpoint(hovercar.getModel(), checkpoint2)) {
				currentStage = 2;
			}
			else if (currentStage == 2 && isCollidingWithCheckpoint(hovercar.getModel(), checkpoint3)) {
				currentStage = 3;
			}
			else if (currentStage == 3 && isCollidingWithCheckpoint(hovercar.getModel(), checkpoint4)) {
				currentStage = 4;
			}
			else if (currentStage == 4 && isCollidingWithCheckpoint(hovercar.getModel(), checkpoint5)) {
				currentStage = 5;
			}
			// If player reaches last checkpoint, change game state to GameWon
			else if (currentStage == 5 && isCollidingWithCheckpoint(hovercar.getModel(), checkpoint1)) {
				GameState = GameWon;
			}
			// If player misses a checkpoint, display corresponding message
			else {
				if (currentStage < 1 && isCollidingWithCheckpoint(hovercar.getModel(), checkpoint2)) {
					outText << "Checkpoint " << currentStage + 1 << " missed!";
				}
				if (currentStage < 2 && isCollidingWithCheckpoint(hovercar.getModel(), checkpoint3)) {
					outText << "Checkpoint " << currentStage + 1 << " missed!";
				}
				if (currentStage < 3 && isCollidingWithCheckpoint(hovercar.getModel(), checkpoint4)) {
					outText << "Checkpoint " << currentStage + 1 << " missed!";
				}
				if (currentStage < 4 && isCollidingWithCheckpoint(hovercar.getModel(), checkpoint5)) {
					outText << "Checkpoint " << currentStage + 1 << " missed!";
				}

				// Display checkpoint missed message on screen
				if (!outText.str().empty()) {
					myFont->Draw(outText.str(), myEngine->GetWidth() / 2 - 100, myEngine->GetHeight() / 2, kRed);
					outText.str(""); // Clear myStream
				}
			}
		}

		// Display the current checkpoint text in all states
		stringstream checkpointText;
		if (GameState == GameWon)
		{
			checkpointText.str(""); // Clear checkpointText
			checkpointText << "Race Complete";
		}
		else {
			checkpointText << "Stage ";
			if (currentStage == 0)
			{
				checkpointText << "0";
			}
			else if (currentStage == 1)
			{
				checkpointText << "1";
			}
			else if (currentStage == 2)
			{
				checkpointText << "2";
			}
			else if (currentStage == 3)
			{
				checkpointText << "3";
			}
			else if (currentStage == 4)
			{
				checkpointText << "4";
			}
			else if (currentStage == 5)
			{
				checkpointText << "5";
			}
			checkpointText << " Complete";
		}
		smallFont->Draw(checkpointText.str(), myEngine->GetWidth() / 2 - 100, myEngine->GetHeight() - 35, kWhite);
		checkpointText.str(""); // Clear checkpointText

		// Update the scene each frame
		last_positionX = hovercar.getX();
		last_positionZ = hovercar.getZ();

		// Move the hovercar based on user input
		if (myEngine->KeyHeld(Key_W) && GameState == Play) {
			float speedMultiplier = 1.0f;


			// Apply boost if Space key is held down and boost is not overheated
			if (!hovercar.isBoostOverheated() && myEngine->KeyHeld(Key_Space)) {
				speedMultiplier = boostSpeedMultiplier;
				hovercar.setBoostTime(hovercar.getBoostTime() + frameTime);

				// Set boost to overheated if boost duration has elapsed
				if (hovercar.getBoostTime() >= boostDuration) {
					hovercar.setBoostOverheated(true);
					hovercar.setBoostTime(0.0f);
				}
			}

			hovercar.moveForward(frameTime, speedMultiplier, false);
		}

		if (myEngine->KeyHeld(Key_S) && GameState == Play)
		{
			hovercar.moveBackward(frameTime);
		}

		if (myEngine->KeyHeld(Key_D) && GameState == Play)
		{
			hovercar.turnRight(frameTime);
		}

		if (myEngine->KeyHeld(Key_A) && GameState == Play)
		{
			hovercar.turnLeft(frameTime);
		}

		bool boostActive = !hovercar.isBoostOverheated() && myEngine->KeyHeld(Key_Space);
		hovercar.update(frameTime, boostActive);

		float speedMultiplier = boostActive ? boostSpeedMultiplier : 1.0f;
		if (myEngine->KeyHeld(Key_W) && GameState == Play) {
			hovercar.moveForward(frameTime, speedMultiplier, false);
		}

		if (GameState == Play) {
			float kBounceFactor = 1.0f; // Adjust this value to control the strength of the bounce effect
			handleHovercarCollision(hovercar, opponentCar, frameTime, kBounceFactor);
		}


		if (myEngine->KeyHeld(Key_Up))
		{
			myCamera->MoveZ(kSpeed * frameTime);
		}

		if (myEngine->KeyHeld(Key_Down))
		{
			myCamera->MoveZ(-kSpeed * frameTime);
		}

		if (myEngine->KeyHeld(Key_Left))
		{
			myCamera->MoveX(-kSpeed * frameTime);
		}

		if (myEngine->KeyHeld(Key_Right))
		{
			myCamera->MoveX(kSpeed * frameTime);
		}

		if (myEngine->KeyHit(Key_2))
		{
			CameraMode = FreeMoving;

			myCamera->DetachFromParent(); // DetachFromParent() detaches object from any parent it is attached to
			myCamera->ResetOrientation();

			myCamera->SetPosition(0, 100, -100);

			initialPositionX = 0.0;
			initialPositionY = 100.0;
			initialPositionZ = -100.0;
		}



		// Update the camera position and rotation based on the hover-car's position and rotation
		if (myEngine->KeyHit(Key_3))
		{
			CameraMode = ThirdPerson;
			myCamera->DetachFromParent(); // DetachFromParent() detaches object from any parent it is attached to
			myCamera->ResetOrientation();
			myCamera->SetX(0);
			myCamera->SetY(15);
			myCamera->SetZ(-40);

			myCamera->AttachToParent(hovercar.getModel());
			connection = true;

			initialPositionX = 0.0;
			initialPositionY = 15.0;
			initialPositionZ = -40.0;

		}

		// Update the camera position and rotation based on the hover-car's position and rotation
		if (myEngine->KeyHit(Key_1))
		{
			CameraMode = FirstPerson;
			myCamera->DetachFromParent(); // DetachFromParent() detaches object from any parent it is attached to
			myCamera->ResetOrientation();
			myCamera->SetX(0);
			myCamera->SetY(5);
			myCamera->SetZ(1);
			myCamera->AttachToParent(hovercar.getModel());
			connection = true;

			initialPositionX = 0.0;
			initialPositionY = 5.0;
			initialPositionZ = 1.0;
		}



		// Update camera rotation based on mouse movement
		float mouseMoveX = myEngine->GetMouseMovementX();
		float mouseMoveY = myEngine->GetMouseMovementY();
		myCamera->RotateLocalY(mouseMoveX * kMouseTurnSpeed * frameTime);
		myCamera->RotateLocalX(mouseMoveY * kMouseTurnSpeed * frameTime);

		// Reset camera position and orientation with 'C'
		if (myEngine->KeyHit(Key_C))
		{
			if (connection == true) {
				myCamera->DetachFromParent();
				myCamera->SetPosition(initialPositionX, initialPositionY, initialPositionZ);
				myCamera->ResetOrientation();
				myCamera->AttachToParent(hovercar.getModel());
			}
			else {
				myCamera->SetPosition(initialPositionX, initialPositionY, initialPositionZ);
				myCamera->ResetOrientation();
			}
		}


		if (sphere2Sphere(hovercar.getX(), hovercar.getZ(), kRadius, fallentank1->GetX() - 5, fallentank1->GetZ(), kBarrelRadius)
			|| sphere2Sphere(hovercar.getX(), hovercar.getZ(), kRadius, fallentank2->GetX() + 8, fallentank2->GetZ(), kBarrelRadius))
		{
			hovercar.SetX(last_positionX);
			hovercar.SetZ(last_positionZ);
			hovercar.moveForward(frameTime, speedMultiplier, true);
		}





		for (int i = 0; i < kNumberOfWalls; i++) {
			if (sphere2Box(hovercar.getX(), hovercar.getZ(), kRadius, walls[i].hitbox.xPos, walls[i].hitbox.yPos, walls[i].hitbox.zPos, walls[i].hitbox.width, walls[i].hitbox.length))
			{
				hovercar.SetX(last_positionX);
				hovercar.SetZ(last_positionZ);
			}
		}

		for (int i = 0; i < kNumberOfTanks; i++) {
			if (sphere2Sphere(hovercar.getX(), hovercar.getZ(), kRadius, tanks[i].xPos, tanks[i].zPos, kBarrelRadius))
			{
				hovercar.SetX(last_positionX);
				hovercar.SetZ(last_positionZ);
				hovercar.moveForward(frameTime, speedMultiplier, true);
			}
		}



		if (isCollidingWithCheckpointLegs(hovercar.getModel(), checkpoints, numCheckpoints, checkpointRotations)) {
			hovercar.SetX(last_positionX);
			hovercar.SetZ(last_positionZ);
			hovercar.moveForward(frameTime, speedMultiplier, true);
		}








		// Test for quit
		if (myEngine->KeyHit(Key_Escape))
		{
			myEngine->Stop();
		}
	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}



// Function to create a wall and its hitbox
Wall CreateWall(IMesh* wallMesh, IMesh* isleMesh, float xPos, float yPos, float zPos, float rotation, int length)
{
	// Create a Wall struct instance
	Wall wall;

	// Create the main wall model at the specified position
	wall.wallmodel = wallMesh->CreateModel(xPos, yPos, zPos);

	// Create two isle models for the ends of the wall and attach them to the main wall model
	wall.islemodel1 = isleMesh->CreateModel(0, 0, 8);
	wall.islemodel1->AttachToParent(wall.wallmodel);
	wall.islemodel2 = isleMesh->CreateModel(0, 0, -8 - (length - 1) * 8);
	wall.islemodel2->AttachToParent(wall.wallmodel);

	// Store the position of the wall in the Wall struct
	wall.xPos = xPos;
	wall.yPos = yPos;
	wall.zPos = zPos;

	// Create additional wall models for the length of the wall
	for (int i = 1; i < length; ++i)
	{
		IModel* additionalWallModel = wallMesh->CreateModel(0, 0, -i * 8);
		additionalWallModel->AttachToParent(wall.wallmodel);
	}

	// Rotate the wall based on the provided rotation angle
	wall.wallmodel->RotateY(rotation);

	// Create a hitbox for the wall and store it in the Wall struct
	float wallWidth = 2.0f; // Set the width of the wall hitbox
	float wallLength = 14 + 8.0f * length; // Set the length of the wall hitbox based on the length of the wall
	wall.hitbox.xPos = xPos;
	wall.hitbox.yPos = yPos;
	wall.hitbox.zPos = zPos - 8.0f * (length - 1) / 2; // Adjust the zPos based on the length of the wall
	wall.hitbox.width = wallWidth;
	wall.hitbox.length = wallLength;

	// Return the created Wall struct
	return wall;
}
// CreateTank function: creates a Tank struct instance with a tankmodel attribute set to the provided tankMesh
// and xPos, yPos, zPos attributes set to the provided values
Tank CreateTank(IMesh* tankMesh, float xPos, float yPos, float zPos)
{
	Tank tank;
	tank.tankmodel = tankMesh->CreateModel(xPos, yPos, zPos);
	tank.xPos = xPos;
	tank.yPos = yPos;
	tank.zPos = zPos;

	return tank;
}

// sphere2Box function: checks for collision between a sphere with a given xPos, zPos, and radius and a box with given xPos, yPos, zPos,
// width, and length. Returns true if there is a collision, false otherwise.
bool sphere2Box(float sphereXPos, float sphereZPos, float sphereRadius, float boxXPos, float boxYPos, float boxZPos, float boxWidth, float boxLength)
{
	float xDistance = abs(sphereXPos - boxXPos);
	float zDistance = abs(sphereZPos - boxZPos);
		if (xDistance > (boxWidth / 2 + sphereRadius)) {
			return false;
		}
	if (zDistance > (boxLength / 2 + sphereRadius)) {
		return false;
	}

	if (xDistance <= (boxWidth / 2)) {
		return true;
	}
	if (zDistance <= (boxLength / 2)) {
		return true;
	}

	float cornerDistanceSq = pow(xDistance - boxWidth / 2, 2) + pow(zDistance - boxLength / 2, 2);
	return (cornerDistanceSq <= pow(sphereRadius, 2));
}

// sphere2Sphere function: checks for collision between two spheres with given xPos, zPos, and radii. Returns true if there is a collision, false otherwise.
bool sphere2Sphere(float s1XPos, float s1ZPos, float s1Radius, float s2XPos, float s2ZPos, float s2Radius) {
	float distX = s2XPos - s1XPos;
	float distZ = s2ZPos - s1ZPos;
	float distanceSquared = distX * distX + distZ * distZ;
	float radiiSum = s1Radius + s2Radius;
	return (distanceSquared <= (radiiSum * radiiSum));
}

Vector3 CalculateFacingVector(float rotationYDegrees) {
	float rotationYRadians = rotationYDegrees * kPi / 180.0f;
	Vector3 facingVector;
	facingVector.x = sin(rotationYRadians);
	facingVector.z = cos(rotationYRadians);
	return facingVector;
}

bool isCollidingWithCheckpoint(IModel* hovercar, IModel* checkpoint) {
	float hovercarRadius = kRadius;
	float checkpointRadius = 4.0f; // You can adjust this value as needed
	return sphere2Sphere(hovercar->GetX(), hovercar->GetZ(), hovercarRadius, checkpoint->GetX(), checkpoint->GetZ(), checkpointRadius);
}

bool isCollidingWithCheckpointLegs(IModel* hovercar, IModel* checkpoints[], int numCheckpoints, float checkpointRotations[]) {
	for (int i = 0; i < numCheckpoints; ++i) {
		IModel* checkpoint = checkpoints[i];
		float checkpointRotationY = checkpointRotations[i];

		Vector3 checkpointFacingVector = CalculateFacingVector(checkpointRotationY);

		// Check collision with the checkpoint's main body (sphere)
		float hovercarRadius = kRadius;

		// Calculate the rotated leg positions
		float legOffsetX_rotated = kLegOffsetX * checkpointFacingVector.z;
		float legOffsetZ_rotated = kLegOffsetX * checkpointFacingVector.x;

		// Check collision with the checkpoint's legs using sphere-to-sphere collision
		float legRadius = kLegRadius; // Assuming the leg is a cylinder with a spherical cap

		// Calculate the left leg position
		float leftLegX = checkpoint->GetX() + (checkpointRotationY != 0.0f ? legOffsetZ_rotated : -kLegOffsetX);
		float leftLegY = checkpoint->GetY() + legRadius; // Position of the center of the spherical cap
		float leftLegZ = checkpoint->GetZ() + (checkpointRotationY != 0.0f ? -legOffsetX_rotated : -legOffsetZ_rotated);
		bool leftLegCollision = sphere2Sphere(hovercar->GetX(), hovercar->GetZ(), hovercarRadius, leftLegX, leftLegZ, legRadius);

		// Calculate the right leg position
		float rightLegX = checkpoint->GetX() + (checkpointRotationY != 0.0f ? -legOffsetZ_rotated : kLegOffsetX);
		float rightLegY = checkpoint->GetY() + legRadius; // Position of the center of the spherical cap
		float rightLegZ = checkpoint->GetZ() + (checkpointRotationY != 0.0f ? legOffsetX_rotated : legOffsetZ_rotated);
		bool rightLegCollision = sphere2Sphere(hovercar->GetX(), hovercar->GetZ(), hovercarRadius, rightLegX, rightLegZ, legRadius);

		if (leftLegCollision || rightLegCollision) {
			return true;
		}
	}

	return false;
}

DummyPoint CreateDummy(IMesh* dummyMesh, float xPos, float yPos, float zPos)
{
	DummyPoint dummy;

	dummy.Dummy = dummyMesh->CreateModel(xPos, yPos, zPos);
	dummy.xPos = xPos;
	dummy.yPos = yPos;
	dummy.zPos = zPos;

	return dummy;
}

void handleHovercarCollision(HoverCar& hovercar1, HoverCar& hovercar2, float frameTime, float kBounceFactor) {

	if (sphere2Sphere(hovercar1.getX(), hovercar1.getZ(), kRadius, hovercar2.getX(), hovercar2.getZ(), kRadius)) {
		// Collision detected, calculate collision response

		// Calculate direction vector from hovercar2 to hovercar1
		Vector3 dir;
		dir.x = hovercar1.getX() - hovercar2.getX();
		dir.z = hovercar1.getZ() - hovercar2.getZ();

		// Normalize direction vector
		float magnitude = sqrt(dir.x * dir.x + dir.z * dir.z);
		dir.x /= magnitude;
		dir.z /= magnitude;

		// Move both cars away from each other based on the direction vector and the bounce factor
		hovercar1.SetX(hovercar1.getX() + dir.x * kBounceFactor);
		hovercar1.SetZ(hovercar1.getZ() + dir.z * kBounceFactor);

		hovercar2.SetX(hovercar2.getX() - dir.x * kBounceFactor);
		hovercar2.SetZ(hovercar2.getZ() - dir.z * kBounceFactor);

		// Reverse the hovercars' speeds and apply the bounce factor
		hovercar1.moveForward(frameTime, -1.0f, true);
		hovercar2.moveForward(frameTime, -1.0f, true);
	}
}

