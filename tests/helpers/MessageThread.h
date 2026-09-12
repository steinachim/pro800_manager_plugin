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

#include <juce_events/juce_events.h>

namespace TestMessages
{
    /** A message thread for the duration of a test: timers and async updates run on it while runFor() is called. */
    struct MessageThread
    {
        MessageThread() { juce::MessageManager::getInstance(); }

        ~MessageThread()
        {
            juce::DeletedAtShutdown::deleteAll(); // the timer thread's shutdown detector, like shutdownJuce_GUI() does
            juce::MessageManager::deleteInstance();
        }

        void runFor (int milliseconds)
        {
            juce::MessageManager::getInstance()->runDispatchLoopUntil (milliseconds);
        }
    };
}
