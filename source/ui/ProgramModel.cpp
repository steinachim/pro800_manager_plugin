#include "ProgramModel.h"

#include "../midi/ProgramMessage.h"

ProgramModel::ProgramModel(ModelType type, juce::ListBox *parent) : juce::ListBoxModel()
{
    this->modelType = type;
    this->parentListBox = parent;
    reset();
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
    if (rowIsSelected || rowNumber == highlightedRow)
    {
        g.fillAll (juce::Colours::lightblue);
    }

    auto program = getProgramForRow(rowNumber);
    juce::Colour textColor = juce::Colours::white;
    if ( !program || !program->isValid() )
    {
        textColor = juce::Colours::grey;
    }

    juce::AttributedString s;
    s.setWordWrap (juce::AttributedString::none);
    s.setJustification (juce::Justification::centredLeft);
    s.append (getNameForRow (rowNumber), textColor);
    s.draw (g, juce::Rectangle<int> (width, height).expanded (-4, 50).toFloat());
}

void ProgramModel::listBoxItemDoubleClicked (int row, const juce::MouseEvent &/*event*/)
{
    if (modelType == SYNTH )
    {
        return;
    }
    
    if (rows.size() <= row)
    {
        return;
    }

    auto programMessage = rows[row];

    if ( !programMessage->isValid() )
    {
        return;
    }

    this->nameChangeMessageBox = std::make_unique<juce::AlertWindow>("Preset Name Change", "Please enter the new preset name", juce::MessageBoxIconType::NoIcon);
    this->nameChangeMessageBox->addTextEditor(NAME_CHANGE_INPUT, programMessage->getProgramName());
    this->nameChangeMessageBox->addButton("OK", NameChangeResult::OK);
    this->nameChangeMessageBox->addButton("Cancel", NameChangeResult::CANCEL);

    this->nameChangeMessageBox->enterModalState(true, juce::ModalCallbackFunction::create([this, programMessage] (int modalResult) {
        this->nameChangeMessageBox->setVisible(false);

        if ( modalResult != NameChangeResult::OK)
            return;

        juce::String resultString = this->nameChangeMessageBox->getTextEditorContents(NAME_CHANGE_INPUT);
        programMessage->setProgramName(resultString.toStdString());
        parentListBox->repaintRow(programMessage->getProgramNumber());
    }));
}

void ProgramModel::deleteKeyPressed (int /*lastRowSelected*/) 
{
    if ( modelType != LOCAL )
    {
        return;
    }

    auto selectedRows = this->parentListBox->getSelectedRows();

    for( int i = 0; i < selectedRows.size(); i++ )
    {
        uint16_t row = (uint16_t)selectedRows[i];
        if ( rows.size() > row )
        {
            auto emptyMessage = std::make_shared<ProgramMessage>();
            emptyMessage->setProgramNumber(row);
            rows.set(row, emptyMessage);
            parentListBox->repaintRow(row);
        }
    }   
}


juce::var ProgramModel::getDragSourceDescription (const juce::SparseSet<int>& selectedRows)
{
    if ( modelType != LOCAL)
    {
        return juce::var();
    }

    juce::StringArray rowsIds;

    for (int i = 0; i < selectedRows.size(); ++i)
        rowsIds.add (juce::String (selectedRows[i] + 1));

    return juce::String(ProgramModel::DRAG_SOURCE_DESCRIPTION) + ":" + rowsIds.joinIntoString (",");
}

void ProgramModel::reset()
{
    rows.clear();
    for ( uint16_t i = 0; i < ProgramMessage::NUM_PROGRAMS; i++ )
    {
        auto emptyMessage = std::make_shared<ProgramMessage>();
        emptyMessage->setProgramNumber(i);
        rows.add( emptyMessage );        
    } 
    parentListBox->repaint();
}

void ProgramModel::updateElement(std::shared_ptr<ProgramMessage> message)
{
    int programNumber = message->getProgramNumber();

    if ( programNumber >= rows.size() )
    {
        std::cerr << "ProgramModel::updateElement() - received program out of range" << std::endl;
        return;
    }

    rows.set(message->getProgramNumber(), message);

    parentListBox->repaintRow(programNumber);
}

void ProgramModel::highlightRow(int row)
{
    this->highlightedRow = row;
    parentListBox->repaintRow(row);
}
