//
// Copyright (c) 2008-2020 the Urho3D project.
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

/// \file

#pragma once

#include "../IO/MemoryBuffer.h"
#include "../IO/VectorBuffer.h"
#include "../Replica/LocalClockSynchronizer.h"
#include "../Replica/NetworkTime.h"
#include "../Replica/ProtocolMessages.h"

#include <EASTL/optional.h>
#include <EASTL/unordered_set.h>
#include <EASTL/bonus/ring_buffer.h>

namespace Urho3D
{

class AbstractConnection;
class Network;
class NetworkObject;
class NetworkManagerBase;
class Scene;
struct NetworkSetting;

/// Maintains scene clocks of replica on the Client side.
/// Note: it also keeps physical world synchronized.
class URHO3D_API ClientReplicaClock : public Object
{
public:
    ClientReplicaClock(Scene* scene, AbstractConnection* connection, const MsgSceneClock& initialClock,
        const VariantMap& serverSettings);
    ~ClientReplicaClock();

    /// Return constant properties of replica.
    /// @{
    Scene* GetScene() const { return scene_; }
    AbstractConnection* GetConnection() const { return connection_; }
    unsigned GetConnectionId() const { return thisConnectionId_; }
    unsigned GetUpdateFrequency() const { return updateFrequency_; }
    double SecondsToFrames(double value) const { return value * updateFrequency_; }
    /// @}

    /// Return dynamic properties of replica. Return values may vary over time.
    /// @{
    unsigned GetInputDelay() const { return inputDelay_; }
    const Variant& GetSetting(const NetworkSetting& setting) const;
    /// @}

    /// Return prediced exact server time.
    NetworkTime GetServerTime() const { return serverTime_; }

    /// Return replica interpolation time which is always behind server time.
    /// Scene is expected to be exactly replicated at replica time.
    NetworkTime GetReplicaTime() const { return replicaTime_.Get(); }

    /// Return time at which ongoing client input will be processed on server.
    /// Input time is always ahead of server.
    NetworkTime GetInputTime() const { return inputTime_.Get(); }
    bool IsNewInputFrame() const { return synchronizedPhysicsTick_.has_value(); }
    NetworkTime GetLatestScaledInputTime() const { return latestScaledInputTime_; }
    /// Return which physical tick of this frame corresponds to the beginning of the network frame.
    ea::optional<unsigned> GetSynchronizedPhysicsTick() const { return synchronizedPhysicsTick_; }

protected:
    /// Apply elapsed timestep and accumulated clock updates.
    /// Returns possibly scaled (dilated or contracted) timestep.
    float ApplyTimeStep(float timeStep, const ea::vector<MsgSceneClock>& pendingClockUpdates);

    Scene* const scene_{};
    AbstractConnection* const connection_{};

private:
    void UpdateServerTime(const MsgSceneClock& msg, bool skipOutdated);

    NetworkTime ToClientTime(const NetworkTime& serverTime) const;
    NetworkTime ToInputTime(const NetworkTime& serverTime) const;

    const VariantMap serverSettings_;

    const unsigned thisConnectionId_{};
    const unsigned updateFrequency_{};
    const float timeSnapThreshold_{};
    const float timeErrorTolerance_{};
    const float minTimeDilation_{};
    const float maxTimeDilation_{};

    unsigned inputDelay_{};
    NetworkTime serverTime_;
    unsigned latestServerFrame_{};
    NetworkTime latestScaledInputTime_{};
    ea::optional<unsigned> synchronizedPhysicsTick_{};

    SoftNetworkTime replicaTime_;
    SoftNetworkTime inputTime_;
    PhysicsClockSynchronizer physicsSync_;
};

/// Client part of NetworkManager subsystem.
/// TODO(network): Rename to ClientReplica
class URHO3D_API ClientNetworkManager : public ClientReplicaClock
{
    URHO3D_OBJECT(ClientNetworkManager, ClientReplicaClock);

public:
    ClientNetworkManager(Scene* scene, AbstractConnection* connection, const MsgSceneClock& initialClock,
        const VariantMap& serverSettings);

    void ProcessMessage(NetworkMessageId messageId, MemoryBuffer& messageData);

    ea::string GetDebugInfo() const;

    /// Return global properties of client state.
    /// @{
    unsigned GetTraceCapacity() const;
    unsigned GetPositionExtrapolationFrames() const;
    /// @}

private:
    void SynchronizeClocks(float timeStep);
    void UpdateReplica(float timeStep);
    void SendObjectsFeedbackUnreliable(unsigned feedbackFrame);

    NetworkObject* CreateNetworkObject(NetworkId networkId, StringHash componentType, bool isOwned);
    NetworkObject* GetCheckedNetworkObject(NetworkId networkId, StringHash componentType);
    void RemoveNetworkObject(WeakPtr<NetworkObject> networkObject);

    void ProcessSceneClock(const MsgSceneClock& msg);
    void ProcessRemoveObjects(MemoryBuffer& messageData);
    void ProcessAddObjects(MemoryBuffer& messageData);
    void ProcessUpdateObjectsReliable(MemoryBuffer& messageData);
    void ProcessUpdateObjectsUnreliable(MemoryBuffer& messageData);

    Network* network_{};
    NetworkManagerBase* base_{};
    Scene* scene_{};
    AbstractConnection* connection_{};

    ea::vector<MsgSceneClock> pendingClockUpdates_;
    ea::unordered_set<WeakPtr<NetworkObject>> ownedObjects_;

    VectorBuffer componentBuffer_;
};

}