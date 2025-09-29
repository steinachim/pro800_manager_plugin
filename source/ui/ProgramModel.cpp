#include "ProgramModel.h"

#include "../midi/ProgramMessage.h"

ProgramModel::ProgramModel() : juce::ListBoxModel()
{
}

ProgramModel::~ProgramModel()
{
}

int ProgramModel::getNumRows()
{
    return rows.size();
}

juce::String ProgramModel::getNameForRow (int rowNumber)
{
    if (rows.size() > rowNumber)
    {
        return rows[rowNumber]->getProgramBankNumber() + ": " + rows[rowNumber]->getProgramName();
    }

    return juce::String();
}

std::shared_ptr<ProgramMessage> ProgramModel::getProgramForRow(int rowNumber)
{
    if (rows.size() > rowNumber)
    {
        return rows[rowNumber];
    }

    return std::shared_ptr<ProgramMessage>();
}

void ProgramModel::paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
    {
        g.fillAll (juce::Colours::lightblue);
    }

    juce::AttributedString s;
    s.setWordWrap (juce::AttributedString::none);
    s.setJustification (juce::Justification::centredLeft);
    s.append (getNameForRow (rowNumber), juce::Colours::white);
    s.draw (g, juce::Rectangle<int> (width, height).expanded (-4, 50).toFloat());
}

void ProgramModel::listBoxItemDoubleClicked (int row, const juce::MouseEvent &/*event*/)
{
    if (rows.size() <= row)
    {
        return;
    }

    auto programMessage = rows[row];
    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Program Details", programMessage->toString());
}

void ProgramModel::clear()
{
    rows.clear();
}

void ProgramModel::addElement (std::shared_ptr<ProgramMessage> message)
{
    rows.add (message);
}