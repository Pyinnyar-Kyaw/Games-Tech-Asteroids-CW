#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"
#include "ExtraLifePowerup.h"
#include <algorithm>

// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char* argv[])
	: GameSession(argc, argv), mEnableSpiralBullet(true), mEnableBrake(true), mEnableExtraLife(true)
{
	mLevel = 0;
	mAsteroidCount = 0;
	mInStartMenu = true;
}

/** Destructor. */
Asteroids::~Asteroids(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an asteroids game. */
void Asteroids::Start()
{
	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	Animation* explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation* asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation* spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");
	Animation* heart_anim = AnimationManager::GetInstance().CreateAnimationFromFile("heart", 32, 32, 32, 32, "heart.png");

	// Create some asteroids and add them to the world
	CreateAsteroids(10);

	LoadHighScores();

	//Create the GUI
	CreateGUI();

	// Add a player (watcher) to the game world
	mGameWorld->AddListener(&mPlayer);

	// Add this class as a listener of the player
	mPlayer.AddListener(thisPtr);

	// TEMPORARY SOLUTION: MAKE LIVES AND SCORE LABEL INVISIBLE ON GAME START
	mLivesLabel->SetVisible(false);
	mScoreLabel->SetVisible(false);

	// Start the game
	GameSession::Start();
}

/** Stop the current game. */
void Asteroids::Stop()
{
	// Stop the game
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	if (mAwaitingGamerTag)
	{
		if (key == 13 || key == '\r')
		{
			//save high score and exit input mode
			SaveHighScore(mGamerTagBuffer, mScoreKeeper.GetScore());
			mAwaitingGamerTag = false;
			mGamerTagInputLabel->SetVisible(false);
		}
		else if (key == 8 || key == 127)
		{
			if (!mGamerTagBuffer.empty())
				mGamerTagBuffer.pop_back();
		}
		else if (isprint(key) && mGamerTagBuffer.length() < 12)
		{
			mGamerTagBuffer += key;
		}

		//update input label text
		std::ostringstream label;
		label << "Enter name: " << mGamerTagBuffer;
		mGamerTagInputLabel->SetText(label.str());

		return;
	}
	
	switch (key)
	{
	case ' ':
		if (!mInStartMenu && mSpaceship && mSpaceship->GetWorld() != nullptr) {
			mSpaceship->Shoot();
		}
		break;

	case 'b':
		if (!mInStartMenu && mEnableSpiralBullet && mSpaceship && mSpaceship->GetWorld() != nullptr) {
			mSpaceship->ShootSpiral();
		}
		break;

	case 's':
		if (!mSpaceship || mSpaceship->GetWorld() == nullptr) {
			if (mPlayer.GetLives() > 0) {
				mGameWorld->AddObject(CreateSpaceship());
				CreateExtraLifePowerup();
				mStartMenuLabel->SetVisible(false);
				mLivesLabel->SetVisible(true);
				mScoreLabel->SetVisible(true);
				mInstructionsShootLabel->SetVisible(false);
				mInstructionsMovementLabel->SetVisible(false);
				mSpiralLabel->SetVisible(false);
				mBrakeLabel->SetVisible(false);
				mExtraLifeLabel->SetVisible(false);
				mInStartMenu = false;
				for (auto& label : mHighScoreLabels)
					label->SetVisible(false);
			}
		}
		break;

	case '1':
		if (mInStartMenu) {
			mEnableSpiralBullet = !mEnableSpiralBullet;
			UpdateToggleLabels();
		}
		break;

	case '2':
		if (mInStartMenu) {
			mEnableBrake = !mEnableBrake;
			UpdateToggleLabels();
		}
		break;

	case '3':
		if (mInStartMenu) {
			mEnableExtraLife = !mEnableExtraLife;
			UpdateToggleLabels();
		}
		break;

	default:
		break;
	}

}

void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	switch (key)
	{
		// If up arrow key is pressed start applying forward thrust
	case GLUT_KEY_UP:
		if (mSpaceship && mSpaceship->GetWorld() != nullptr)
		{
			mSpaceship->Thrust(10);
		}
		break;
		// If left arrow key is pressed start rotating anti-clockwise
	case GLUT_KEY_LEFT:
		if (mSpaceship && mSpaceship->GetWorld() != nullptr)
		{
			mSpaceship->Rotate(90);
		}
		break;
		// If right arrow key is pressed start rotating clockwise
	case GLUT_KEY_RIGHT:
		if (mSpaceship && mSpaceship->GetWorld() != nullptr)
		{
			mSpaceship->Rotate(-90);
		}
		break;
	case GLUT_KEY_DOWN:
		if (mEnableBrake && mSpaceship && mSpaceship->GetWorld() != nullptr)
		{
			mSpaceship->Brake();
		}
		break;
		// Default case - do nothing
	default: break;
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	switch (key)
	{
		// If up arrow key is released stop applying forward thrust
	case GLUT_KEY_UP:
		if (mSpaceship && mSpaceship->GetWorld() != nullptr)
		{
			mSpaceship->Thrust(0);
		}
		break;
		// If left arrow key is released stop rotating
	case GLUT_KEY_LEFT:
		if (mSpaceship && mSpaceship->GetWorld() != nullptr)
		{
			mSpaceship->Rotate(0);
		}
		break;
		// If right arrow key is released stop rotating
	case GLUT_KEY_RIGHT:
		if (mSpaceship && mSpaceship->GetWorld() != nullptr)
		{
			mSpaceship->Rotate(0);
		}
		break;
	case GLUT_KEY_DOWN:
		if (mSpaceship && mSpaceship->GetWorld() != nullptr)
		{
			//mSpaceship->Thrust(0);
		}
		break;
		// Default case - do nothing
	default: break;
	}
}


// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		mGameWorld->AddObject(explosion);
		mAsteroidCount--;
		if (mAsteroidCount <= 0)
		{
			SetTimer(500, START_NEXT_LEVEL);
		}
	}
	else if (object->GetType() == GameObjectType("ExtraLifePowerUp"))
	{
		int lives = mPlayer.GetLives();
		mPlayer.SetLives(lives + 2);
		if (mLivesLabel)
		{
			std::ostringstream msg;
			msg << "Lives: " << (lives + 2);
			mLivesLabel->SetText(msg.str());
		}
		return;
	}
}


// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	if (value == CREATE_NEW_PLAYER)
	{
		mSpaceship->Reset();
		mGameWorld->AddObject(mSpaceship);
	}

	if (value == START_NEXT_LEVEL)
	{
		mLevel++;
		int num_asteroids = 10 + 2 * mLevel;
		CreateAsteroids(num_asteroids);
	}

	if (value == SHOW_GAME_OVER)
	{
		mGameOverLabel->SetVisible(true);
	}
}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////
shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	mSpaceship->Reset();

	// Return the spaceship so it can be added to the world
	return mSpaceship;
}

void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount = num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
	}
}

void Asteroids::CreateExtraLifePowerup()
{
	if (!mEnableExtraLife) return;
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("heart");
	shared_ptr<Sprite> powerup_sprite
		= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	powerup_sprite->SetLoopAnimation(true);

	shared_ptr<GameObject> powerup = make_shared<ExtraLifePowerup>();
	powerup->SetBoundingShape(make_shared<BoundingSphere>(powerup->GetThisPtr(), 5.0f));
	powerup->SetSprite(powerup_sprite);
	powerup->SetScale(0.3f);
	mGameWorld->AddObject(powerup);
}

void Asteroids::CreateGUI()
{
	// Add a (transparent) border around the edge of the game display
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));
	// Create a new GUILabel and wrap it up in a shared_ptr
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	// Set the vertical alignment of the label to GUI_VALIGN_TOP
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> score_component
		= static_pointer_cast<GUIComponent>(mScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

	// Game Start Menu GUI Label
	mStartMenuLabel = make_shared<GUILabel>("Press S to start");
	mStartMenuLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mStartMenuLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	shared_ptr<GUIComponent> start_menu_component = static_pointer_cast<GUIComponent>(mStartMenuLabel);
	mGameDisplay->GetContainer()->AddComponent(start_menu_component, GLVector2f(0.5f, 0.5f));

	// Gameplay Instructions Shoot label
	mInstructionsShootLabel = make_shared<GUILabel>("(SPACE) to shoot");
	mInstructionsShootLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	shared_ptr<GUIComponent> instructions_shoot_component = static_pointer_cast<GUIComponent>(mInstructionsShootLabel);
	//mGameDisplay->GetContainer()->AddComponent(instructions_shoot_component, GLVector2f(0.0f, 0.37f));
	mGameDisplay->GetContainer()->AddComponent(instructions_shoot_component, GLVector2f(0.0f, 0.8f));

	// Gameplay Instructions Movement label
	mInstructionsMovementLabel = make_shared<GUILabel>("(ARROW KEYS) to move");
	mInstructionsMovementLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	shared_ptr<GUIComponent> instructions_movement_component = static_pointer_cast<GUIComponent>(mInstructionsMovementLabel);
	//mGameDisplay->GetContainer()->AddComponent(instructions_movement_component, GLVector2f(0.0f, 0.27f));
	mGameDisplay->GetContainer()->AddComponent(instructions_movement_component, GLVector2f(0.0f, 0.75f));

	//highscore table label
	mHighScoreLabels.clear();
	// High score header label
	auto header = make_shared<GUILabel>("High Scores:");
	header->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_LEFT);
	header->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	mGameDisplay->GetContainer()->AddComponent(header, GLVector2f(0.7f, 0.85f));
	mHighScoreLabels.push_back(header);
	float yOffset = 0.80f;
	int rank = 1;

	for (auto& hs : mHighScores)
	{
		std::ostringstream line;
		line << rank++ << ". " << hs.first << " - " << hs.second;

		auto scoreLabel = make_shared<GUILabel>(line.str());
		scoreLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_LEFT);
		scoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);

		mGameDisplay->GetContainer()->AddComponent(scoreLabel, GLVector2f(0.7f, yOffset));
		yOffset -= 0.03f;

		mHighScoreLabels.push_back(scoreLabel);
	}

	// Create a new GUILabel and wrap it up in a shared_ptr
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	// Set the vertical alignment of the label to GUI_VALIGN_BOTTOM
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
	mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mGameOverLabel = shared_ptr<GUILabel>(new GUILabel("GAME OVER"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility of the label to false (hidden)
	mGameOverLabel->SetVisible(false);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> game_over_component
		= static_pointer_cast<GUIComponent>(mGameOverLabel);
	mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.5f, 0.5f));

	// enter name label
	mGamerTagInputLabel = make_shared<GUILabel>("Enter name: ");
	mGamerTagInputLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGamerTagInputLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	mGamerTagInputLabel->SetVisible(false);
	shared_ptr<GUIComponent> tag_input_component = static_pointer_cast<GUIComponent>(mGamerTagInputLabel);
	mGameDisplay->GetContainer()->AddComponent(tag_input_component, GLVector2f(0.5f, 0.4f));

	//powerups toggle labels
	mSpiralLabel = make_shared<GUILabel>("1 - Spiral Bullets (b): ON");
	mBrakeLabel = make_shared<GUILabel>("2 - Brake (down arrow): ON");
	mExtraLifeLabel = make_shared<GUILabel>("3 - Extra Life: ON");

	shared_ptr<GUIComponent> spiralToggle = static_pointer_cast<GUIComponent>(mSpiralLabel);
	shared_ptr<GUIComponent> brakeToggle = static_pointer_cast<GUIComponent>(mBrakeLabel);
	shared_ptr<GUIComponent> extraLifeToggle = static_pointer_cast<GUIComponent>(mExtraLifeLabel);

	mGameDisplay->GetContainer()->AddComponent(spiralToggle, GLVector2f(0.0f, 0.36f));
	mGameDisplay->GetContainer()->AddComponent(brakeToggle, GLVector2f(0.0f, 0.32f));
	mGameDisplay->GetContainer()->AddComponent(extraLifeToggle, GLVector2f(0.0f, 0.28f));

}

