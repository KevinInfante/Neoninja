

#include<SFML/Graphics.hpp>
#include<iostream>
#include<vector>
#include"Menu.hpp"

bool isMouseHovering(sf::Vector2f position, sf::Vector2f size, sf::RenderWindow& window) {
	sf::Vector2f halfSize = size / 2.0f;

	const int screenWidth = sf::VideoMode::getDesktopMode().width;
	const int screenHeight = sf::VideoMode::getDesktopMode().height;
	// Note: the window position + screen size/2 gives the current center of the 
					// window in screen coordinates
	// if window position + windowHalfSize is not screenCenter, then window is not centered.
	//if()
	float offsetX = 0.0f;
	float offsetY = 0.0f;

	if (window.getPosition().x + window.getSize().x / 2.0f == screenWidth / 2.0f &&
		window.getPosition().y + window.getSize().y / 2.0f == screenHeight / 2.0f) {
		//if true, window is centered (default)
		if (window.getSize().x == 950 && window.getSize().y == 650) {
			//if true, window is default size
			offsetX = 12.0f;
			offsetY = 45.0f;
		}
		// if it is centered, but not default size...
	}
	else if (window.getSize().x != screenWidth) { //not in full screen
		//subtract the distance from the center to get the offset
		offsetX = (window.getPosition().x + window.getSize().x / 2.0f) -
			(screenWidth / 2.0f) + 12; //add or subtract the default offset?
		offsetY = (window.getPosition().y + window.getSize().y / 2.0f) -
			(screenHeight / 2.0f) + 45;

	} //else it is in full screen, in which case offset should remain 0

	// get mouse position (returns screen coordinates)
	float mousePosX = sf::Mouse::getPosition().x;
	float mousePosY = sf::Mouse::getPosition().y;
	// mousePositions relative to the screen's center:
	float relativeMousePosX = mousePosX - screenWidth / 2.0f;
	float relativeMousePosY = mousePosY - screenHeight / 2.0f;

	//object positions relative to the view's center (world coordinates)
	float relativeObjPosX = position.x - window.getView().getCenter().x;
	float relativeObjPosY = position.y - window.getView().getCenter().y;

	relativeObjPosY += offsetY; // adding an offset to account for window's title bar
	relativeObjPosX += offsetX; // not sure why, but this helps too

	// the rest is typical for a collision function
	float deltaX = relativeMousePosX - relativeObjPosX;
	float deltaY = relativeMousePosY - relativeObjPosY;

	float intersectX = abs(deltaX) - halfSize.x;
	float intersectY = abs(deltaY) - halfSize.y;

	if (intersectX <= 0 && intersectY <= 0) return true; 
	else return false;
}

//globals
//static const float VIEW_HEIGHT = 650.0F;
const int screenWidth = sf::VideoMode::getDesktopMode().width; //1920
const int screenHeight = sf::VideoMode::getDesktopMode().height; //1080
const sf::Vector2f screenCenter(screenWidth / 2.0f, screenHeight / 2.0f); //960, 540


