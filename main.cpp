/**************steps for running with g++ ***************************/
// (with static libraries)

// way 1:
//g++ -DSFML_STATIC -I../sfml_64/include  main.cpp -L../sfml_64/lib -lsfml-graphics-s -lfreetype -lsfml-window-s 
//      -lopengl32 -lgdi32 -lsfml-system-s -lwinmm -o main
// note: the above compile line works, but did not get rid of squiggly error on vs code, which said 
//      "cannot open sfml/graphics.hpp"...
// Note: "sfml_64" is the name of the folder where I had my sfml installation, and it was one directory back 
//    from my project directory, hence the "../".

// way 2:
// added "sfml_headers" folder where I copied include/SFML, and "sfml_libs" folder
// where I copied the 5 static libraries: libreetype.a,libopenal32.a,libsfml-graphics-s.a,
// libsfml-system-s.a,libsfml-window-s.a
//g++ -static -DSFML_STATIC -Isfml_headers main.cpp -Lsfml_libs -lsfml-graphics-s -lfreetype -lsfml-window-s -lopengl32 -lgdi32 -lsfml-system-s -lwinmm
// -o main
// above compile line works, and this way results in 0 errors/warnings in vs code
// note, include the "-static" option above, this will generate a .exe that isn't dependent on any dlls, only the assets folder

// way 3:
// If using VS Code, use the run/debug options, along with the launch.json and tasks.json included
// in this project. This requires the same folders and headers/lib files described in way 2. I added
// "args" to the tasks.json which corresponded to the options from the compile line in way 2.

/*****************************************************************/

// main8.cpp
/*
* [post hoc remark] The entire application can be built from this file (not counting the assets folder and the
   sfml files, which are also needed). In future commits, the project has multiple header and cpp files
*
*  goals for main8:
*  (done) go back into void Skeleton::animate() and try to make it more efficient with the settexturerect
*  (somewhat done) fix the collision bw player and skeleton (idea: check velocity)
*		problems to fix:
*		-(seemingly fixed)I encountered a bug where my character straigh up disappeared when I dashed into skeleguy
*		-if they're moving in the same direction while one is moving faster, they come to a complete stop when they collide
*		-I haven't accounted for the collision when they're moving toward each other vertically, ie falling vs rising
*  (done) make enemies disappear/fade when their healthbars deplete to 0
*  (mostly done) enemy behavior
*  (done)the isDashing() check should happen in a class function
*  - make the player attacks push back the enemies a bit (and later, the enemy attacks too)
*  -(mostly done)add reset screen
*  -(made progress) make the map larger and add more enemies
* 
*  note: 6.6 on learncpp says that comparing floating point vals with any of the relational operators can be dangerous
*		-I should look over my code for instances of this and revise.
* 
*  goals for main9: menu, player death/ death screen, refactor the code a bit (mainly put more code into functions).
*/


#include<SFML/Graphics.hpp>
#include<math.h> //for sqrtf()
#include<iostream>
#include<vector>

static const float VIEW_HEIGHT = 650.0F; //CTRL+F "+300", "950"

void checkCollision(sf::RectangleShape& host, sf::RectangleShape& other, sf::Vector2f& direction);
sf::Vector2f viewTopLeft(sf::RenderWindow& window, sf::View& view, float offsetx, float offsety);

class Surface {
public:
	Surface(sf::Texture* texture, sf::Vector2f size, sf::Vector2f position, int x = 255, int y = 255, int z = 255);
	void draw(sf::RenderWindow& window);
	sf::RectangleShape& Body() { return body; }; //returns a reference to body
private:
	sf::RectangleShape body;
};

class Healthbar {
public:
	Healthbar(sf::Vector2f position, sf::RectangleShape gHealth = sf::RectangleShape(sf::Vector2f(80.0f, 16.0f)),
		sf::RectangleShape rHealth = sf::RectangleShape(sf::Vector2f(80.0f, 16.0f))) {
		greenHealth = gHealth;
		redHealth = rHealth;
		greenHealth.setFillColor(sf::Color::Green);
		redHealth.setFillColor(sf::Color::Red);
		greenHealth.setOrigin(greenHealth.getSize().x / 2, greenHealth.getSize().y / 2);
		redHealth.setOrigin(redHealth.getSize().x / 2, redHealth.getSize().y / 2);
		greenHealth.setPosition(position);
		redHealth.setPosition(position);
	}
	//functions
	void draw(sf::RenderWindow& window) {
		window.draw(redHealth);
		window.draw(greenHealth);
	}
	void subtract(int num) { hp -= num; if (hp < 0) hp = 100; }
	void update() {
		greenHealth.setSize(sf::Vector2f(redHealth.getSize().x * (hp * 0.01), greenHealth.getSize().y));
	}
	void setPosition(const sf::Vector2f& position) {
		greenHealth.setPosition(position);
		redHealth.setPosition(position);
	}
	sf::Vector2f getSize() { return redHealth.getSize(); }
	int getHp() { return hp; }
	void printHealth() { std::cout << hp << std::endl; }
	void setHp(int num) { hp = num; }

private:
	int hp = 100; //this should be the initial value
	sf::RectangleShape greenHealth;
	sf::RectangleShape redHealth;
};

