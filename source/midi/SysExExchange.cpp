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

#include "SysExExchange.h"

SysExExchange::SysExExchange (Sender newSender) : sender (std::move (newSender))
{
}

SysExExchange::~SysExExchange()
{
    // nothing is completed from here: whoever waits for a reply is being torn down as well
    stopTimer();
}

void SysExExchange::enqueue (Request request)
{
    JUCE_ASSERT_MESSAGE_THREAD

    this->queue.push_back (std::move (request));
    startNext();
}

void SysExExchange::cancelAll()
{
    JUCE_ASSERT_MESSAGE_THREAD

    stopTimer();

    // take everything first: a callback may enqueue again, and that must land in an empty exchange
    std::deque<Request> cancelled;
    cancelled.swap (this->queue);
    std::optional<Request> inFlight = std::move (this->current);
    this->current.reset();

    if (inFlight && inFlight->onComplete)
    {
        inFlight->onComplete (nullptr);
    }

    for (auto& request : cancelled)
    {
        if (request.onComplete)
        {
            request.onComplete (nullptr);
        }
    }
}

bool SysExExchange::isBusy() const
{
    return this->current.has_value() || !this->queue.empty();
}

bool SysExExchange::isCurrentPolling() const
{
    return this->current.has_value() && this->current->isPolling;
}

bool SysExExchange::offerInbound (const juce::MidiMessage& message)
{
    JUCE_ASSERT_MESSAGE_THREAD

    if (!this->current || !this->current->matches || !this->current->matches (message))
    {
        return false;
    }

    complete (&message);
    return true;
}

void SysExExchange::startNext()
{
    if (this->current || this->queue.empty())
    {
        return;
    }

    this->current = std::move (this->queue.front());
    this->queue.pop_front();
    this->attemptsLeft = 1 + juce::jmax (0, this->current->retries);
    sendAttempt();
}

void SysExExchange::sendAttempt()
{
    this->attemptsLeft--;
    this->sender (this->current->message, this->current->isPolling);
    startTimer (juce::jmax (1, this->current->timeoutMs));
}

void SysExExchange::complete (const juce::MidiMessage* reply)
{
    stopTimer();

    // hand the request over before calling back: the callback may enqueue the next one
    Request finished = std::move (*this->current);
    this->current.reset();

    if (finished.onComplete)
    {
        finished.onComplete (reply);
    }

    startNext();
}

void SysExExchange::timerCallback()
{
    stopTimer();

    if (!this->current)
    {
        return;
    }

    if (this->attemptsLeft > 0)
    {
        juce::Logger::writeToLog ("[WARNING] No reply to " + this->current->description + " - sending again (" + juce::String (this->attemptsLeft) + " attempt(s) left)");
        sendAttempt();
        return;
    }

    juce::Logger::writeToLog ("[WARNING] No reply to " + this->current->description + " - giving up");
    complete (nullptr);
}
