#ifndef __PHYSICS_EVENTS_HPP__
#define __PHYSICS_EVENTS_HPP__

#include "core/utility/Observable.hpp"
#include "core/entities/Soldier.hpp"

namespace Soldank
{
struct PhysicsEvents
{
    Observable<Soldier&, float /* damage */> soldier_hit_by_bullet;
};
} // namespace Soldank

#endif
