//
// Copyright (c) 2017-2020 the rbfx project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "../Container/Ptr.h"
#include "../Container/RefCounted.h"

#include <EASTL/fixed_function.h>
#include <EASTL/utility.h>
#include <EASTL/vector.h>

namespace Urho3D
{

/// Signal with specified or default sender type.
template <typename Signature, typename Sender = RefCounted>
class Signal;

template <typename Sender, typename... Args>
class Signal<void(Args...), Sender>
{
public:
    /// Small object optimization buffer size.
    static const unsigned HandlerSize = 4 * sizeof(void*);

    /// Signal handler type.
    using Handler = ea::fixed_function<HandlerSize, bool(RefCounted*, Sender*, Args...)>;

    /// Subscribe to event.
    template <typename Receiver, typename Callback>
    void Subscribe(Receiver* receiver, Callback handler)
    {
        handlers_.emplace_back(WeakPtr<RefCounted>(static_cast<RefCounted*>(receiver)),
            [handler](RefCounted* receiverPtr, Sender* sender, Args... args)
            {
                auto receiver = static_cast<Receiver*>(receiverPtr);
                if constexpr (ea::is_invocable_r_v<bool, Callback, Receiver*, Sender*, Args...>)
                    return (receiver->*handler)(sender, args...);
                else if constexpr (ea::is_invocable_r_v<void, Callback, Receiver*, Sender*, Args...>)
                    return (receiver->*handler)(sender, args...), true;
                else if constexpr (ea::is_invocable_r_v<bool, Callback, Receiver*, Args...>)
                    return (receiver->*handler)(args...);
                else// if constexpr (ea::is_invocable_r_v<void, Callback, Receiver*, Args...>)
                    return (receiver->*handler)(args...), true;
            }
        );
    }

    /// Unsubscribe all handlers of specified receiver from this events.
    void Unsubscribe(RefCounted* receiver)
    {
        for (auto it = handlers_.begin(); it != handlers_.end();)
        {
            ea::pair<WeakPtr<RefCounted>, Handler>& pair = *it;
            if (pair.first.Expired() || pair.first == receiver)
                it = handlers_.erase(it);
            else
                ++it;
        }
    }

    /// Invoke event.
    template <typename... InvokeArgs>
    void operator()(Sender* sender, InvokeArgs&&... args)
    {
        for (auto it = handlers_.begin(); it != handlers_.end();)
        {
            ea::pair<WeakPtr<RefCounted>, Handler>& pair = *it;
            if (RefCounted* receiver = pair.first.Get())
            {
                if (pair.second(receiver, sender, args...))
                    ++it;
                else
                    it = handlers_.erase(it);
            }
            else
                it = handlers_.erase(it);
        }
    }

    /// Returns true when event has at least one subscriber.
    bool HasSubscribers() const { return !handlers_.empty(); }

protected:
    /// A collection of event handlers.
    ea::vector<ea::pair<WeakPtr<RefCounted>, Handler>> handlers_;
};


}