Menu::Menu(sf::RenderWindow& window, sf::View& view, sf::Font& font, const sf::String& name,
	sf::Vector2f size, sf::Vector2f pos, sf::Color bgColor, sf::Color txtColor) {
	//(for starters) make the body 40% the size of the view, and center it

	body.setSize(size); 
	body.setFillColor(bgColor);

	//sf::Vector2f viewCent = window.getView().getCenter();
	body.setOrigin(body.getSize() / 2.0f);
	body.setPosition(pos);

	menuName = sf::Text(name, font); //size paramter is 30 by default
	menuName.setFillColor(txtColor);
	this->txtColor = txtColor;
	//menuName.setOrigin(menuName.getLocalBounds().getSize() / 2.0f);
	////subtracting 3 because it says that the left bound is offset by 3
	//menuName.setPosition(body.getPosition().x - 3.0f + body.getSize().x / 2.0f,
	//	body.getPosition().y + 5);
	//std::cout << "viewcent x and y: " << viewCent.x << " " << viewCent.y << "\n";
	////^outputs 325 325
}
void Menu::draw(sf::RenderWindow& window) {
	window.draw(body);
	//window.draw(menuName); // not needed right now bc it's in the Options vector
	for (int i = 0; i < Options.size(); i++) {
		window.draw(*(Options.at(i)));
	}
}
void Menu::readMenuInputs(sf::RenderWindow& window, sf::Event& event, float deltaTIme, sf::Color optionsColor) {
	//if the cursor hovers over the labels, highlight the labels
	for (int i = 0; i < Options.size(); i++) {
		bool indexFound = false;
		for (int j = 0; j < UnselectableOptionIndexes.size(); j++) {
			if (UnselectableOptionIndexes.at(j) == i){
				indexFound = true;
				break;
			}
		}
		if (indexFound == true) continue;
		if (isMouseHovering(Options.at(i)->getPosition(), Options.at(i)->getLocalBounds().getSize(), window)) {
			if (Options.at(i)->getFillColor() != sf::Color::White) {
				Options.at(i)->setFillColor(sf::Color::White);
				//Options.at(i)->setScale(1.3, 1.3);
				//std::cout << "Mouse is hovering over " << i << "\n";
			}
			//if mouse click, toggle bool, print statement
			if (event.type == sf::Event::MouseButtonPressed) { //TODO: get rid of this whole if block
				if (event.mouseButton.button == sf::Mouse::Left) {
					std::string labelName = Options.at(i)->getString();
					//std::cout << "\n" << labelName << " clicked\n";
				}
			}
		}
		else if (Options.at(i)->getFillColor() == sf::Color::White) {
			Options.at(i)->setFillColor(optionsColor);
			//Options.at(i)->setScale(1, 1);
		}
	}
}
void Menu::placeOptions(sf::Color optionsColor) {
	//by making firstIteration on optional paramter, we allow the option to, e.g.,
	//set firstIter to 1, and then place label(s) above the Options that aren't selectable
	for (int i = 0; i < Options.size(); i++) {
		Options.at(i)->setOrigin(Options.at(i)->getLocalBounds().getSize() / 2.0f);
		//^ assumes they're same size as menu
		Options.at(i)->setFillColor(optionsColor);

		double size = Options.size(); //I want this to return double to avoid integer division

		float xPosition = body.getPosition().x - 3.0f;
		float yPosition = body.getPosition().y - 9.0 - body.getSize().y / 2.0f
			+ ((0.5) * (1.0 / size) * body.getSize().y + (i / size) * body.getSize().y);
		//std::cout << "added " << i << " " << yPosition - body.getPosition().y << "\n";

		Options.at(i)->setPosition(sf::Vector2f(xPosition, yPosition));
	}
}
void Menu::setUpControlsPage(sf::Font &font, sf::Vector2f size) {
	Back = sf::Text("<- Back", font);
	Back.setFillColor(txtColor);
	Back.setOrigin(Back.getLocalBounds().getSize() / 2.0f);
	Back.setPosition(body.getPosition().x - 3.0f,//(center back btn:) +Back.getLocalBounds().getSize().x/2.0f
		body.getPosition().y - 9.0 - body.getSize().y / 2.0f + ((0.5) * (1.0 / Options.size()) * body.getSize().y));
	unsigned int fontSize = 12; //default font size for the controls page
	if (size.x != 300.0f && size.x == size.y) {
		fontSize *= (size.x / 300.0f); //gives us 18 when size is 450
	}
	ControlsPage = sf::Text("Right Shift:   Lock/Unlock camera"
		"\n/:             Center Camera"
		"\nSpace:         Attack"
		"\nArrow Keys:    Movement"
		"\nNumpad 0:      Dash"
		"\nP:             Pause"
		"\nR:             Reset game", font, fontSize);
	//you can't preemptively set the size/bounds?
	ControlsPage.setFillColor(txtColor);
	ControlsPage.setPosition(body.getPosition().x - body.getSize().x / 2.0f + size.x * 0.1, //basically 10% margin
		body.getPosition().y - body.getSize().y / 2.0f + Back.getLocalBounds().getSize().y * 4);
}

