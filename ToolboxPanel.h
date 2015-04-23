#pragma once
#include <cavelib/GUIPanel.h>
#include <cavelib/Components/Button.h>

class NextToolboxButton;
class PreviousToolboxButton;
class CrimeScene;
class Label;
class PositionalDevice;

class ToolboxPanel : public GUIPanel
{
	CrimeScene* crimeScene;
	Panel* topPanel;
	Panel* buttonPanel;
	std::vector<Label*> descriptionLabels;
	NextToolboxButton* nextToolboxButton;
	PreviousToolboxButton* previousToolboxButton;

public:
	ToolboxPanel(CrimeScene* crimeScene);
	~ToolboxPanel();

	void updatePosition(PositionalDevice* head);
	void addDefaultComponents();
	void setDescription(std::string newDescription);
};