class Character {
public:
	Character(sf::Vector2f size, sf::Vector2f position, sf::Texture* texture, sf::Vector2u imageCount, float switchTime,
		float speed, sf::Texture* atkSprite = nullptr) { //also need to add an initializer list for animation here once I add animation
		this->speed = speed;
		body.setSize(size);
		body.setOrigin(size / 2.0f);
		body.setPosition(position);
		body.setTexture(texture);
		this->imageCount = imageCount; //note, I added this myself, might remove later
		canJump = true;
		this->initialPos = position;
		// the next 3 lines can be removed later, their purpose is to settexture rect when game begins on pause
		sf::Vector2u textureSize = texture->getSize();
		textureSize.x /= imageCount.x; textureSize.y /= imageCount.y;
		body.setTextureRect(sf::IntRect(textureSize.x * (col), textureSize.y * (row), textureSize.x, textureSize.y));
	}
	void draw(sf::RenderWindow& window) { window.draw(body); }
	void animate(float deltaTime); //animate(float deltaTime, sf::Vector2u textureSize)
	sf::Vector2f getPosition() { return body.getPosition(); }
	sf::Vector2f getVelocity() { return velocity; }
	void setVelocity(sf::Vector2f vel) { velocity = vel; }
	sf::RectangleShape& getBody() { return body; } //function that returns a reference
	void onCollision(sf::Vector2f direction); //handles things like canJump, resets target's gravity
	void checkAttackCollision(Character& other, int attackingFrame = 1);
	bool isAttacking() { return attacking; }
	void print() { std::cout << velocity.x << std::endl; } //this function is for testing
	Healthbar& getHealthBar() { return *health; }
	void toggleHealth(int input = -1) {
		healthVisible = !healthVisible; if (input == 0) healthVisible = false;
		if (input == 1) healthVisible = true;
	}
	//void setPosition(sf::Vector2f pos) { body.setPosition(pos); }
protected:
	sf::RectangleShape body;
	int row = 0; //used for animation //starts at 0 (idle)
	int col = 0; //starts at 0 (idle)
	float speed;
	bool faceRight = true;
	bool canJump;
	sf::Vector2f velocity;
	bool attacking = false;
	bool attackLanded = false;
	float attackTime = 0.0f;
	int attackFrame = 0;
	float totalTime = 0; //used for animation
	sf::RectangleShape* attackRect = nullptr; //pointer to attackrect
	Healthbar* health = nullptr;	//pointer to healthbar object
	bool healthVisible = true;
	sf::Vector2u imageCount; //consider reworking this later, after animate functions done
	sf::Vector2f initialPos; // initial position
};

class PlayerCharacter : public Character {
public:
	PlayerCharacter(sf::Vector2f size, sf::Vector2f position, sf::Texture* texture, sf::Vector2u imageCount,
		float switchTime, float speed, sf::Texture* atkSprite = nullptr)
		: Character(size, position, texture, imageCount, switchTime, speed, atkSprite) {
		//not sure if I should use () or {} after Character
		attackRect.setOrigin(75.0f, 67.0f);
		attackRect.setTexture(atkSprite);
		Character::attackRect = &attackRect;
		health = &healthbar; //Character:: might be unnecessary
	}
	void update(float deltaTime); //handles inputs, actions
	void animate(float deltaTime); //handles sprites
	void draw(sf::RenderWindow& window) {
		window.draw(body); health->draw(window); if (attacking && attackFrame > 0) window.draw(attackRect);
	}
	bool isDashing() { return dashing; }
private:
	sf::RectangleShape attackRect = sf::RectangleShape(sf::Vector2f(150.0f, 134.0f));
	Healthbar healthbar = Healthbar(sf::Vector2f(95.0f, 31.0f), sf::RectangleShape(sf::Vector2f(160.0f, 32.0f)),
		sf::RectangleShape(sf::Vector2f(160.0f, 32.0f)));
	bool dashing = false;
	float dashTime = 0.0f;
	float dashCoolDown = 0.0f;
};

class Npc : public Character {
public:
	Npc(sf::Vector2f size, sf::Vector2f position, sf::Texture* texture, sf::Vector2u imageCount, float switchTime,
		float speed, bool moveable, sf::Texture* atkSprite = nullptr) : Character(size, position, texture, imageCount,
			switchTime, speed) {
		this->moveable = moveable;
		healthbar.setPosition(sf::Vector2f(position.x, (position.y - 67) - 20)); //y - (half size) -20
		health = &healthbar;
	}
	void update(float deltaTime, Character* target = nullptr); //
	void animate(float deltaTime);
	void draw(sf::RenderWindow& window) { if(healthVisible) health->draw(window); 
	if (!attacking) window.draw(body); else window.draw(*attackRect); }
	void behave(float deltaTime, Character& target);
protected:
	bool moveable;
	Healthbar healthbar = Healthbar(sf::Vector2f(625.0f, (325.0f - 67) - 20));
	float behaveCooldown = 0.0f;
};

class Skeleton : public Npc {
public:
	Skeleton(sf::Vector2f size, sf::Vector2f position, sf::Texture* texture, sf::Vector2u imageCount, float switchTime,
		float speed, bool moveable, sf::Texture* atkSprite = nullptr) : Npc(size, position, texture, imageCount,
			switchTime, speed, moveable, atkSprite) {
		Character::attackRect = &attackRect; //the npc attackrect pointer points to attackrect
		attackRect.setTexture(atkSprite);
	}
	void update(float deltaTime, Character* target = nullptr);
	void animate(float deltaTime);
	float getFaded() { return fadeFactor; }
	void resetFade() { fadeFactor = 1.0f; }
	void revive();
private:
	sf::RectangleShape attackRect = sf::RectangleShape(sf::Vector2f(184.25f, 184.25));
	float fadeFactor = 1.0f;
};

void movePlayerHealth(sf::RenderWindow& window, sf::View& view, PlayerCharacter& player);
// overloading the function below. In the future I may want to look into doing this with auto or templates
void checkCollision(Character &host, Character & other, sf::Vector2f& direction);

