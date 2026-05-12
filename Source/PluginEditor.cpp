#include "PluginEditor.h"

namespace
{
const juce::Colour backgroundTop { 0xff151922 };
const juce::Colour backgroundBottom { 0xff242b31 };
const juce::Colour panel { 0xff20262d };
const juce::Colour panelLight { 0xff2b333b };
const juce::Colour accent { 0xffff8a3d };
const juce::Colour accent2 { 0xff70c8ff };
}

SGMDelayAudioProcessorEditor::SGMDelayAudioProcessorEditor (SGMDelayAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    title.setText ("SGM Delay", juce::dontSendNotification);
    title.setFont (juce::Font (25.0f, juce::Font::bold));
    title.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (title);

    subtitle.setText ("Clean echo, tone, motion, and output controls", juce::dontSendNotification);
    subtitle.setFont (juce::Font (13.0f));
    subtitle.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible (subtitle);

    const std::array<juce::String, static_cast<size_t> (Group::count)> groupNames {
        "Delay", "Tone", "Motion", "Output"
    };

    for (size_t i = 0; i < groupLabels.size(); ++i)
    {
        groupLabels[i].setText (groupNames[i], juce::dontSendNotification);
        groupLabels[i].setFont (juce::Font (15.0f, juce::Font::bold));
        groupLabels[i].setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.92f));
        groupLabels[i].setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (groupLabels[i]);
    }

    addKnob ("mix", "Mix", Group::delay, true);
    addKnob ("delayTimeMs", "Msec", Group::delay, true);
    addKnob ("feedback", "Feedback", Group::delay, true);
    addKnob ("spread", "Spread", Group::delay);

    timeDivisionLabel.setText ("Time", juce::dontSendNotification);
    timeDivisionLabel.setJustificationType (juce::Justification::centred);
    timeDivisionLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.88f));
    timeDivisionLabel.setFont (juce::Font (14.0f, juce::Font::bold));
    addAndMakeVisible (timeDivisionLabel);

    timeDivisionBox.addItemList ({ "8/1", "4/1", "Msec", "1/2", "1/4", "1/8", "1/16", "1/8T", "1/8D" }, 1);
    timeDivisionBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff242b32));
    timeDivisionBox.setColour (juce::ComboBox::outlineColourId, accent.withAlpha (0.65f));
    timeDivisionBox.setColour (juce::ComboBox::textColourId, juce::Colours::white);
    timeDivisionBox.setColour (juce::ComboBox::arrowColourId, accent);
    timeDivisionAttachment = std::make_unique<ComboBoxAttachment> (processor.apvts, "timeDivision", timeDivisionBox);
    addAndMakeVisible (timeDivisionBox);

    addKnob ("lowCut", "Low Cut", Group::tone);
    addKnob ("highCut", "High Cut", Group::tone);

    addKnob ("modRate", "Rate", Group::motion);
    addKnob ("modDepth", "Depth", Group::motion);

    addKnob ("width", "Width", Group::output);
    addKnob ("ducking", "Ducking", Group::output);

    pingPongButton.setButtonText ("Ping Pong");
    pingPongButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    pingPongButton.setColour (juce::ToggleButton::tickColourId, accent2);
    pingPongAttachment = std::make_unique<ButtonAttachment> (processor.apvts, "pingPong", pingPongButton);
    addAndMakeVisible (pingPongButton);

    freezeButton.setButtonText ("Freeze");
    freezeButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    freezeButton.setColour (juce::ToggleButton::tickColourId, accent2);
    freezeAttachment = std::make_unique<ButtonAttachment> (processor.apvts, "freeze", freezeButton);
    addAndMakeVisible (freezeButton);

    pingPongLabel.setText ("Stereo bounce", juce::dontSendNotification);
    pingPongLabel.setFont (juce::Font (12.0f));
    pingPongLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.62f));
    pingPongLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (pingPongLabel);

    freezeLabel.setText ("Buffer hold", juce::dontSendNotification);
    freezeLabel.setFont (juce::Font (12.0f));
    freezeLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.62f));
    freezeLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (freezeLabel);

    setResizable (true, true);
    setResizeLimits (720, 500, 1200, 780);
    setSize (900, 560);
}

void SGMDelayAudioProcessorEditor::addKnob (const juce::String& parameterId, const juce::String& text, Group group, bool primary)
{
    auto* knob = knobs.add (new Knob());
    knob->slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    knob->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, primary ? 94 : 82, 22);
    knob->slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
    knob->slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff39414a));
    knob->slider.setColour (juce::Slider::thumbColourId, juce::Colours::white);
    knob->slider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    knob->slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x00000000));
    knob->slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0x00000000));
    knob->attachment = std::make_unique<SliderAttachment> (processor.apvts, parameterId, knob->slider);
    knob->group = group;
    knob->primary = primary;

    knob->label.setText (text, juce::dontSendNotification);
    knob->label.setJustificationType (juce::Justification::centred);
    knob->label.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.88f));
    knob->label.setFont (juce::Font (primary ? 14.0f : 13.0f, juce::Font::bold));

    addAndMakeVisible (knob->slider);
    addAndMakeVisible (knob->label);
}

void SGMDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient gradient (backgroundTop, 0.0f, 0.0f, backgroundBottom, 0.0f, static_cast<float> (getHeight()), false);
    g.setGradientFill (gradient);
    g.fillAll();

    auto bounds = getLocalBounds().reduced (18);
    g.setColour (panel);
    g.fillRoundedRectangle (bounds.toFloat(), 8.0f);

    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawRoundedRectangle (bounds.toFloat(), 8.0f, 1.0f);

    g.setColour (accent.withAlpha (0.85f));
    g.fillRect (bounds.removeFromTop (3));

    for (size_t i = 0; i < groupBounds.size(); ++i)
    {
        if (groupBounds[i].isEmpty())
            continue;

        auto groupArea = groupBounds[i].toFloat();
        g.setColour (i == static_cast<size_t> (Group::delay) ? panelLight : juce::Colour (0xff242b32));
        g.fillRoundedRectangle (groupArea, 7.0f);

        g.setColour (juce::Colours::white.withAlpha (0.07f));
        g.drawRoundedRectangle (groupArea, 7.0f, 1.0f);
    }
}

void SGMDelayAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (30);
    auto header = area.removeFromTop (96);
    auto toggleArea = header.removeFromRight (188);
    title.setBounds (header.removeFromTop (36));
    subtitle.setBounds (header);

    auto pingPongArea = toggleArea.removeFromTop (34);
    pingPongButton.setBounds (pingPongArea.withTrimmedLeft (20));
    pingPongLabel.setBounds (toggleArea.removeFromTop (18).withTrimmedLeft (20));
    freezeButton.setBounds (toggleArea.removeFromTop (34).withTrimmedLeft (20));
    freezeLabel.setBounds (toggleArea.withTrimmedLeft (20));

    area.removeFromTop (8);

    groupBounds[static_cast<size_t> (Group::delay)] = area.removeFromTop (176);
    area.removeFromTop (10);

    auto bottom = area;
    const auto outputWidth = juce::jmax (190, bottom.getWidth() / 4);
    groupBounds[static_cast<size_t> (Group::output)] = bottom.removeFromRight (outputWidth);
    bottom.removeFromRight (10);
    groupBounds[static_cast<size_t> (Group::motion)] = bottom.removeFromRight (bottom.getWidth() / 2);
    bottom.removeFromRight (10);
    groupBounds[static_cast<size_t> (Group::tone)] = bottom;

    layoutDelayGroup (groupBounds[static_cast<size_t> (Group::delay)]);
    layoutGroup (Group::tone, groupBounds[static_cast<size_t> (Group::tone)], 2);
    layoutGroup (Group::motion, groupBounds[static_cast<size_t> (Group::motion)], 2);
    layoutGroup (Group::output, groupBounds[static_cast<size_t> (Group::output)], 2);
}

void SGMDelayAudioProcessorEditor::layoutDelayGroup (juce::Rectangle<int> bounds)
{
    auto content = bounds.reduced (14);
    groupLabels[static_cast<size_t> (Group::delay)].setBounds (content.removeFromTop (24));
    content.removeFromTop (4);

    constexpr auto columns = 5;
    const auto cellW = content.getWidth() / columns;
    const auto cellH = content.getHeight();

    auto timeCell = juce::Rectangle<int> (content.getX() + cellW, content.getY(), cellW, cellH).reduced (6);
    timeDivisionLabel.setBounds (timeCell.removeFromTop (22));
    timeCell.reduce (8, 18);
    timeDivisionBox.setBounds (timeCell.removeFromTop (34));

    juce::Array<Knob*> delayKnobs;
    for (auto* knob : knobs)
        if (knob->group == Group::delay)
            delayKnobs.add (knob);

    const std::array<int, 4> knobColumns { 0, 2, 3, 4 };
    for (int i = 0; i < delayKnobs.size() && i < static_cast<int> (knobColumns.size()); ++i)
    {
        auto cell = juce::Rectangle<int> (content.getX() + knobColumns[static_cast<size_t> (i)] * cellW, content.getY(), cellW, cellH).reduced (6);
        delayKnobs[i]->label.setBounds (cell.removeFromTop (22));
        delayKnobs[i]->slider.setBounds (cell);
    }
}

void SGMDelayAudioProcessorEditor::layoutGroup (Group group, juce::Rectangle<int> bounds, int columns)
{
    auto content = bounds.reduced (14);
    groupLabels[static_cast<size_t> (group)].setBounds (content.removeFromTop (24));
    content.removeFromTop (4);

    juce::Array<Knob*> groupKnobs;
    for (auto* knob : knobs)
    {
        if (knob->group == group)
            groupKnobs.add (knob);
    }

    columns = juce::jlimit (1, juce::jmax (1, groupKnobs.size()), columns);
    const auto rows = static_cast<int> (std::ceil (static_cast<double> (groupKnobs.size()) / static_cast<double> (columns)));
    const auto cellW = content.getWidth() / columns;
    const auto cellH = content.getHeight() / juce::jmax (1, rows);

    for (int i = 0; i < groupKnobs.size(); ++i)
    {
        const auto row = i / columns;
        const auto col = i % columns;
        auto cell = juce::Rectangle<int> (content.getX() + col * cellW, content.getY() + row * cellH, cellW, cellH).reduced (6);
        const auto labelHeight = groupKnobs[i]->primary ? 22 : 20;

        groupKnobs[i]->label.setBounds (cell.removeFromTop (labelHeight));
        groupKnobs[i]->slider.setBounds (cell);
    }
}