PauseMenu::PauseMenu(sf::RenderWindow& window, sf::View& view, sf::Font& font, const sf::String& name,
	sf::Vector2f size, sf::Vector2f pos, sf::Color bgColor) :
	Menu(window, view, font, name, size, pos, bgColor) {
	{
		//initialize the options
		Unpause = sf::Text("Unpause [P]", font);
		Hide = sf::Text("Hide [H]", font);
		Controls = sf::Text("Controls", font);
		Quit = sf::Text("Quit", font);
		//add the options to the vector
		Options.push_back(&menuName); //I may want to change or remove this later
		Options.push_back(&Unpause);
		Options.push_back(&Hide);
		Options.push_back(&Controls);
		Options.push_back(&Quit);
		//for loop that positions the options
		Menu::placeOptions();

		UnselectableOptionIndexes.push_back(0);
		
		setUpControlsPage(font, size);

		//Text labels when the player selects "Quit"
		QuitQuestion = sf::Text("Are you sure?", font);
		Yes = sf::Text("Yes", font);
		No = sf::Text("No", font);
	}
}
void PauseMenu::readMenuInputs(sf::RenderWindow& window, sf::Event& event, float deltaTime) {
	Menu::readMenuInputs(window, event);
	if (active == true && isControlPressed == false && isHidden == false)
		for (int i = 0; i < Options.size(); i++) {
			if (isMouseHovering(Options.at(i)->getPosition(), Options.at(i)->getLocalBounds().getSize(), window)) {
				if (event.type == sf::Event::MouseButtonPressed) {
					if (event.mouseButton.button == sf::Mouse::Left) {
						std::string labelName = Options.at(i)->getString();
						//std::cout << "\n" << labelName << " clicked\n";
						if (!labelName.compare("Controls")) {
							isControlPressed = true;
						}
						if (!labelName.compare("Unpause [P]")) { toggleActive(); }
						if (!labelName.compare("Hide [H]")) {
							isHidden = true;
						}
						if (!labelName.compare("Quit")) {
							while (Options.size() != 0) { //maybe dangerous
								Options.erase(Options.begin());
							}
							Options.push_back(&QuitQuestion);
							Options.push_back(&Yes);
							Options.push_back(&No);
							placeOptions();
							break;
						}
						if (!labelName.compare("Yes")) window.close();
						if (!labelName.compare("No")) {
							while (Options.size() != 0) { //maybe dangerous
								Options.erase(Options.begin());
							}
							Options.push_back(&menuName);
							Options.push_back(&Unpause);
							Options.push_back(&Hide);
							Options.push_back(&Controls);
							Options.push_back(&Quit);
							placeOptions();
							break;
						}
					}
				}
			}
		}
	else if (isControlPressed == true) {
		if (isMouseHovering(Back.getPosition(), Back.getLocalBounds().getSize(), window)) {
			Back.setFillColor(sf::Color::White);
			if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Left) {
					Back.setFillColor(txtColor);
					isControlPressed = false;
				}
			}
		}
		else Back.setFillColor(txtColor);
	}
}
void PauseMenu::draw(sf::RenderWindow& window) {
	if(isHidden) return;
	if (!isControlPressed) Menu::draw(window);
	else {
		window.draw(body);
		window.draw(Back);
		window.draw(ControlsPage);
	}
}
void PauseMenu::setPosition(sf::Vector2f position) {
	Menu::setPosition(position);
	isControlPressed = false; //when the menu changes position, we go back to the default page
	Back.setPosition(body.getPosition().x - 3.0f,//(center back btn:) +Back.getLocalBounds().getSize().x/2.0f
		body.getPosition().y - 9.0 - body.getSize().y / 2.0f + ((0.5) * (1.0 / Options.size()) * body.getSize().y));
	ControlsPage.setPosition(body.getPosition().x - body.getSize().x / 2.0f + body.getSize().x * 0.1,
		body.getPosition().y - body.getSize().y / 2.0f + Back.getLocalBounds().getSize().y * 3);
}