int main() {
	sf::RenderWindow window(sf::VideoMode(950, 650), "Neoninja", sf::Style::Close | sf::Style::Resize);
	bool cameraLocked = true;

	sf::Texture sprite;
	sprite.loadFromFile("assets/newSprites.png");

	sf::Texture attackSprite;
	attackSprite.loadFromFile("assets/attackSprites.png");

	sf::Texture skeleSprite;
	skeleSprite.loadFromFile("assets/skeleWalk.png");

	sf::Texture skeleSprite2;
	skeleSprite2.loadFromFile("assets/skeleWalk2.png");

	sf::Texture skelAttackSprites;
	skelAttackSprites.loadFromFile("assets/skeleguyAttackSprites.png");
	// these vectors are probably unecessary since I added initalPos
	std::vector<sf::Vector2f>Positions;
	Positions.push_back(sf::Vector2f(325.0f, 325.0f));
	Positions.push_back(sf::Vector2f(625.0f, 325.0f));
	Positions.push_back(sf::Vector2f(525.0f, 750.0f));
	Positions.push_back(sf::Vector2f(325.0f, -270.0f)); //325
	Positions.push_back(sf::Vector2f(1000.0f, -270.0f));
	Positions.push_back(sf::Vector2f(1625.0f, 350.0f));

	PlayerCharacter player(sf::Vector2f(100.0f, 134.0f), Positions.at(0), &sprite, sf::Vector2u(3, 3), 0.3f, 300.0f,
		&attackSprite);
	Skeleton skeleguy(sf::Vector2f(100.0f, 134.0f), Positions.at(1), &skeleSprite, sf::Vector2u(3, 1), 0.3f, 300.0f, false,
		&skelAttackSprites);
	Skeleton skeleguy2(sf::Vector2f(100.0f, 134.0f), Positions.at(2), &skeleSprite2, sf::Vector2u(3, 1), 0.3f, 300.0f, false,
		&skelAttackSprites);
	Skeleton skeleguy3(sf::Vector2f(100.0f, 134.0f), Positions.at(3), &skeleSprite2, sf::Vector2u(3, 1), 0.3f, 300.0f, false,
		&skelAttackSprites);
	Skeleton skeleguy4(sf::Vector2f(100.0f, 134.0f), Positions.at(4), &skeleSprite, sf::Vector2u(3, 1), 0.3f, 300.0f, false,
		&skelAttackSprites);
	Skeleton skeleguy5(sf::Vector2f(100.0f, 134.0f), Positions.at(5), &skeleSprite2, sf::Vector2u(3, 1), 0.3f, 300.0f, false,
		&skelAttackSprites);

	std::vector<Skeleton*> skeleguys; //making it a pointer vector bc I want to modify the original variables
	std::vector<Skeleton*> deadSkeles;
	/*skeleguys.push_back( Skeleton(sf::Vector2f(100.0f, 134.0f), Positions.at(0), &skeleSprite, sf::Vector2u(3,1), 0.3f, 300.0f, &skelAttackSprites));
	skeleguys.push_back( Skeleton(sf::Vector2f(100.0f, 134.0f), Positions.at(0), &skeleSprite, sf::Vector2u(3,1), 0.3f, 300.0f, &skelAttackSprites));*/
	skeleguys.push_back(&skeleguy);
	skeleguys.push_back(&skeleguy2);
	skeleguys.push_back(&skeleguy3);
	skeleguys.push_back(&skeleguy4);
	skeleguys.push_back(&skeleguy5);

	//https://www.color-hex.com/color-palette/5997 
	std::vector<Surface> surfaces;
	surfaces.push_back(Surface(nullptr, sf::Vector2f(900.0f, 100.0f), sf::Vector2f(525.0f, 442.0f))); //white
	surfaces.push_back(Surface(nullptr, sf::Vector2f(650.0f, 100.0f), sf::Vector2f(1500.0f, 442.0f), 0)); //cyan
	surfaces.push_back(Surface(nullptr, sf::Vector2f(50.0f, 150.0f), sf::Vector2f(1200.0f, 417.0f), 0)); //cyan
	surfaces.push_back(Surface(nullptr, sf::Vector2f(100.0f, 30.0f), sf::Vector2f(1950.0f, 267.0f), 253, 255, 50));// yellow 
	surfaces.push_back(Surface(nullptr, sf::Vector2f(100.0f, 30.0f), sf::Vector2f(2100.0f, 140.0f), 233, 255, 50));// yellow 
	surfaces.push_back(Surface(nullptr, sf::Vector2f(100.0f, 30.0f), sf::Vector2f(2250.0f, 20.0f), 212, 255, 2));// yellow 
	surfaces.push_back(Surface(nullptr, sf::Vector2f(100.0f, 30.0f), sf::Vector2f(1900.0f, -100.0f), 212, 255, 2));
	surfaces.push_back(Surface(nullptr, sf::Vector2f(1650.0f, 100.0f), sf::Vector2f(750.0f, -210.0f), 255, 0, 167)); //pink

	surfaces.push_back(Surface(nullptr, sf::Vector2f(600.0f, 100.0f), sf::Vector2f(1225.0f, 842.0f), 100, 100)); //purp
	surfaces.push_back(Surface(nullptr, sf::Vector2f(600.0f, 100.0f), sf::Vector2f(425.0f, 882.0f), 0, 255, 70)); //bright greem
	surfaces.push_back(Surface(nullptr, sf::Vector2f(100.0f, 100.0f), sf::Vector2f(175.0f, 782.0f), 0, 255, 70)); //brigh green
	surfaces.push_back(Surface(nullptr, sf::Vector2f(600.0f, 100.0f), sf::Vector2f(-200.0f, 690.0f), 255, 100, 100)); //reddish
	surfaces.push_back(Surface(nullptr, sf::Vector2f(100.0f, 100.0f), sf::Vector2f(-135.0f, 570.0f), 1, 103, 149));

	sf::View view(player.getPosition(), sf::Vector2f(VIEW_HEIGHT + 300, VIEW_HEIGHT));

	// I should probably put this stuff in a function or class later on
	sf::Text label;
	sf::Font font;
	if (!font.loadFromFile("assets/RobotoMono-Regular.ttf")) { std::cout << "yeah no lmao"; }
	label.setFont(font);
	label.setString("PAUSE");
	label.setFillColor(sf::Color::Blue);
	label.setPosition(viewTopLeft(window, view, 10, 50));

	float deltaTime = 0.0f;
	bool pause = true;
	sf::Clock clock; //starts the clock
	std::srand(static_cast<unsigned>(time(nullptr)));

	while (window.isOpen()) {
		deltaTime = clock.restart().asSeconds();
		if (deltaTime > 1.0f / 20.0f)
			deltaTime = 1.0f / 20.0f;

		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::Closed:
				window.close();
				break;

			case sf::Event::Resized:
				//velocity.y = 0.0f; //Is this important? //probably to prevent falling after resize
				sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
				window.setView(sf::View(visibleArea));
				view.setSize(event.size.width, event.size.height);
				break;
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Slash) && !cameraLocked)
		{
			window.setView(view);
			movePlayerHealth(window, view, player);
			label.setPosition(viewTopLeft(window, view, 10, 50));
		}

		static float CameraSwitchTime = 0.0; //this is to prevent rapid toggling
		CameraSwitchTime += deltaTime;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift))
		{
			if (CameraSwitchTime >= 0.25) {
				cameraLocked = !cameraLocked;
				CameraSwitchTime = 0.0;
			}
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)){
			//if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			// I want to reset the world
			int emergencyBreak = 0;
			player.getBody().setPosition(Positions.at(0));
			player.getHealthBar().setHp(100);
			player.getHealthBar().update();
			while(deadSkeles.size() != 0) {
				skeleguys.push_back(deadSkeles.at(0));
				deadSkeles.erase(deadSkeles.begin());
				emergencyBreak++;
				if (emergencyBreak > 10) {
					std::cout << " EMERGENCY BREAK "; break;
				}
			}
			for (int i = 0; i < skeleguys.size(); i++) {
				skeleguys.at(i)->revive();
				/*skeleguys.at(i)->getHealthBar().setHp(100);
				skeleguys.at(i)->resetFade();
				skeleguys.at(i)->toggleHealth();*/
			}
			pause = true;
		}

		//this code is for testing		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Numpad1)) {
			if (CameraSwitchTime >= 0.25) {
				std::cout << "player velocity is "; player.print();
				std::cout << "skeleguy velocity is "; skeleguy.print();
				std::cout << "rand 1: " << rand() << std::endl;
				std::cout << "rand 2: " << rand() << std::endl;
				CameraSwitchTime = 0.0;
			}
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P)) {
			if (CameraSwitchTime >= 0.15) {
				CameraSwitchTime = 0.0;
				pause = !pause;
			}
		}
		/*if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			if (CameraSwitchTime >= 0.25) {
				CameraSwitchTime = 0.0;
				std::cout << "mouse clicked " << sf::Mouse::getPosition().x <<
					" " << sf::Mouse::getPosition().y << std::endl;
			}
		}*/

		//removing skeleguys from the list if their health drops below 0
		for (int i = 0; i < skeleguys.size(); i++) {
			if (skeleguys.at(i)->getFaded() == 0) { //removes them once they're faded
				deadSkeles.push_back(skeleguys.at(i));
				skeleguys.erase(skeleguys.begin() + i);
			}
		}

		//*********Consider rearranging the order of when the update function is called
		if (!pause) { //might want to have the collision detectors in here?
			player.update(deltaTime);   //skeleguy.update(deltaTime);  //skeleguy2.update(deltaTime);
			for (Skeleton* skeleton : skeleguys) skeleton->update(deltaTime, &player);
		}

		sf::Vector2f direction; //the direction of the collision
		for (int i = 0; i < surfaces.size(); i++) {
			checkCollision(player.getBody(), surfaces.at(i).Body(), direction);
			player.onCollision(direction);

			for (Skeleton* skeleton : skeleguys) {
				checkCollision(skeleton->getBody(), surfaces.at(i).Body(), direction);
				skeleton->onCollision(direction);
			}
		}
		//TODO: the isDashing() check should happen in a class function
		if (!player.isDashing()) { //if player is not dashing
			for (Skeleton* skeleton : skeleguys) {
				checkCollision(player, *skeleton, direction);
				player.onCollision(direction);
			}
		}

		//for every enemy
		if (player.isAttacking()) {
			for (Skeleton* skeleton : skeleguys) player.checkAttackCollision(*skeleton);
		}

		for (Skeleton* skeleton : skeleguys){
			if (skeleton->isAttacking()) 
				if (skeleton->getHealthBar().getHp() > 0)
					skeleton->checkAttackCollision(player, 3);
		}

		view.setCenter(player.getPosition());

		window.clear(sf::Color(70, 70, 70));
		if (cameraLocked) { 
			window.setView(view); 
			movePlayerHealth(window, view, player); 
			label.setPosition(viewTopLeft(window, view, 10, 50));
		}
		for (Surface& surface : surfaces)
			surface.draw(window);

		player.draw(window);
		/*skeleguy.draw(window);
		skeleguy2.draw(window);*/
		for (Skeleton* skeleton : skeleguys) skeleton->draw(window);
		if (pause) window.draw(label);
		window.display();
	}
	std::cout << window.getSize().x << std::endl;
	std::cout << window.getSize().y << std::endl;

	return 0;
}

