/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** NetSystems
*/

#include "engine/scheduler/Scheduler.hpp"
#include "net/Replication.hpp"
#include "systems/EngineSystems.hpp"

#include <memory>
#include <utility>

namespace engine {

void netAssignIds(SystemTargets& targets) {
    if (targets.replication) targets.replication->assignIds(targets.world);
}

void netSnapshot(SystemTargets& targets) {
    if (targets.replication) targets.replication->sendSnapshots(targets.world, targets.time);
}

void netApplySnapshot(SystemTargets& targets) {
    if (targets.replication) targets.replication->applySnapshots(targets.world, targets.time);
}

void netInterpolate(SystemTargets& targets) {
    if (targets.replication) targets.replication->interpolate(targets.world, targets.time);
}

void netSendInput(SystemTargets& targets) {
    if (targets.replication && targets.input) targets.replication->sendInput(*targets.input);
}

void netReceiveInput(SystemTargets& targets) {
    if (targets.replication) targets.replication->applyInputs(targets.world);
}

void registerNetSystems(Scheduler& scheduler, std::shared_ptr<TargetsOf> of) {
    const auto system = [of](void (*body)(SystemTargets&)) {
        return [of, body](Context& ctx) {
            SystemTargets targets = (*of)(ctx);
            body(targets);
        };
    };
    scheduler.add(Stage::FixedUpdate, "engine.net.receive_input", system(netReceiveInput)).roles(ServerRole);
    scheduler.add(Stage::FixedUpdate, "engine.net.apply_snapshot", system(netApplySnapshot)).roles(ClientRole);
    scheduler.add(Stage::LateUpdate, "engine.net.assign_ids", system(netAssignIds)).roles(ServerRole);
    scheduler.add(Stage::LateUpdate, "engine.net.snapshot", system(netSnapshot))
        .after("engine.net.assign_ids")
        .roles(ServerRole);
    scheduler.add(Stage::LateUpdate, "engine.net.send_input", system(netSendInput)).roles(ClientRole);
    scheduler.add(Stage::Render, "engine.net.interpolate", system(netInterpolate)).roles(ClientRole);
}

}
