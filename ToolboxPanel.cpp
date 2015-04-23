#include "ToolboxPanel.h"
#include "CrimeScene.h"
#include "Utils.h"

#include <cavelib/LayoutManagers/TableLayout.h>
#include <cavelib/LayoutManagers/FlowLayout.h>
#include <cavelib/Components/Panel.h>
#include <cavelib/Components/Label.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/*
Author: Bas Rops - 25-04-2014
Last edit: <name> - dd-mm-yyyy
*/
class ToolboxTopPanel : public Panel
{
public:
	ToolboxTopPanel() : Panel(new TableLayoutManager(1)) {}

	virtual float minWidth()
	{
		return 3.0f;
	}

	virtual float minHeight()
	{
		return 1.0f;
	}
};

/*
Author: Bas Rops - 25-04-2014
Last edit: <name> - dd-mm-yyyy
*/
class ToolboxButtonPanel : public Panel
{
public:
	ToolboxButtonPanel() : Panel(new TableLayoutManager(3)) {}

	virtual float minWidth()
	{
		return 2.0f;
	}

	virtual float minHeight()
	{
		return 1.0f;
	}
};

/*
Author: Bas Rops - 25-04-2014
Last edit: <name> - dd-mm-yyyy
*/
class NextToolboxButton : public Button
{
	CrimeScene* crimeScene;

public:
	NextToolboxButton(CrimeScene* crimeScene, std::string name) : Button(name, fastdelegate::MakeDelegate(this, &NextToolboxButton::click))
	{
		this->crimeScene = crimeScene;
	}

	void setText(std::string txt)
	{
		text = txt;
	}

	void click()
	{

	}
};

/*
Author: Bas Rops - 25-04-2014
Last edit: <name> - dd-mm-yyyy
*/
class PreviousToolboxButton : public Button
{
	CrimeScene* crimeScene;

public:
	PreviousToolboxButton(CrimeScene* crimeScene, std::string name) : Button(name, fastdelegate::MakeDelegate(this, &PreviousToolboxButton::click))
	{
		this->crimeScene = crimeScene;
	}

	void setText(std::string txt)
	{
		text = txt;
	}

	void click()
	{

	}
};

/*
Constructor
Author: Bas Rops - 25-04-2014
Last edit: <name> - dd-mm-yyyy
*/
ToolboxPanel::ToolboxPanel(CrimeScene* crimeScene) : GUIPanel("")
{
	this->crimeScene = crimeScene;
	rootPanel = new Panel(new FlowLayoutManager());

	topPanel = new ToolboxTopPanel();

	rootPanel->add(topPanel);

	descriptionLabels = std::vector<Label*>();
	//addDefaultComponents();

	rootPanel->setFont(font);
	rootPanel->reposition(0, 0, 0.6f, 0.9f);

	renderMatrix = glm::mat4();
	renderMatrix = glm::translate(renderMatrix, glm::vec3(-1.5f, -1.0f, -2.0f));
}

/*
Destructor
Author: Bas Rops - 25-04-2014
Last edit: <name> - dd-mm-yyyy
*/
ToolboxPanel::~ToolboxPanel()
{
	for each (Label* label in descriptionLabels)
	{
		delete label;
	}
	descriptionLabels.clear();

	delete nextToolboxButton;
	delete previousToolboxButton;
	delete buttonPanel;
	delete topPanel;
}

/*
Update the position of the panel to always be in front of the user's head
Author: Bas Rops - 25-04-2014
Last edit: <name> - dd-mm-yyyy
*/
void ToolboxPanel::updatePosition(PositionalDevice* head)
{
	renderMatrix = head->getData();
	renderMatrix = glm::scale(renderMatrix, glm::vec3(1, -1, 1));

	renderMatrix = glm::rotate(renderMatrix, -90.0f, glm::vec3(0, 0, 1));
	renderMatrix = glm::translate(renderMatrix, glm::vec3(-1.5f, -0.5f, -2.0f));
}

/*
Add the default ToolboxPanel components to the panel
Author: Bas Rops - 25-04-2014
Last edit: <name> - dd-mm-yyyy
*/
void ToolboxPanel::addDefaultComponents()
{
	buttonPanel = new ToolboxButtonPanel();
	topPanel->add(buttonPanel);
	buttonPanel->add(nextToolboxButton = new NextToolboxButton(crimeScene, "Toevoegen aan inventaris"));
	buttonPanel->add(previousToolboxButton = new PreviousToolboxButton(crimeScene, "Volgende tool"));
}

/*
Set the description of the panel
Author: Bas Rops - 25-04-2014
Last edit: <name> - dd-mm-yyyy
*/
void ToolboxPanel::setDescription(std::string newDescription)
{
	topPanel->clear();

	std::vector<std::string> words = Utils::splitString(newDescription, " ");
	std::vector<std::string> lines = std::vector<std::string>();

	std::string tmpString = std::string();
	int lineLength = 60;
	for (unsigned int i = 0; i < words.size(); i++)
	{
		int newLength = i == words.size() - 1 ? tmpString.length() + words[i].length() : tmpString.length() + words[i].length() + 1;
		if (newLength <= lineLength)
		{
			tmpString += i == words.size() - 1 ? words[i] : words[i] + " ";
		}
		else
		{
			for (unsigned int j = 0; j < lineLength - tmpString.length(); j++)
				tmpString += std::string(" ");

			lines.push_back(tmpString);
			tmpString.clear();
			tmpString += i == words.size() - 1 ? words[i] : words[i] + " ";
		}

		if (i == words.size() - 1 && !tmpString.empty())
		{
			for (unsigned int j = 0; j < lineLength - tmpString.length(); j++)
				tmpString += std::string(" ");

			lines.push_back(tmpString);
			tmpString.clear();
		}
	}

	for each (std::string line in lines)
	{
		topPanel->add(new Label(line));
	}
	//addDefaultComponents();
	rootPanel->setFont(font);
	rootPanel->reposition(0, 0, 0.6f, 0.9f);
}