void checkCollision(sf::RectangleShape& host, sf::RectangleShape& other, sf::Vector2f& direction) {
	sf::Vector2f otherPosition = other.getPosition();
	sf::Vector2f otherHalfSize = other.getSize() / 2.0f;
	sf::Vector2f thisPosition = host.getPosition();
	sf::Vector2f thisHalfSize = host.getSize() / 2.0f;

	float deltaX = otherPosition.x - thisPosition.x; //if this is neg, then other is left of this
	float deltaY = otherPosition.y - thisPosition.y; //if this is neg, then other is above this

	float intersectX = abs(deltaX) - (thisHalfSize.x + otherHalfSize.x);
	float intersectY = abs(deltaY) - (thisHalfSize.y + otherHalfSize.y);

	if (intersectX < 0 && intersectY < 0) { //there is a collision
		if (abs(intersectX) < abs(intersectY)) { //more overlap on the y axis
			float speed = 0.1f;					//therefore, push out on x axis
			if (deltaX > 0) {		//collision is on the right of this
				host.move(intersectX, 0.0f);
				direction.x = 1.0f;
				direction.y = 0.0f;
			}
			if (deltaX < 0) {	//collision is on the left of this
				host.move(-intersectX, 0.0f);
				direction.x = -1.0f;
				direction.y = 0.0f;
			}
		}
		if (abs(intersectY) < abs(intersectX)) { //more overlap on the x
			if (deltaY < 0) { // other is above this
				host.move(0.0f, -intersectY);
				direction.x = 0.0f;
				direction.y = 1.0f;
			}
			if (deltaY > 0) { //"if delta.y is bigger than 0.0f, then we're colliding with something underneath us"
				direction.x = 0.0f;
				direction.y = -1.0f;
				host.move(0.0f, intersectY);
			}
		}
	}
	else { //there is no collision
		direction.x = 0.0f;
		direction.y = 0.0f;
	}
}