void Asteroids::UpdateToggleLabels()
{
	mSpiralLabel->SetText("1 - Spiral Bullets (b): " + std::string(mEnableSpiralBullet ? "ON" : "OFF"));
	mBrakeLabel->SetText("2 - Brake (down arrow): " + std::string(mEnableBrake ? "ON" : "OFF"));
	mExtraLifeLabel->SetText("3 - Extra Life: " + std::string(mEnableExtraLife ? "ON" : "OFF"));
}


void Asteroids::OnScoreChanged(int score)
{
	//format using string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	//get score message as a string
	std::string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);
}

void Asteroids::OnPlayerKilled(int lives_left)
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	//format livse using string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives_left;
	//get the lives left message as a string
	std::string lives_msg = msg_stream.str();
	mLivesLabel->SetText(lives_msg);

	if (lives_left > 0)
	{
		SetTimer(1000, CREATE_NEW_PLAYER);
	}
	else if (lives_left <= 0)
	{
		SetTimer(500, SHOW_GAME_OVER);

		mAwaitingGamerTag = true;
		mGamerTagBuffer = "";
		mGamerTagInputLabel->SetText("Enter name: ");
		mGamerTagInputLabel->SetVisible(true);

		for (auto& label : mHighScoreLabels)
			label->SetVisible(true);

	}
}

void Asteroids::SaveHighScore(const std::string& tag, int score)
{
	mHighScores.push_back(std::make_pair(tag, score));
	std::sort(mHighScores.begin(), mHighScores.end(),
		[](auto& a, auto& b) { return b.second < a.second; });

	if (mHighScores.size() > 10) mHighScores.pop_back();

	std::ofstream file("highscores.txt");
	for (auto& hs : mHighScores)
		file << hs.first << " " << hs.second << "\n";
	file.close();
}

void Asteroids::LoadHighScores()
{
	std::ifstream file("highscores.txt");
	std::string tag;
	int score;
	mHighScores.clear();

	while (file >> tag >> score)
		mHighScores.push_back(std::make_pair(tag, score));

	file.close();
}


shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}