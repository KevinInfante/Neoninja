#pragma once
#include<SFML/Graphics.hpp>

//const int screenWidth = sf::VideoMode::getDesktopMode().width; //1920
//const int screenHeight = sf::VideoMode::getDesktopMode().height; //1080
//const sf::Vector2f screenCenter(screenWidth / 2.0f, screenHeight / 2.0f); //960, 540

bool isMouseHovering(sf::Vector2f position, sf::Vector2f size, sf::RenderWindow& window);

class Menu {
public:
	//constructor
	Menu(sf::RenderWindow& window, sf::View& view, sf::Font& font, const sf::String& name = "menuName",
		sf::Vector2f size = sf::Vector2f(300.0f, 300.0f), sf::Vector2f pos = sf::Vector2f(325.0f, 325.0f), 
		sf::Color bgColor = sf::Color::Black, sf::Color txtColor = sf::Color::Blue);
	// will need a draw function for drawing the menu object to the screen
	void draw(sf::RenderWindow& window);
	bool isActive() { return active; }
	void toggleActive() { active = !active; }

	float getMenuNameWidth() { return menuName.getLocalBounds().width; }
	void readMenuInputs(sf::RenderWindow& window, sf::Event& event, float deltaTime=0.0f, sf::Color optionsColor=sf::Color::Blue);
	//returns the centered origin
	sf::Vector2f getPosition() {
		//the body's origin isn't centered, but I'll return the centered origin
		return body.getPosition() / 2.0f;
	}
	void setPosition(sf::Vector2f position) { body.setPosition(position); placeOptions(txtColor); }
																	//the constructor defaults ^txtColor to blue
	void placeOptions(sf::Color optionsColor = sf::Color::Blue); 
	void setUpControlsPage(sf::Font& font, sf::Vector2f size);
protected:
	// make a body(sf::rectangle) which will then be drawn by the draw function
	sf::RectangleShape body; // this should prob be made a protected var later (for the child classes to access it)
	sf::Text menuName;
	bool active = false;
	std::vector<sf::Text*>Options;

	bool isControlPressed = false;
	sf::Text Back;
	sf::Text ControlsPage;

	std::vector<int>UnselectableOptionIndexes;
	sf::Color txtColor;
};

class PauseMenu : public Menu {
public:
	PauseMenu(sf::RenderWindow& window, sf::View& view, sf::Font& font, const sf::String& name, 
		sf::Vector2f size, sf::Vector2f pos, sf::Color bgColor = sf::Color::Cyan);
	void readMenuInputs(sf::RenderWindow& window, sf::Event& event, float deltaTime = 0.0f);
	void draw(sf::RenderWindow& window);
	bool getIsHidden() { return isHidden;  }
	void setIsHidden(bool hide) { isHidden = hide; }
	void setPosition(sf::Vector2f position);

private:
	sf::Text Unpause;
	sf::Text Controls;
	sf::Text Hide;
	sf::Text Quit;

	sf::Text QuitQuestion;
	sf::Text Yes;
	sf::Text No;

	bool isHidden = false;
};

class StartMenu : public Menu {
public:
	StartMenu(sf::RenderWindow& window, sf::View& view, sf::Font& font, const sf::String& name,
	   sf::Vector2f size, sf::Vector2f pos, sf::Color bgColor = sf::Color(20,20,20), 
		sf::Color txtColor = sf::Color::Blue);
	void readMenuInputs(sf::RenderWindow& window, sf::Event& event, float deltaTime);
	void draw(sf::RenderWindow& window);
private:
	sf::Text Controls;
	sf::Text About;
	sf::Text Quit;

	sf::Text QuitQuestion;
	sf::Text Yes;
	sf::Text No;
	sf::Text AboutPage;
	bool isAboutPressed = false;
};

class DeathMenu : public Menu {
public:
	DeathMenu(sf::RenderWindow& window, sf::View& view, sf::Font& font, const sf::String& name,
		sf::Vector2f size, sf::Vector2f pos, sf::Color bgColor = sf::Color::Black,
		sf::Color txtColor = sf::Color::Red);
	void readMenuInputs(sf::RenderWindow& window, sf::Event& event,float deltaTime, sf::Color optionsColor = sf::Color::Red);
	//void draw(sf::RenderWindow& window);
	void setPosition(sf::Vector2f position) { body.setPosition(position); placeOptions(sf::Color::Red); }
	bool getRestartPressed() { return restartPressed; }
	void setRestartPressed(bool state) { restartPressed = state; }
	bool getRevivePressed() { return revivePressed; }
	void setRevivePressed(bool state) { revivePressed = state; }
private:
	sf::Text Revive;
	sf::Text Restart;
	sf::Text Quit;

	bool restartPressed = false;

	sf::Text quitQuestion;
	sf::Text Yes;
	sf::Text No;

	bool revivePressed = false;
};

class LevelMenu : public Menu {
public:
	LevelMenu(sf::RenderWindow& window, sf::View& view, sf::Font& font, const sf::String& name,
		sf::Vector2f size, sf::Vector2f pos, sf::Color bgColor = sf::Color(10, 0, 130), //setting bg here is important
		sf::Color txtColor = sf::Color::Yellow); //setting text color here is also important
	void setTime(const std::string& gameTime) { Time.setString(gameTime); }
	void setScore(const std::string& gameScore) { Score.setString(gameScore); }
	void readMenuInputs(sf::RenderWindow& window, sf::Event& event, float deltaTime = 0.0f, sf::Color optionsColor = sf::Color::Yellow);
	//need to write an overload of placeOptions to set the textColor, OR do some rewriting of class Menu
	// the Menu::setPosition() func calls placeOptions(), and its default color is blue
	bool getRestartPressed() { return restartPressed; }
	void setRestartPressed(bool state) { restartPressed = state; }
	void resetOptions();
private:
	sf::Text Time;
	sf::Text Score;
	sf::Text Advance;
	sf::Text Restart;
	sf::Text Quit;
	sf::Text quitQuestion;
	sf::Text Yes;
	sf::Text No;

	sf::Text Back;
	sf::Text advancePage;
	bool restartPressed = false;
};