sf::Vector2f viewTopLeft(sf::RenderWindow& window, sf::View& view, float offsetx, float offsety) {
	sf::Vector2f viewCent = window.getView().getCenter();
	sf::Vector2f TopLeft(viewCent.x - view.getSize().x / 2 + offsetx, viewCent.y - view.getSize().y / 2 + offsety);
	return TopLeft;
}
void movePlayerHealth(sf::RenderWindow& window, sf::View& view, PlayerCharacter& player) {
	player.getHealthBar().setPosition(viewTopLeft(window, view, player.getHealthBar().getSize().x / 2 + 15,
		player.getHealthBar().getSize().y / 2 + 15));
}


Surface::Surface(sf::Texture* texture, sf::Vector2f size, sf::Vector2f position, int x, int y, int z) {
	body.setPosition(position);
	body.setSize(size);
	body.setOrigin(size / 2.0f);
	body.setTexture(texture);
	body.setFillColor(sf::Color(x, y, z));
}
void Surface::draw(sf::RenderWindow& window) {
	window.draw(body);
}

void Character::animate(float deltaTime) {
	sf::Vector2u textureSize = body.getTexture()->getSize();
	textureSize.x /= imageCount.x;
	textureSize.y /= imageCount.y;
	if ((imageCount.x == 1) && (imageCount.y == imageCount.x)) {
		row = col = 0;//(idle) // I might want to expand or modify this later
	}

	if (faceRight)
		body.setTextureRect(sf::IntRect(textureSize.x * (col), textureSize.y * row, textureSize.x, textureSize.y));
	else //facing left
		body.setTextureRect(sf::IntRect((textureSize.x) * (col + 1), textureSize.y * row, 0 - (textureSize.x), textureSize.y));
}

void Character::onCollision(sf::Vector2f direction) {
	if (direction.x < 0.0f) //collision on the left, so stop movement
		velocity.x = 0.0f;
	else if (direction.x > 0.0f) { //collision on the right
		velocity.x = 0.0f;
	}
	if (direction.y < 0.0f) { //collision below us
		velocity.y = 0.0f;
		canJump = true;
	}
	if (direction.y > 0.0f) { //collision above
		velocity.y = 0.0f;
	}
}

void Character::checkAttackCollision(Character& other, int attackingFrame) { //add an extra parameter, like damage
	sf::Vector2f otherPosition = other.getPosition();
	sf::Vector2f otherHalfSize = other.getBody().getSize() / 2.0f;
	sf::Vector2f thisPosition = attackRect->getPosition(); //attackRect is a pointer
	sf::Vector2f thisHalfSize = attackRect->getSize() / 2.0f;

	float deltaX = otherPosition.x - thisPosition.x; //if this is neg, then other is left of this
	float deltaY = otherPosition.y - thisPosition.y; //if this is neg, then other is above this

	float intersectX = abs(deltaX) - (thisHalfSize.x + otherHalfSize.x);
	float intersectY = abs(deltaY) - (thisHalfSize.y + otherHalfSize.y);

	if (intersectX < 0 && intersectY < 0 && !attackLanded) { //there is a collision
		if (attackFrame >= attackingFrame) { //the first frame is a startup frame
			std::cout << "Attack Landed! ";
			attackLanded = true;
			other.getHealthBar().subtract(25);
			other.getHealthBar().printHealth();
			other.getHealthBar().update();
		}
	}
}

void PlayerCharacter::update(float deltaTime) {
	velocity.x = 0.0f;
	float jumpHeight = 150.0f; //note, I can makes this a parameter/member var

	if (velocity.y > 0.0f) //remember that the y is inverted, so positive y means going downward
		canJump = false; //when falling, canJump = false

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && (!attacking))
		velocity.x -= speed;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && (!attacking))
		velocity.x += speed;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && canJump && (!attacking)) {
		canJump = false;
		velocity.y = -sqrtf(2.0f * 981.0f * jumpHeight);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && (!attacking) && (canJump)) {
		attacking = true;
		velocity.x = 0.0; //halts player while attacking
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Numpad0) && (!attacking) && (!dashing)
		&& (dashCoolDown >= 0.0f)) {
		dashing = true;
		dashCoolDown = -0.5f;
	}
	if (dashCoolDown < 0.0f) dashCoolDown += deltaTime;
	if (dashing) {
		int faceFactor = (faceRight) ? 1 : -1;
		velocity.x += faceFactor * 2000; // speed is 300.0 by default
		velocity.y = 0.0f;
		dashTime += deltaTime;
		if (dashTime > 0.10f) {
			dashing = false;
			dashTime = 0.0f;
		}
	}
	else velocity.y += 981.0f * deltaTime; //no gravity while dashing

	if (velocity.x == 0.0f) { //idle animation
		row = 0; //row for idle animation
	}
	else {
		//row = 0; //row for movement animation
		if (velocity.x > 0.0f)
			faceRight = true;
		else
			faceRight = false;
	}

	sf::Vector2u textureSize = body.getTexture()->getSize(); //proviional
	textureSize.x /= imageCount.x; //provisional
	textureSize.y /= imageCount.y; //provisional

	animate(deltaTime);
	body.move(velocity * deltaTime);

	float attackRectDistance = getBody().getSize().x / 2.0f + attackRect.getSize().x / 2.0f;
	if (!faceRight) attackRectDistance *= -1;
	attackRect.setPosition(getBody().getPosition().x + attackRectDistance, getBody().getPosition().y);
}

