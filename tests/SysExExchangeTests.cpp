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

#include "helpers/MessageThread.h"
#include "helpers/TestMessages.h"

#include "midi/SysExExchange.h"
#include "midi/VersionMessage.h"

#include <juce_events/juce_events.h>

using namespace TestMessages;

namespace
{
    struct Harness
    {
        MessageThread messageThread;
        std::vector<std::vector<uint8_t>> sent;
        std::vector<bool> sentAsPolling;
        SysExExchange exchange { [this] (const juce::MidiMessage& message, bool isPolling) {
            sent.push_back (bytesOf (message));
            sentAsPolling.push_back (isPolling);
        } };

        /** Completions in the order they happened; nullptr replies are recorded as empty vectors. */
        std::vector<std::pair<juce::String, std::vector<uint8_t>>> completed;

        SysExExchange::Request request (const juce::String& description, SysExMatchers::Matcher matcher, int timeoutMs = 20, int retries = 1)
        {
            SysExExchange::Request r;
            r.message = VersionMessage::request();
            r.matches = std::move (matcher);
            r.timeoutMs = timeoutMs;
            r.retries = retries;
            r.description = description;
            r.onComplete = [this, description] (const juce::MidiMessage* reply) {
                completed.emplace_back (description, reply != nullptr ? bytesOf (*reply) : std::vector<uint8_t>());
            };
            return r;
        }
    };

    const auto versionReply = sysEx ({ VersionMessage::RESPONSE_ID, 0x00, 1, 4, 6 });
}

//==============================================================================
TEST_CASE ("SysExExchange: a request is sent once and completed by the first matching reply", "[midi][exchange]")
{
    Harness h;
    h.exchange.enqueue (h.request ("version", SysExMatchers::isVersionReply));

    REQUIRE (h.sent.size() == 1);
    REQUIRE (h.sent[0] == bytesOf (VersionMessage::request()));
    REQUIRE (h.exchange.isBusy());

    SECTION ("a reply that does not match is left for others and the request stays in flight")
    {
        REQUIRE_FALSE (h.exchange.offerInbound (toMidi (statusReply (0x00))));
        REQUIRE (h.completed.empty());
        REQUIRE (h.exchange.isBusy());
    }

    REQUIRE (h.exchange.offerInbound (toMidi (versionReply)));
    REQUIRE (h.completed.size() == 1);
    REQUIRE (h.completed[0].second == versionReply);
    REQUIRE_FALSE (h.exchange.isBusy());
    REQUIRE (h.sent.size() == 1); // no resend

    SECTION ("nothing is in flight afterwards, so a late duplicate is not consumed")
    {
        REQUIRE_FALSE (h.exchange.offerInbound (toMidi (versionReply)));
    }
}

TEST_CASE ("SysExExchange: a lost request is sent again and the retry's reply completes it", "[midi][exchange]")
{
    Harness h;
    h.exchange.enqueue (h.request ("version", SysExMatchers::isVersionReply, 100, 1));
    REQUIRE (h.sent.size() == 1);

    // past the first timeout; the retry's own timeout cannot run while the loop is not
    REQUIRE (h.messageThread.runUntil ([&h] { return h.sent.size() == 2; }, 2000));
    REQUIRE (h.completed.empty());

    REQUIRE (h.exchange.offerInbound (toMidi (versionReply)));
    REQUIRE (h.completed.size() == 1);
    REQUIRE (h.completed[0].second == versionReply);
}

TEST_CASE ("SysExExchange: after the last retry the request completes with no reply", "[midi][exchange]")
{
    Harness h;
    h.exchange.enqueue (h.request ("version", SysExMatchers::isVersionReply, 10, 3));

    REQUIRE (h.messageThread.runUntil ([&h] { return h.completed.size() == 1; }, 2000));
    REQUIRE (h.sent.size() == 4); // the first attempt plus three retries
    REQUIRE (h.completed[0].second.empty());
    REQUIRE_FALSE (h.exchange.isBusy());

    h.messageThread.runFor (50);
    REQUIRE (h.sent.size() == 4); // and nothing more once it has given up

    SECTION ("zero retries means exactly one attempt")
    {
        h.exchange.enqueue (h.request ("once", SysExMatchers::isVersionReply, 10, 0));
        REQUIRE (h.messageThread.runUntil ([&h] { return h.completed.size() == 2; }, 2000));
        REQUIRE (h.sent.size() == 5);
    }
}

