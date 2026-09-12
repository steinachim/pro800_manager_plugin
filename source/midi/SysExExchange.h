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

#include "SysExMatchers.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>

#include <deque>
#include <functional>
#include <optional>

/**
 * Serialized request/reply exchange with the synth. Message thread only.
 *
 * The Pro-800 carries no request id, so only one request is in flight at a time: the next one goes out once the
 * current one has been answered or given up. A request is answered by the first inbound message its matcher
 * accepts (see SysExMatchers); if nothing acceptable arrives within timeoutMs it is re-sent, up to `retries`
 * times. Losses are normal rather than exceptional: the first request after opening the port is lost about one
 * time in five, a port that sat idle for seconds loses two in a row, and any channel-voice message costs the next
 * SysEx request one attempt (docs/Pro800SysExMessages.md, "Transport behaviour").
 *
 * The actual sending is delegated to the Sender passed to the constructor, so the exchange can be tested without
 * a MIDI device.
 */
class SysExExchange : private juce::Timer
{
public:
    struct Request
    {
        juce::MidiMessage message;
        SysExMatchers::Matcher matches;
        int timeoutMs = DEFAULT_TIMEOUT_MS;
        int retries = DEFAULT_RETRIES;
        bool isPolling = false; // routine background traffic: the log may hide it
        juce::String description;

        /** Called on the message thread with the reply, or with nullptr if every attempt went unanswered or the exchange was cancelled. */
        std::function<void (const juce::MidiMessage* reply)> onComplete;
    };

    /** The synth answers within a few milliseconds; a longer wait does not recover a lost request, a resend does. */
    static constexpr int DEFAULT_TIMEOUT_MS = 500;
    static constexpr int DEFAULT_RETRIES = 1;
    /** For the very first request on a port: an idle USB link loses two requests before it wakes up. */
    static constexpr int FIRST_REQUEST_RETRIES = 3;

    using Sender = std::function<void (const juce::MidiMessage& message, bool isPolling)>;

    explicit SysExExchange (Sender sender);
    ~SysExExchange() override;

    /** Queues the request; it is sent as soon as nothing else is in flight. */
    void enqueue (Request request);

    /** Gives up on everything: every pending request completes with nullptr. */
    void cancelAll();

    /** True while a request is in flight or waiting. */
    bool isBusy() const;

    /** True if the request in flight is routine polling (false if nothing is in flight). */
    bool isCurrentPolling() const;

    /** Offers a received message to the request in flight. Returns true if it was accepted as the reply. */
    bool offerInbound (const juce::MidiMessage& message);

private:
    void startNext();
    void sendAttempt();
    void complete (const juce::MidiMessage* reply);
    void timerCallback() override;

    Sender sender;
    std::deque<Request> queue;
    std::optional<Request> current;
    int attemptsLeft = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SysExExchange)
};