void PlayerCharacter::animate(float deltaTime) { //
	totalTime += deltaTime;
	float switchTime = 0.2f;
	float attackSwitch = 0.08f; //I had it at 0.15 before

	sf::Vector2u texSize = body.getTexture()->getSize();
	texSize.x /= 3;
	texSize.y /= 3;

	sf::Vector2u atkTexSize = attackRect.getTexture()->getSize();
	atkTexSize.x /= 3;

	std::vector<sf::IntRect> attFrames;
	attFrames.push_back(sf::IntRect(texSize.x * 2, texSize.y * 1, texSize.x, texSize.y));
	attFrames.push_back(sf::IntRect(texSize.x * 0, texSize.y * 2, texSize.x, texSize.y));
	attFrames.push_back(sf::IntRect(texSize.x * 1, texSize.y * 2, texSize.x, texSize.y));
	attFrames.push_back(sf::IntRect(texSize.x * 2, texSize.y * 2, texSize.x, texSize.y));

	if (dashing) {
		if (faceRight) body.setTextureRect(sf::IntRect(texSize.x * 1, texSize.y * 1, texSize.x, texSize.y));
		else body.setTextureRect(sf::IntRect(texSize.x * 2, texSize.y * 1, 0 - texSize.x, texSize.y));
	}
	else if (canJump == false) {
		body.setTextureRect(sf::IntRect(texSize.x * 1, texSize.y * 0, texSize.x, texSize.y));
		if (!faceRight) body.setTextureRect(sf::IntRect(texSize.x * 2, texSize.y * 0, 0 - texSize.x, texSize.y));
	}
	else { //not jumping and not dashing
		if (velocity.x == 0) {
			row = col = 0; //idle

			if (attacking) {
				attackRect.setSize(sf::Vector2f(150.0f, 134.0f));
				attackTime += deltaTime;
				col = attFrames.at(attackFrame).left / texSize.x;
				row = attFrames.at(attackFrame).top / texSize.y;
				if (attackTime >= attackSwitch) { //change later
					attackTime -= attackSwitch;
					attackFrame++;
					if (attackFrame > 3) {
						attackFrame = 0;
						attackRect.setSize(sf::Vector2f(0.0f, 0.0f));
						attacking = false;
						attackLanded = false;
					}
					int faceLeft = 0;
					if (!faceRight) faceLeft = -1;
					if (attackFrame > 0) //for face left we want to add x, then negative x,
						attackRect.setTextureRect(sf::IntRect(atkTexSize.x * (attackFrame - 1 - faceLeft),
							atkTexSize.y * 0, atkTexSize.x * (1 + 2 * faceLeft), atkTexSize.y));
				}
			}
		}
		else { //not idle, i.e. running
			//row = 1; //for run1, it should be row 0, col 2. Run 2 is row1 col1
			if (totalTime >= switchTime) { //if it's less than switch time, col =0
				totalTime -= switchTime;
				if (row == 0) { row = 1; col = 0; }
				else { row = 0; col = 2; }
			}
		}
		Character::animate(deltaTime); //fips the sprite when going left/right
	}
}

void Npc::update(float deltaTime, Character* target) {
	if (!moveable && target != nullptr && health->getHp()>0) { //if moveable false and target is not null
		behave(deltaTime, *target);
	}
	else velocity.x = 0.0f;
	if (moveable && !attacking) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			velocity.x -= speed;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			velocity.x += speed;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Period) && (!attacking) && (canJump)) {
			attacking = true;
			if (attackFrame == 0) attackFrame = 1;
			velocity.x = 0.0; //halts player while attacking
		}
	}


	if (velocity.x == 0.0f) { //idle animation
		row = 0; //row for idle animation
	}
	else {
		row = 0; //row for movement animation
		if (velocity.x > 0.0f)
			faceRight = true;
		else faceRight = false;
	}

	velocity.y += 981.0f * deltaTime; //adding gravity

	sf::Vector2u textureSize = body.getTexture()->getSize(); //proviional
	textureSize.x /= imageCount.x; //provisional
	textureSize.y /= imageCount.y; //provisional
	body.setTextureRect(sf::IntRect(0, 0, textureSize.x, textureSize.y)); //default

	//note: originally, animate(delaTime) was before body.move(...)
	body.move(velocity * deltaTime);
	health->setPosition(sf::Vector2f(body.getPosition().x,
		(body.getPosition().y - body.getSize().y / 2) - 20)); //move healthbar above npc
}

