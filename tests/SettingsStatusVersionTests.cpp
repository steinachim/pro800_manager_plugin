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

#include <catch2/catch_test_macros.hpp>

#include "helpers/TestMessages.h"

#include "midi/SettingsMessage.h"
#include "midi/StatusMessage.h"
#include "midi/VersionMessage.h"
#include "tailoring/Pro800SettingsConstants.h"

using namespace TestMessages;

//==============================================================================
TEST_CASE ("SettingsMessage: only the exact dump size is valid", "[midi][settings]")
{
    REQUIRE (SettingsMessage (toMidi (settingsDump())).isValid());

    auto tooShort = settingsDump();
    tooShort.erase (tooShort.end() - 2); // drop one data byte, keep F7
    REQUIRE_FALSE (SettingsMessage (toMidi (tooShort)).isValid());

    auto tooLong = settingsDump();
    tooLong.insert (tooLong.end() - 1, 0x00);
    REQUIRE_FALSE (SettingsMessage (toMidi (tooLong)).isValid());
}

TEST_CASE ("SettingsMessage: signed and unsigned fields", "[midi][settings]")
{
    SettingsMessage settings (toMidi (settingsDump()));

    settings.setValue (Pro800Settings::TRANSPOSE, -12);
    REQUIRE (settings.getValue (Pro800Settings::TRANSPOSE) == -12);

    settings.setValue (Pro800Settings::TRANSPOSE, 12);
    REQUIRE (settings.getValue (Pro800Settings::TRANSPOSE) == 12);

    settings.setValue (Pro800Settings::SYNC_CLOCK_BPM, 1205); // 2 bytes, 120.5 BPM
    REQUIRE (settings.getValue (Pro800Settings::SYNC_CLOCK_BPM) == 1205);

    settings.setValue (Pro800Settings::MIDI_RX_CHANNEL, SETTINGS_MIDI_RX_16);
    REQUIRE (settings.getValue (Pro800Settings::MIDI_RX_CHANNEL) == SETTINGS_MIDI_RX_16);

    REQUIRE (allDataBytesAre7Bit (settings.getRawData()));
    REQUIRE (settings.toString().contains ("Transpose: 12"));
}

TEST_CASE ("SettingsMessage: the selection pointer is Current Preset Number plus Current Bank", "[midi][settings]")
{
    REQUIRE (PRO800_SETTINGS_FIELDS.at (Pro800Settings::CURRENT_BANK).firstByte == 23);
    REQUIRE (PRO800_SETTINGS_FIELDS.at (Pro800Settings::PRESET_NUM).firstByte == 6);

    SettingsMessage settings (toMidi (settingsDump()));
    const auto before = settings.getRawData();

    SECTION ("selecting writes the full program number and the bank, and nothing else")
    {
        settings.setCurrentProgram (143); // B43, as the synth itself stores it
        REQUIRE (settings.getValue (Pro800Settings::PRESET_NUM) == 143);
        REQUIRE (settings.getValue (Pro800Settings::CURRENT_BANK) == 1);
        REQUIRE (settings.getCurrentProgram() == 143);
        REQUIRE (allDataBytesAre7Bit (settings.getRawData()));

        const auto after = settings.getRawData();
        REQUIRE (after.size() == before.size());
        int changedBytes = 0;
        for (size_t i = 0; i < before.size(); i++)
        {
            changedBytes += (before[i] != after[i]) ? 1 : 0;
        }
        REQUIRE (changedBytes == 3); // the number's low byte, its high bit in the overflow byte at 0, and the bank
    }

    SECTION ("only the number modulo 100 selects the slot; the bank comes from its own field")
    {
        settings.setValue (Pro800Settings::PRESET_NUM, 560); // accepted and kept by the synth, which shows D60
        settings.setValue (Pro800Settings::CURRENT_BANK, 3);
        REQUIRE (settings.getCurrentProgram() == 360);

        settings.setValue (Pro800Settings::PRESET_NUM, 0);
        settings.setValue (Pro800Settings::CURRENT_BANK, 1); // measured: B00, not A00
        REQUIRE (settings.getCurrentProgram() == 100);
    }

    SECTION ("an invalid message or an impossible bank has no pointer")
    {
        settings.setValue (Pro800Settings::CURRENT_BANK, 4);
        REQUIRE_FALSE (settings.getCurrentProgram().has_value());

        auto tooShort = settingsDump();
        tooShort.erase (tooShort.end() - 2);
        REQUIRE_FALSE (SettingsMessage (toMidi (tooShort)).getCurrentProgram().has_value());
    }

    SECTION ("every program number round-trips")
    {
        for (int program : { 0, 99, 100, 199, 200, 299, 300, 399 })
        {
            settings.setCurrentProgram (program);
            REQUIRE (settings.getCurrentProgram() == program);
        }
    }
}