TEST_CASE ("SysExExchange: requests go out one at a time, in order", "[midi][exchange]")
{
    Harness h;
    h.exchange.enqueue (h.request ("first", SysExMatchers::isVersionReply));
    h.exchange.enqueue (h.request ("second", SysExMatchers::isStatusReply));
    h.exchange.enqueue (h.request ("third", SysExMatchers::isVersionReply));

    REQUIRE (h.sent.size() == 1); // the second waits for the first

    // the status reply belongs to the second request, which is not in flight yet: not consumed
    REQUIRE_FALSE (h.exchange.offerInbound (toMidi (statusReply (0x00))));

    REQUIRE (h.exchange.offerInbound (toMidi (versionReply)));
    REQUIRE (h.sent.size() == 2);
    REQUIRE (h.exchange.offerInbound (toMidi (statusReply (0x00))));
    REQUIRE (h.sent.size() == 3);
    REQUIRE (h.exchange.offerInbound (toMidi (versionReply)));

    REQUIRE (h.completed.size() == 3);
    REQUIRE (h.completed[0].first == "first");
    REQUIRE (h.completed[1].first == "second");
    REQUIRE (h.completed[2].first == "third");
    REQUIRE_FALSE (h.exchange.isBusy());
}

TEST_CASE ("SysExExchange: a completion callback may enqueue the next request", "[midi][exchange]")
{
    Harness h;

    SysExExchange::Request chained = h.request ("first", SysExMatchers::isVersionReply);
    chained.onComplete = [&h] (const juce::MidiMessage* reply) {
        h.completed.emplace_back ("first", reply != nullptr ? bytesOf (*reply) : std::vector<uint8_t>());
        h.exchange.enqueue (h.request ("chained", SysExMatchers::isStatusReply));
    };
    h.exchange.enqueue (std::move (chained));

    REQUIRE (h.exchange.offerInbound (toMidi (versionReply)));
    REQUIRE (h.sent.size() == 2);
    REQUIRE (h.exchange.isBusy());
    REQUIRE (h.exchange.offerInbound (toMidi (statusReply (0x00))));
    REQUIRE (h.completed.size() == 2);
    REQUIRE (h.completed[1].first == "chained");
}

TEST_CASE ("SysExExchange: cancelling completes everything pending with no reply", "[midi][exchange]")
{
    Harness h;
    h.exchange.enqueue (h.request ("in flight", SysExMatchers::isVersionReply, 1000));
    h.exchange.enqueue (h.request ("queued", SysExMatchers::isVersionReply, 1000));

    h.exchange.cancelAll();
    REQUIRE (h.completed.size() == 2);
    REQUIRE (h.completed[0].first == "in flight");
    REQUIRE (h.completed[0].second.empty());
    REQUIRE (h.completed[1].first == "queued");
    REQUIRE_FALSE (h.exchange.isBusy());
    REQUIRE (h.sent.size() == 1); // the queued one never went out

    // the cancelled timeout must not fire afterwards
    h.messageThread.runFor (30);
    REQUIRE (h.completed.size() == 2);
    REQUIRE_FALSE (h.exchange.offerInbound (toMidi (versionReply)));
}

TEST_CASE ("SysExExchange: the polling flag reaches the sender", "[midi][exchange]")
{
    Harness h;
    auto poll = h.request ("poll", SysExMatchers::isVersionReply);
    poll.isPolling = true;
    h.exchange.enqueue (std::move (poll));
    h.exchange.enqueue (h.request ("user", SysExMatchers::isVersionReply));

    REQUIRE (h.exchange.offerInbound (toMidi (versionReply)));
    REQUIRE (h.exchange.offerInbound (toMidi (versionReply)));
    REQUIRE (h.sentAsPolling == std::vector<bool> { true, false });
}