void Skeleton::update(float deltaTime, Character* target) {
	if (health->getHp() != 0) {
		Npc::update(deltaTime, target);
		animate(deltaTime);

		if (attacking) {
			float X = attackRect.getSize().x / 2.0f - getBody().getSize().x / 2.0f; //0, then +42 (working)
			float Y = attackRect.getSize().y / 2.0f - getBody().getSize().y / 2.0f;
			if (!faceRight) X *= -1;
			attackRect.setPosition(getBody().getPosition().x + X, getBody().getPosition().y - Y);
		}
	}
	//this should prob be moved to Npc::update
	else { /*if (health->getHp() == 0)*/  //or less than 0...
		toggleHealth(0);
		if (fadeFactor > 0.0f) fadeFactor -= deltaTime;
		else fadeFactor = 0.0f;
		body.setFillColor(sf::Color(255, 255, 255, 255*fadeFactor));
		if (attacking) attackRect.setFillColor(sf::Color(255, 255, 255, 255 * fadeFactor));
	}
}
void Skeleton::animate(float deltaTime) { //623 to 672
	//totalTime += deltaTime;
	totalTime += deltaTime;
	float switchTime = 0.2f;
	float attackSwitch = 0.15f;
	sf::Vector2u atktexSize = attackRect.getTexture()->getSize();
	atktexSize.y /= 2;
	(attackFrame < 4) ? atktexSize.x /= 5.5f : atktexSize.x /= 3.0f;

	if (attacking) {
		attackTime += deltaTime;

		if (attackTime >= attackSwitch) {
			attackTime -= attackSwitch;
			attackFrame++;
		}
		//attackFrame will be 1 thru 6 (inclusive) with 0 being the idle attackFrame
		int i = attackFrame;
		if (attackFrame > 0) {
			int faceLeft = 0;
			if (!faceRight) faceLeft = -1;
			if (attackFrame < 4) { //width of each one is 18 here, or 100
				body.setSize(sf::Vector2f(100.0f, 160.0f));
				body.setOrigin(sf::Vector2f(50.0f, 80.0f));
				attackRect.setSize(sf::Vector2f(100.0f, 184.25f)); //later it will be 184 184
				attackRect.setOrigin(sf::Vector2f(50.0f, 92.125f));
				attackRect.setTextureRect(sf::IntRect(atktexSize.x * (i - 1 - faceLeft), atktexSize.y * 0, atktexSize.x * (1 + 2 * faceLeft), atktexSize.y));
			}
			else if (attackFrame < 7){ //frames 4,5,6
				attackRect.setSize(sf::Vector2f(184.25f, 184.25f)); //later it will be 184 184
				attackRect.setOrigin(sf::Vector2f(92.125f, 92.125f));
				attackRect.setTextureRect(sf::IntRect(atktexSize.x * (i - 4 - faceLeft), atktexSize.y * 1, atktexSize.x * (1 + 2 * faceLeft), atktexSize.y));
			}
		}
		if (attackFrame > 6) {
			//half size of 184 -  half size of 134
			float displacement = 92.125f - 67.0f; //some 25
			body.setSize(sf::Vector2f(100.0f, 134.0f));
			body.setOrigin(sf::Vector2f(50.0f, 67.0f)); //bring him back down by the distance he changed

			body.setPosition(sf::Vector2f(body.getPosition().x, body.getPosition().y + displacement));
			attacking = false;
			attackLanded = false;
			attackFrame = 0;
			attackRect.setSize(sf::Vector2f(0.0f, 0.0f));
		}
	}
	else { //not jumping and not dashing
		if (velocity.x == 0) {
			row = col = 0; //idle
		}
		else { //not idle, ie running
			if (totalTime > switchTime) {
				totalTime -= switchTime;
				(col == 2) ? col = 1 : col = 2;
			}
		}
	}
	Character::animate(deltaTime);
}
void Skeleton::revive() {
	health->setHp(100);
	health->update();
	fadeFactor = 1.0f; //resetting the fade factor
	body.setFillColor(sf::Color(255, 255, 255)); //back to normal
	attackRect.setFillColor(sf::Color(255, 255, 255, 255));//back to norm
	body.setPosition(initialPos);
	health->setPosition(sf::Vector2f(initialPos.x, initialPos.y - (67 + 20)));
	attacking = false;
	attackFrame = 0;
	toggleHealth(1);
}
void Npc::behave(float deltaTime, Character& target) {
	// this should prob be called inside npc update()
	//assumes moveable is false
	sf::Vector2f otherPosition = target.getPosition();
	sf::Vector2f otherHalfSize = target.getBody().getSize() / 2.0f;
	sf::Vector2f thisPosition = getPosition();
	sf::Vector2f thisHalfSize = getBody().getSize() / 2.0f;

	float deltaX = otherPosition.x - thisPosition.x; //if this is neg, then target is left of this
	float deltaY = otherPosition.y - thisPosition.y; //

	float Xdistance = abs(body.getPosition().x  - target.getPosition().x);
	float Ydistance = abs(body.getPosition().y - target.getPosition().y);

	// plan: 
	// step 1: look
		// I'll set the distance to 600 for now
		//if the absolute distance <= 600
		// optional, you can add both half sizes in there, ie 50
		// note, this doesn't take into account obstacles that obstruct vision
		// note, the skelattack extends to 84.25 beyond its body. Other enemies may have different
			//the hitbox would be 84.25 + 67 (player half size)
	// step 2: if found, pursue, else idle
	if (behaveCooldown < 0.0f) behaveCooldown += deltaTime; //continue previous behavior
	else if (Xdistance <= 650 && Ydistance <= 135) { // within sights
		velocity.x = 0;					//reset the velocity (so as to not accelerate)
		if (Xdistance < 151.25) {	//within attacking distance
			//velocity.x = 0.0f; //temporary
			behaveCooldown = -0.5f;
			unsigned int choice = rand() % 4; //rand num of either 0,1,2,3
			if (attacking == true) { // if still attacking, wait until attack is done
				velocity.x = 0.0f;
			}
			else if (choice == 0) {
				velocity.x -= speed;
			}
			else if (choice == 1) {
				velocity.x += speed;
			}
			else {
				attacking = true;
				if (attackFrame == 0) attackFrame = 1;
				velocity.x = 0.0;
			}
		}
		else if (deltaX < 0 && !attacking) // target is on the left, so move left
			velocity.x -= speed;
		else if (deltaX > 0 && !attacking) velocity.x += speed; // target is on the right, so move right
		//else std::cout << " do nothing, attacking: " << attacking;
	}
	// step 3: if distance closed, randomly do one of the every variable time:
		// move forward, move backward, attack
		// set cooldown that prevents further action until specified time
}

