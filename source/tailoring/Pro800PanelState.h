/**
 * Pro800 Manager Plugin
 * Copyright (C) 2025 Achim Stein
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/

#pragma once

#include "Pro800CCConstants.h"
#include "Pro800PanelConstants.h"
#include "Pro800ProgramConstants.h"

#include <map>
#include <optional>

/** One reading of the physical control surface: the 0x70 panel indices and the 0x72 knob positions. */
struct Pro800PanelState
{
    std::map<Pro800PanelIndex, int> panel;
    std::map<Pro800LiveIndex, int> live;
};

/**
 * A panel reading in the plugin's own terms: program fields on the stored record's scale (0-65535 for knobs, the
 * enum or bit values for switches), and CC values (0-127) for the two knobs no program field holds (Master
 * Tune and Master Volume).
 */
struct Pro800PanelValues
{
    std::map<Pro800ProgramField, int> fields;
    std::map<Pro800CCMessages, int> ccValues;
};

class Pro800PanelConversion
{
public:
    /** The 3-position Filter Keyboard switch is two bits: Off = (0,0), Half = (0,1), Full = (1,0). (1,1) cannot occur. */
    static std::optional<Pro800ProgramFilterKeyboardTracking> filterKeyboardFromBits (int full, int half);

    /**
     * The LFO shape the two-position shape switch implies, given the shape that was selected before.
     *
     * The switch has two positions - Tri/Sine/Saw and Pulse/Random/Noise - and the Performance menu decides which
     * pair of shapes it toggles between: Tri/Pulse, Sine/Random or Saw/Noise. Pro800ProgramLfoShape is ordered in
     * exactly those pairs (Pulse 0 / Triangle 1, Random 2 / Sine 3, Noise 4 / Saw 5), so the odd values are the
     * Tri/Sine/Saw side, the even ones the other, and a shape's partner is `shape ^ 1`. The previously selected
     * shape therefore names the pair and the switch picks within it: keep the shape if it is already on the
     * switch's side, otherwise flip to its partner.
     *
     * This is inferred from how the panel is laid out, not measured over MIDI - the wire only carries the one bit -
     * so it is a best effort: nullopt if no previous shape is known.
     */
    static std::optional<Pro800ProgramLfoShape> lfoShapeFromSwitch (std::optional<int> previousShape, int switchValue);

    /** A 7-bit knob reading on the 16-bit scale of the stored record: 0 -> 0, 127 -> 65535. */
    static int liveValueToProgramRange (int value);

    /** What the reading means for the plugin's controls. previousLfoShape: see lfoShapeFromSwitch(). */
    static Pro800PanelValues toValues (const Pro800PanelState& state, std::optional<int> previousLfoShape = std::nullopt);

    /**
     * The reading as CC values (0-127), ready to be sent so that the synth plays what the panel shows. Knob
     * readings are 7-bit already, so nothing is lost on the way. previousLfoShape: see lfoShapeFromSwitch() - the
     * LFO shape is left out when it cannot be resolved.
     */
    static std::map<Pro800CCMessages, int> toCCValues (const Pro800PanelState& state, std::optional<int> previousLfoShape = std::nullopt);

    /** The program field a knob index feeds, or NONE for the two master knobs (which only have a CC). */
    static Pro800ProgramField liveIndexToProgramField (Pro800LiveIndex index);
    /** The CC of a knob index; every one of the 26 has one. */
    static Pro800CCMessages liveIndexToCC (Pro800LiveIndex index);
};