//==============================================================================
TEST_CASE ("StatusMessage: known and unknown status bytes", "[midi][status]")
{
    SECTION ("OK and Error")
    {
        StatusMessage ok (toMidi (sysEx ({ StatusMessage::RESPONSE_ID, 0x00, 0x00 })));
        REQUIRE (ok.isValid());
        REQUIRE (ok.getStatus() == StatusMessage::STATUS_OK);
        REQUIRE (ok.toString().contains ("OK"));

        StatusMessage error (toMidi (sysEx ({ StatusMessage::RESPONSE_ID, 0x00, 0x01 })));
        REQUIRE (error.getStatus() == StatusMessage::STATUS_ERROR);
        REQUIRE (error.toString().contains ("Error"));
    }

    SECTION ("an unknown status byte is reported, not thrown")
    {
        StatusMessage unknown (toMidi (sysEx ({ StatusMessage::RESPONSE_ID, 0x00, 0x05 })));
        REQUIRE (unknown.isValid());
        REQUIRE_NOTHROW (unknown.toString());
        REQUIRE (unknown.toString().contains ("Unknown status"));
    }

    SECTION ("a status message without the status byte is invalid")
    {
        // F0 <header> 01 00 F7: index 10 (the status byte) is the F7
        StatusMessage truncated (toMidi (sysEx ({ StatusMessage::RESPONSE_ID, 0x00 })));
        REQUIRE_FALSE (truncated.isValid());
        REQUIRE (truncated.getStatus() == StatusMessage::STATUS_INVALID);
    }

    SECTION ("the wrong message type is not a status message")
    {
        StatusMessage notAStatus (toMidi (sysEx ({ VersionMessage::RESPONSE_ID, 0x00, 0x00 })));
        REQUIRE_FALSE (notAStatus.isValid());
    }
}

//==============================================================================
TEST_CASE ("VersionMessage: firmware version parsing", "[midi][version]")
{
    SECTION ("supported version")
    {
        VersionMessage version (toMidi (sysEx ({ VersionMessage::RESPONSE_ID, 0x00, 1, 4, 6 })));
        REQUIRE (version.isValid());
        REQUIRE (version.getVersionString() == "1.4.6");
        REQUIRE (version.isSupported());
        REQUIRE (version.toString().contains ("(supported)"));
    }

    SECTION ("unsupported version")
    {
        VersionMessage version (toMidi (sysEx ({ VersionMessage::RESPONSE_ID, 0x00, 1, 4, 5 })));
        REQUIRE (version.isValid());
        REQUIRE (version.getVersionString() == "1.4.5");
        REQUIRE_FALSE (version.isSupported());
        REQUIRE (version.getSupportedVersions() == "1.4.6");
    }

    SECTION ("a reply missing the last version byte is invalid")
    {
        // F0 <header> 09 00 01 04 F7: index 12 (version byte 3) is the F7
        VersionMessage truncated (toMidi (sysEx ({ VersionMessage::RESPONSE_ID, 0x00, 1, 4 })));
        REQUIRE_FALSE (truncated.isValid());
        REQUIRE (truncated.getVersionString() == "UNKNOWN VERSION");
        REQUIRE_FALSE (truncated.isSupported());
    }
}