StartMenu::StartMenu(sf::RenderWindow& window, sf::View& view, sf::Font& font, const sf::String& name,
	sf::Vector2f size, sf::Vector2f pos, sf::Color bgColor, sf::Color txtColor) :
	Menu(window, view, font, name, size, pos, bgColor, txtColor) {
	//initialize the options/  Controls, About, Quit
	Controls = sf::Text("Controls", font);
	About = sf::Text("About", font);
	Quit = sf::Text("Quit", font);
	Options.push_back(&menuName);
	Options.push_back(&Controls);
	Options.push_back(&About);
	Options.push_back(&Quit);

	placeOptions(txtColor);

	setUpControlsPage(font, size);

	//Text labels when the player selects "Quit"
	QuitQuestion = sf::Text("Are you sure?", font);
	Yes = sf::Text("Yes", font);
	No = sf::Text("No", font);

	AboutPage = sf::Text(
		"Neoninja © 2025  Kevin I."
		"\nPatch 0.10"
		"\n"
		"\nReach the portal to"
		"\nComplete the level", font, 16);
	AboutPage.setFillColor(sf::Color::Yellow);
	AboutPage.setPosition(body.getPosition().x - body.getSize().x / 2.0f + size.x * 0.1,
		body.getPosition().y - body.getSize().y / 2.0f + Back.getLocalBounds().getSize().y * 4);
}
void StartMenu::readMenuInputs(sf::RenderWindow& window, sf::Event& event, float deltaTime) {
	Menu::readMenuInputs(window, event, 0.0f, txtColor);
	static float switchTime = 0.0; //this is to prevent one MouseClick being processed mult times
	switchTime += deltaTime;

	if (active == true && isControlPressed == false && isAboutPressed==false) 
		for (int i = 0; i < Options.size(); i++) {
			if (isMouseHovering(Options.at(i)->getPosition(), Options.at(i)->getLocalBounds().getSize(), window)) {
				if (event.type == sf::Event::MouseButtonPressed) {
					if (event.mouseButton.button == sf::Mouse::Left) {
						if (switchTime >= 0.25f) {
							std::string labelName = Options.at(i)->getString();
							if (!labelName.compare("Start")) {
								active = false;
								switchTime = 0.0f;
								break;
							}
							if (!labelName.compare("Controls")) {
								isControlPressed = true;
								switchTime = 0.0f;
							}
							if (!labelName.compare("About")) {
								isAboutPressed = true;
								switchTime = 0.0f;
							}
							if (!labelName.compare("Quit")) {
								while (Options.size() != 0) { 
									Options.erase(Options.begin());
								}
								Options.push_back(&QuitQuestion);
								Options.push_back(&Yes);
								Options.push_back(&No);
								placeOptions(txtColor);
								switchTime = 0.0f;
								break;
							}
							if (!labelName.compare("Yes")) window.close();
							if (!labelName.compare("No")) {
								while (Options.size() != 0) { //maybe dangerous
									Options.erase(Options.begin());
								}
								Options.push_back(&menuName);
								Options.push_back(&Controls);
								Options.push_back(&About);
								Options.push_back(&Quit);
								placeOptions(txtColor);
								switchTime = 0.0f;
								break;
							}
						}
					}
				}
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))) {
				active = false;
			}
		}
	else if ((isControlPressed == true) || (isAboutPressed == true)) {
		if (isMouseHovering(Back.getPosition(), Back.getLocalBounds().getSize(), window)) {
			Back.setFillColor(sf::Color::White);
			if (event.type == sf::Event::MouseButtonPressed) {
				if(switchTime>=0.25)
				if (event.mouseButton.button == sf::Mouse::Left) {
					Back.setFillColor(txtColor);
					isControlPressed = false;
					isAboutPressed = false;
					switchTime = 0.0f;
				}
			}
		}
		else Back.setFillColor(txtColor);
	}
}
void StartMenu::draw(sf::RenderWindow& window) {
	if (!isControlPressed && !isAboutPressed) Menu::draw(window);
	else {
		window.draw(body);
		window.draw(Back);
	}
	if(isControlPressed) window.draw(ControlsPage);
	if (isAboutPressed) window.draw(AboutPage);
	
}

