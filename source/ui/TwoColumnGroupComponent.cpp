#include "TwoColumnGroupComponent.h"

#include <cassert>

TwoColumnGroupComponent::TwoColumnGroupComponent (const juce::String& title, int rows)
{
    this->numRows = rows;
    this->setText(title);
}

TwoColumnGroupComponent::~TwoColumnGroupComponent()
{
    for ( auto *component : this->componentsToDelete )
    {
        delete component;
    }
    
    this->leftComponents.clear();
    this->rightComponents.clear();
    this->componentsToDelete.clear();
}

void TwoColumnGroupComponent::addComponentPairs(const juce::Array<juce::Component *> &leftColumn, const juce::Array<juce::Component *> &rightColumn)
{
    assert(leftColumn.size() == rightColumn.size());

    this->numRows = (numRows == -1 ? leftColumn.size() : numRows);

    this->leftComponents.addArray(leftColumn);
    this->rightComponents.addArray(rightColumn);

    for ( auto *component : leftComponents )
    {
        addAndMakeVisible(component);
    }

    for ( auto *component : rightColumn )
    {
        addAndMakeVisible(component);
    }
}

void TwoColumnGroupComponent::addLabelledComponents(const juce::Array<juce::String> &labelTexts, const juce::Array<juce::Component *> &componentArray)
{
    juce::Array<juce::Component *> labelArray;
    for ( const auto &text : labelTexts )
    {
        juce::Label *label = new juce::Label { "", text};
        componentsToDelete.add( label );
        labelArray.add( label );
    }

    addComponentPairs(labelArray, componentArray);
}


void TwoColumnGroupComponent::resized()
{
    auto area = getLocalBounds().reduced(15);
    int groupHeight = area.getHeight() / this->numRows;
    
    auto leftArea = area.withTrimmedRight(area.getWidth()/2);
    auto rightArea = area.withTrimmedLeft(area.getWidth()/2);

    // left area
    for ( auto *component : this->leftComponents )
    {
        if ( component == nullptr )
        {
            leftArea.removeFromTop(groupHeight);
            continue;
        }

        component->setBounds(leftArea.removeFromTop(groupHeight).reduced(3));
    }

    // right area
    for ( auto *component : this->rightComponents )
    {
        if ( component == nullptr )
        {
            rightArea.removeFromTop(groupHeight);
            continue;
        }
            
        component->setBounds(rightArea.removeFromTop(groupHeight).reduced(3));
    }
}