// collision for two Characters
void checkCollision(Character& host, Character& other, sf::Vector2f& direction) {
	sf::Vector2f otherPosition = other.getPosition();
	sf::Vector2f otherHalfSize = other.getBody().getSize() / 2.0f;
	sf::Vector2f thisPosition = host.getPosition();
	sf::Vector2f thisHalfSize = host.getBody().getSize() / 2.0f;

	float deltaX = otherPosition.x - thisPosition.x; //if this is neg, then other is left of this
	float deltaY = otherPosition.y - thisPosition.y; //if this is neg, then other is above this

	float intersectX = abs(deltaX) - (thisHalfSize.x + otherHalfSize.x);
	float intersectY = abs(deltaY) - (thisHalfSize.y + otherHalfSize.y);

	//if both arguments belong to the character class
	bool dash = false;
	float hostVelocityX = host.getVelocity().x;
	float otherVelocityX = other.getVelocity().x;
	if (abs(hostVelocityX) > 1999.0f) {
		(hostVelocityX > 0) ? hostVelocityX = 300.0f : hostVelocityX = -300.0f;
		dash = true;
	}
	float totalVelocity = abs(hostVelocityX) + abs(other.getVelocity().x);

	if (intersectX < 0 && intersectY < 0) { //there is a collision
		if (abs(intersectX) < abs(intersectY)) { //more overlap on the y axis
			float speed = 0.1f;					//therefore, push out on x axis
			if (deltaX > 0) {		//collision is on the right of this
				if (totalVelocity == 0.0) { //this happens when skeleguy attacks, while both stationary
					std::cout << "   AAAAAAAAAAHHHHHH222 " << std::endl;
					//both X velocities are presumed to be 0
					totalVelocity = 1.0f;
					hostVelocityX = 1.0f;
				}
				if (dash && hostVelocityX < 0) //dashed left to opposite side of other
				{
					intersectX *= -1;
					std::cout << "dashed left";
				}
				if (dash && hostVelocityX == other.getVelocity().x) { //dashed while both moving in same direc
					hostVelocityX *= 2;
					// if player ends up on the left side, then always push him out on the left side
					otherVelocityX = 0.0f;
				}
				if (abs(hostVelocityX) > 300 || abs(intersectX) > 300) {
					std::cout << "oof ";
					std::cout << "intersect x: " << intersectX << std::endl;
					std::cout << "host velocity " << hostVelocityX << std::endl;
				}
				if (abs(intersectX * (hostVelocityX / totalVelocity)) > 100.0f) {
					std::cout << "something prob wrong. deltaX: " <<deltaX<< std::endl;
					std::cout << "intersect x: " << intersectX << std::endl;
					std::cout << "host velocity " << hostVelocityX << std::endl;
					std::cout << "total velocity" << totalVelocity << std::endl;
				}
 				host.getBody().move(intersectX * (hostVelocityX / totalVelocity), 0.0f);
				//^if player is moving right and collision is on the right, then 
				// velocity is pos and intsctx is neg, therefore player is moved left
				other.getBody().move(intersectX * (otherVelocityX / totalVelocity), 0.0f);
				// if player is moving right and other is moving left, then intersectx is neg
				// and other.velocity is neg, therefore other is moved right

				//note: after player.onCollistion(), player's velocity.x is set to 0 but 
				 // other's is not
				direction.x = 1.0f;
				direction.y = 0.0f;
			}
			if (deltaX < 0) {	//collision is on the left of this
				if (totalVelocity == 0.0) { //this happens when skeleguy attacks, while both stationary
					// can also happen when player dashes into enemy moving in the same direction
					std::cout << "   AAAAAAAAAAHHHHHH222 " << std::endl;
					//both X velocities are presumed to be 0
					totalVelocity = 1.0f;
					hostVelocityX = 1.0f;
				}
				if (abs(hostVelocityX) > 300 || abs(intersectX) > 300) {
					std::cout << "oof ";
					std::cout << "intersect x: " << intersectX << std::endl;
					std::cout << "host velocity " << hostVelocityX << std::endl;
				}
				if (abs(intersectX * (hostVelocityX / totalVelocity)) > 100.0f) {
					std::cout << "something prob wrong. deltaX: " << deltaX << std::endl;
					std::cout << "intersect x: " << intersectX << std::endl;
					std::cout << "host velocity " << hostVelocityX << std::endl;
					std::cout << "total velocity" << totalVelocity << std::endl;
				}
				if (dash && hostVelocityX > 0) //dashed right to opposite side of other
				{
					intersectX *= -1;
					std::cout << "dashed right";
				}
				if (dash && hostVelocityX == other.getVelocity().x) { //dashed while both moving in same direc
					hostVelocityX *= 2;
					// if player ends up on the left side, then always push him out on the left side
					otherVelocityX = 0.0f;
				}
				host.getBody().move(intersectX * (hostVelocityX / totalVelocity), 0.0f);
				//^if player is moving left and collision is on the left, then 
				// velocity is neg and intsctx is neg, th4 player is moved right
				other.getBody().move(intersectX * (otherVelocityX / totalVelocity), 0.0f);
				// if player is moving left and other is moving right, then intersectx is neg
				// and other.velocity is pos, therefore other is moved left
				direction.x = -1.0f;
				direction.y = 0.0f;
			}
		}
		if (abs(intersectY) < abs(intersectX)) { //more overlap on the x
			if (deltaY < 0) { // other is above this
				if (abs(hostVelocityX) > 300 || abs(intersectX) > 300) {
					std::cout << "oof ";
				}
				if (abs(intersectX * (hostVelocityX / totalVelocity)) > 100.0f) {
					std::cout << "something prob wrong. deltaX: " << deltaX << std::endl;
					std::cout << "intersect x: " << intersectX << std::endl;
					std::cout << "host velocity " << hostVelocityX << std::endl;
					std::cout << "total velocity" << totalVelocity << std::endl;
				}
				//if the collision is above, always push other out
				//host.getBody().move(0.0f, -intersectY);
				other.getBody().move(0.0f, intersectY); //intsctY is neg, so we're pushing other up
				//set other velocity to 0
				other.setVelocity(sf::Vector2f(other.getVelocity().x, 0.0f)); //resetting the gravity
				direction.x = 0.0f;
				direction.y = 1.0f;
			}
			if (deltaY > 0) { //"if delta.y is bigger than 0.0f, then we're colliding with something underneath us"
				if (abs(hostVelocityX) > 300 || abs(intersectX) > 300) {
					std::cout << "oof ";
				}
				if (abs(intersectX * (hostVelocityX / totalVelocity)) > 100.0f) {
					std::cout << "something prob wrong. deltaX: " << deltaX << std::endl;
					std::cout << "intersect x: " << intersectX << std::endl;
					std::cout << "host velocity " << hostVelocityX << std::endl;
					std::cout << "total velocity" << totalVelocity << std::endl;
				}
				host.getBody().move(0.0f, intersectY);//other is beneath, so host is pushed up
				direction.x = 0.0f;
				direction.y = -1.0f;
			}
		}
	}
	else { //there is no collision
		direction.x = 0.0f;
		direction.y = 0.0f;
	}
}