DeathMenu::DeathMenu(sf::RenderWindow& window, sf::View& view, sf::Font& font, const sf::String& name,
	sf::Vector2f size, sf::Vector2f pos, sf::Color bgColor, sf::Color txtColor):
	Menu(window, view, font, name, size, pos, bgColor, txtColor){

	Revive = sf::Text("Revive", font);
	Restart = sf::Text("Restart", font);
	Quit = sf::Text("Quit", font);

	Options.push_back(&menuName);
	Options.push_back(&Revive);
	Options.push_back(&Restart);
	Options.push_back(&Quit);

	UnselectableOptionIndexes.push_back(0);

	quitQuestion = sf::Text("Are you sure?", font);
	Yes = sf::Text("Yes", font);
	No = sf::Text("No", font);

	placeOptions(sf::Color::Red);
}

void DeathMenu::readMenuInputs(sf::RenderWindow& window, sf::Event& event, float deltaTime, sf::Color optionsColor) {
	Menu::readMenuInputs(window, event, deltaTime, optionsColor);
	// if(active)....
	static float delayTime = 0.0; //this is to prevent one MouseClick being process mult times
	delayTime += deltaTime;
	for (int i = 0; i < Options.size(); i++) {
		if (isMouseHovering(Options.at(i)->getPosition(), Options.at(i)->getLocalBounds().getSize(), window)) {
			if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Left) {
					std::string labelName = Options.at(i)->getString();
					if (!labelName.compare("Restart")){
						restartPressed = true; //active = false;
					} // at the moment, it's more convenient to toggleAtive in the main function
					// since "Restart" can be triggered by both click and keypress
					if(delayTime>=0.25)
					if (!labelName.compare("Quit")) {
						while (Options.size() != 0)
							Options.erase(Options.begin());
						Options.push_back(&quitQuestion);
						Options.push_back(&Yes);
						Options.push_back(&No);
						delayTime = 0.0f;
					}
					if (!labelName.compare("Yes")) {
						window.close();
					}
					if (delayTime >= 0.25)
					if (!labelName.compare("No")) {
						while (Options.size() != 0)
							Options.erase(Options.begin());
						Options.push_back(&menuName);
						Options.push_back(&Revive);
						Options.push_back(&Restart);
						Options.push_back(&Quit);
						delayTime = 0.0f;
					}
					if (!labelName.compare("Revive")) {
						revivePressed = true;
						//active = false;
					}
				}		
			}
		}
	}
}

LevelMenu::LevelMenu(sf::RenderWindow& window, sf::View& view, sf::Font& font, const sf::String& name,
	sf::Vector2f size, sf::Vector2f pos, sf::Color bgColor, sf::Color txtColor) :
	Menu(window, view, font, name, size, pos, bgColor, txtColor) {

	Time = sf::Text("Time: ", font);
	Score = sf::Text("Score: 0", font);
	Advance = sf::Text("Advance to next level", font);
	Restart = sf::Text("Restart", font);
	Quit = sf::Text("Quit", font);

	Options.push_back(&menuName);
	Options.push_back(&Score);
	Options.push_back(&Time);
	Options.push_back(&Advance);
	Options.push_back(&Restart);
	Options.push_back(&Quit);

	UnselectableOptionIndexes.push_back(0);
	UnselectableOptionIndexes.push_back(1);
	UnselectableOptionIndexes.push_back(2);

	quitQuestion = sf::Text("Are you sure?", font);
	Yes = sf::Text("Yes", font);
	No = sf::Text("No", font);

	Back = sf::Text("<- Back", font);
	advancePage = sf::Text("I only made one level lmaoo", font, 22);
	//^I don't need to set their fill colors, it'll be done later with placeOptions()

	placeOptions(sf::Color::Yellow); //Color here(in the constructor) does not matter, 
								//bc the color is changed every time Menu::setPosition() is called
}

void LevelMenu::readMenuInputs(sf::RenderWindow& window, sf::Event& event, float deltaTime, sf::Color optionsColor) {
	Menu::readMenuInputs(window, event, deltaTime, txtColor);
	for (int i = 0; i < Options.size(); i++) {
		if (isMouseHovering(Options.at(i)->getPosition(), Options.at(i)->getLocalBounds().getSize(), window)) {
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) { //interesting, didn't need the event.type
				std::string labelName = Options.at(i)->getString();
				//if advance clicked  //if (sf::Mouse::isButtonPressed(sf::Mouse::Left)
				if (!labelName.compare("Advance to next level")) {
					//remove existing options from options vector
					while (Options.size() != 0) {
						Options.erase(Options.begin());
					}
					Options.push_back(&Back);
					Options.push_back(&advancePage);
					// add back and advance page to the options vector
					placeOptions(txtColor);
					while (UnselectableOptionIndexes.size() != 0)
						UnselectableOptionIndexes.erase(UnselectableOptionIndexes.begin());
					UnselectableOptionIndexes.push_back(1);
				}
				//if back clicked
				if (!labelName.compare("<- Back")) {
					resetOptions();
				}
				//if restart clicked
				if (!labelName.compare("Restart")) {
					restartPressed = true; //active = false;
				} //rn it's more convenient to toggle active from the main fnc
				//if quit clicked
				if (!labelName.compare("Quit")) {
					while (Options.size() != 0)
						Options.erase(Options.begin());
					Options.push_back(&quitQuestion);
					Options.push_back(&Yes);
					Options.push_back(&No);
					while (UnselectableOptionIndexes.size() != 0)
						UnselectableOptionIndexes.erase(UnselectableOptionIndexes.begin());
					UnselectableOptionIndexes.push_back(0);
				}
				if (!labelName.compare("Yes")) {
					window.close();
				}
				if (!labelName.compare("No")) {
					resetOptions();
				}
			}
		}
	}
}
void LevelMenu::resetOptions() {
	while (Options.size() != 0)
		Options.erase(Options.begin());
	Options.push_back(&menuName);
	Options.push_back(&Score);
	Options.push_back(&Time);
	Options.push_back(&Advance);
	Options.push_back(&Restart);
	Options.push_back(&Quit);
	placeOptions(txtColor);
	while (UnselectableOptionIndexes.size() != 0)
		UnselectableOptionIndexes.erase(UnselectableOptionIndexes.begin());
	UnselectableOptionIndexes.push_back(0);
	UnselectableOptionIndexes.push_back(1);
	UnselectableOptionIndexes.push_back(2);
}
//int main() {
//	sf::RenderWindow window(sf::VideoMode(650, 650), "Sandbox", sf::Style::Close | sf::Style::Resize);
//	sf::View view(window.getView().getCenter(), sf::Vector2f(VIEW_HEIGHT, VIEW_HEIGHT));
//
//	sf::Font font;
//	if (!font.loadFromFile("assets/RobotoMono-Regular.ttf")) { std::cout << "yeah no lmao"; }
//
//	PauseMenu pauseMenu(window, view, font, "Pause Menu");
//
//	while (window.isOpen()) {
//		sf::Event event;
//		while (window.pollEvent(event)) {
//			switch (event.type) {
//			case sf::Event::Closed:
//				window.close();
//				std::cout << "size: "; pauseMenu.printLabelSize();
//				break;
//
//			case sf::Event::Resized:
//				sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
//				window.setView(sf::View(visibleArea));
//				view.setSize(event.size.width, event.size.height);
//				//view.setCenter(event.size.width / 2.0, event.size.height / 2.0); //just added 
//				window.setView(view);
//				std::cout << "view size: " << view.getSize().x << " " << view.getSize().y;
//				std::cout << "window size: " << window.getSize().x << " " << window.getSize().y;
//				break;
//			}
//			if (event.type == sf::Event::MouseButtonPressed) {
//				if (event.mouseButton.button == sf::Mouse::Left) {
//					std::cout << "mouse position is " <<
//						sf::Mouse::getPosition().x << " " << sf::Mouse::getPosition().y;
//					//<<std::endl;
//					pauseMenu.printLabelPos();
//					// center of the screen using mouse (screen) coordinates is 960, 540
//					// center of the screen using view coordinates is 325, 325, although the
//					 //y axis might be thrown off a bit by the title bar
//					std::cout << "view center: " << view.getCenter().x << " " << view.getCenter().y;
//
//				}
//			}
//			if (pauseMenu.isActive() == true) {
//				pauseMenu.readMenuInputs(window, event);
//			}
//		}
//
//		window.clear();
//		pauseMenu.draw(window);
//		window.display();
//	}
//}