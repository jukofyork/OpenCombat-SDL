# OpenCombat SDL - Asset Reorganization Plan (Final)

## Executive Summary

Clean break migration to hierarchical, inheritance-based asset organization. Three separate inheritance chains (weapons/units/vehicles) with composition in squads. No backward compatibility.

**Philosophy:**
- Weapons define equipment
- Units define people (infantry, crew)  
- Vehicles define platforms (armor, mounted weapons, crew positions)
- Squads compose all of the above

---

## Final Directory Structure

```
OpenCombat-SDL/
│
├── weapons/                    # Equipment - personal and mounted
│   ├── base.xml                # Root: weight, icon, sounds
│   ├── personal/               # Carried by units
│   │   ├── firearm/            # Base for guns
│   │   │   ├── rifle/
│   │   │   │   └── m1-garand/
│   │   │   │       └── weapon.xml    # <Weapon parent="../../rifle">
│   │   │   ├── smg/
│   │   │   │   └── thompson/
│   │   │   ├── lmg/
│   │   │   │   └── bar/
│   │   │   └── hmg/
│   │   │       └── mg-30cal/
│   │   ├── launcher/
│   │   │   └── bazooka/
│   │   └── explosives/
│   │       ├── grenade/
│   │       └── smoke/
│   │
│   └── mounted/                # Vehicle-mounted only
│       ├── tank-gun/
│       │   ├── 75mm-l48/
│       │   └── 88mm-kwk36/
│       ├── hull-mg/
│       └── coax-mg/
│
├── units/                      # Individual people
│   ├── base.xml                # Root: health, basic abilities
│   │
│   ├── infantry/               # Foot soldiers
│   │   ├── base.xml            # Movement: walk, run, crawl
│   │   ├── rifle/
│   │   │   ├── unit.xml        # <Unit parent="../base.xml">
│   │   │   ├── portrait.tga
│   │   │   └── variants/
│   │   │       ├── elite/
│   │   │       │   └── unit.xml    # <Unit parent="../unit.xml">
│   │   │       └── winter/
│   │   ├── thompson/
│   │   ├── bar/
│   │   ├── bazooka/
│   │   └── mg/
│   │
│   └── crew/                   # Vehicle crew (with sidearms)
│       ├── base.xml
│       ├── tank-commander/
│       ├── tank-gunner/
│       ├── tank-loader/
│       └── tank-driver/
│
├── vehicles/                   # Platforms requiring crew
│   ├── base.xml                # Root: armor, speed, mobility type
│   │
│   ├── tracked/                # Tank, tank destroyer
│   │   ├── base.xml            # Rotation rate, ground pressure
│   │   └── tank/
│   │       ├── base.xml        # Turret mechanics
│   │       └── panzer-ivg/
│   │           ├── vehicle.xml # <Vehicle parent="../base.xml">
│   │           ├── hull.tga
│   │           ├── turret.tga
│   │           └── wreck.tga
│   │
│   └── wheeled/                # Trucks, jeeps, halftracks
│       ├── truck/
│       │   └── opel-blitz/
│       └── jeep/
│           └── willys-mb/
│
├── squads/                     # Organizational compositions
│   ├── base.xml                # Formation defaults, spacing
│   │
│   ├── infantry/               # Infantry-only squads
│   │   ├── rifle/
│   │   │   └── squad.xml       # <Squad parent="../base.xml">
│   │   ├── weapons/
│   │   └── assault/
│   │
│   ├── vehicle/                # Vehicle + crew
│   │   ├── tank/
│   │   │   └── panzer-ivg/
│   │   │       └── squad.xml   # References vehicle + crew units
│   │   └── antitank/
│   │
│   └── mixed/                  # Combined arms (future)
│       └── panzer-grenadier/
│
├── common/                     # Shared graphics, sounds, animations
│   ├── animations/
│   ├── sounds/
│   ├── effects/
│   └── ui/
│
├── maps/
│   └── Acqueville/
│       ├── map.xml
│       ├── terrain.txt
│       └── graphics/
│
└── system/                     # Global game data
    ├── game.xml
    ├── actions.xml
    ├── terrain/
    └── nationalities.xml
```

---

## Inheritance Model

### Three Separate Chains

**1. Weapons** (equipment characteristics)
```
weapons/base.xml
  └── personal/firearm/base.xml
        └── rifle/base.xml
              └── m1-garand/weapon.xml
```

**2. Units** (person characteristics)
```
units/base.xml
  └── infantry/base.xml
        └── rifle/unit.xml
              └── variants/elite/unit.xml
```

**3. Vehicles** (platform characteristics)
```
vehicles/base.xml
  └── tracked/base.xml
        └── tank/base.xml
              └── panzer-ivg/vehicle.xml
```

### No Cross-Inheritance

- Weapons never inherit from units or vehicles
- Units never inherit from vehicles (different concerns)
- Vehicles never inherit from units

---

## XML Schema Examples

### 1. Weapon Definition

**Path:** `weapons/personal/firearm/rifle/m1-garand/weapon.xml`

```xml
<?xml version="1.0" encoding="utf-8"?>
<Weapon parent="../../rifle">
    <Name>M1 Garand</Name>
    
    <Combat>
        <RoundsPerClip>8</RoundsPerClip>
        <Range>460</Range>
        <Accuracy>1.0</Accuracy>
    </Combat>
    
    <Timing>
        <Fire>400</Fire>
        <ReloadClip>8000</ReloadClip>
    </Timing>
    
    <Assets>
        <Icon>icon.tga</Icon>
        <Sound>garand.wav</Sound>
    </Assets>
</Weapon>
```

### 2. Unit Definition (Infantry)

**Path:** `units/infantry/rifle/unit.xml`

```xml
<?xml version="1.0" encoding="utf-8"?>
<Unit parent="../base.xml">
    <Name>Rifle Soldier</Name>
    
    <Equipment>
        <PrimaryWeapon>weapons/personal/firearm/rifle/m1-garand</PrimaryWeapon>
        <Clips>4</Clips>
    </Equipment>
    
    <Animations>
        <Variant>rifle</Variant>
    </Animations>
</Unit>
```

**Variant Example:**

**Path:** `units/infantry/rifle/variants/elite/unit.xml`

```xml
<?xml version="1.0" encoding="utf-8"?>
<Unit parent="../../unit.xml">
    <Name>Elite Rifleman</Name>
    
    <Vitals>
        <Morale base="70"/>  <!-- Override: was 50 -->
    </Vitals>
    
    <Equipment>
        <Clips>6</Clips>  <!-- Override: was 4 -->
    </Equipment>
</Unit>
```

### 3. Vehicle Definition

**Path:** `vehicles/tracked/tank/panzer-ivg/vehicle.xml`

```xml
<?xml version="1.0" encoding="utf-8"?>
<Vehicle parent="../base.xml">
    <Name>Panzer IV Ausf. G</Name>
    
    <Armor>
        <Front>80</Front>
        <Side>30</Side>
        <Rear>20</Rear>
    </Armor>
    
    <Performance>
        <MaxSpeed>10.55</MaxSpeed>
        <Acceleration>1.32</Acceleration>
    </Performance>
    
    <Weapons>
        <Mount id="0" type="turret">
            <Weapon>weapons/mounted/tank-gun/75mm-l48</Weapon>
        </Mount>
        <Mount id="1" type="hull">
            <Weapon>weapons/mounted/hull-mg/mg34</Weapon>
        </Mount>
    </Weapons>
    
    <Crew>
        <Position id="driver" required="true" weapon="-1"/>
        <Position id="gunner" required="true" weapon="0"/>
        <Position id="loader" required="true" weapon="0"/>
        <Position id="commander" required="true" weapon="-1"/>
    </Crew>
    
    <Graphics>
        <Hull>hull.tga</Hull>
        <Turret>turret.tga</Turret>
        <Wreck>wreck.tga</Wreck>
    </Graphics>
    
    <Transform type="OnDestroyed">
        <SpawnCrewAs>units/crew/tank-dismounted</SpawnCrewAs>
    </Transform>
</Vehicle>
```

### 4. Squad Definition (Infantry Only)

**Path:** `squads/infantry/rifle/squad.xml`

```xml
<?xml version="1.0" encoding="utf-8"?>
<Squad parent="../base.xml">
    <Name>Rifle Squad</Name>
    
    <Composition>
        <Member unit="units/infantry/thompson" role="leader"/>
        <Member unit="units/infantry/bar" role="mg" count="2"/>
        <Member unit="units/infantry/rifle" count="5"/>
    </Composition>
    
    <Formation default="wedge" spacing="30"/>
</Squad>
```

### 5. Squad Definition (Vehicle + Crew)

**Path:** `squads/vehicle/tank/panzer-ivg/squad.xml`

```xml
<?xml version="1.0" encoding="utf-8"?>
<Squad parent="../../base.xml">
    <Name>Panzer IVG Crew</Name>
    
    <Vehicle>vehicles/tracked/tank/panzer-ivg</Vehicle>
    
    <CrewAssignment>
        <Assign position="driver" unit="units/crew/tank-driver"/>
        <Assign position="gunner" unit="units/crew/tank-gunner"/>
        <Assign position="loader" unit="units/crew/tank-loader"/>
        <Assign position="commander" unit="units/crew/tank-commander" isLeader="true"/>
    </CrewAssignment>
</Squad>
```

---

## Key Design Decisions

### 1. Three Separate Inheritance Chains

*Why weapons/units/vehicles don't inherit from each other:*
- Different concerns (equipment vs people vs platforms)
- Prevents confusion (tank doesn't inherit from infantryman)
- Clear boundaries for modders

### 2. Hierarchical Within Categories

*Why not flat like `units/infantry-rifle/`?*
- Variants need clear parent relationship
- `units/infantry/rifle/variants/elite/` shows inheritance chain
- Easier to browse related units

### 3. Vehicles Are Separate, Not Units

*Why not `units/vehicle-panzer-iv/`?*
- Vehicles have fundamentally different properties (armor, mounted weapons)
- Crew are units, vehicle is platform they occupy
- Enables bail-out: vehicle dies, crew become separate infantry

### 4. Squads Compose Everything

*Why squads reference both units AND vehicles?*
- Natural organizational structure
- Tank platoon = 4 × (vehicle + crew)
- Mixed squads possible (infantry + vehicle support)

### 5. Crew Are Units

*Why not define crew inline in vehicle?*
- Crew can bail out and fight independently
- Enables vehicle capture (assign new crew units)
- Crew have skills/experience that persist

---

## Reference System

### Path Resolution

All references are **relative to root** or **relative to parent**:

- `../../rifle` → traverse up 2 levels
- `weapons/personal/firearm/rifle/m1-garand` → absolute from root
- `../base.xml` → sibling in parent directory

### Discovery

1. Scan each top-level folder recursively
2. Load `base.xml` files first (inheritance roots)
3. Load leaf files, resolve parent references
4. Fail fast on missing references

---

## Alternative Approaches Considered

### Option A: Flat Structure with Prefixes

```
units/
  infantry-rifle/
  infantry-mg/
  weapon-m1-garand/
  squad-rifle/
  vehicle-panzer-iv/
```

*Why rejected:* Variants became unclear. `infantry-rifle-elite/` doesn't show it's derived from `infantry-rifle/`. Hierarchical structure makes inheritance explicit.

### Option B: Vehicles as Units

```
units/
  infantry-rifle/
  vehicle-panzer-iv/          # Vehicle treated as unit type
```

*Why rejected:* Forces artificial inheritance. Vehicle has armor, mounted weapons, crew positions - fundamentally different from infantry. Also breaks bail-out mechanics (vehicle "dying" would need to transform into crew, messy with single inheritance chain).

### Option C: Assets Included in Units

```
units/
  infantry-rifle/
    unit.xml
    animations/               # Graphics included
    sounds/
```

*Why rejected:* Massive duplication. 6 soldier types × 15 animations = 90 copies. Better to share in `common/` with variant selection.

### Option D: Pure ECS with Components

```
data/
  components/
    movement/
    combat/
    equipment/
  entities/
    infantry-rifle/           # Just references components
```

*Why rejected:* Too abstract for modders. "Compose movement + combat + equipment" is harder to understand than "inherit from rifleman". Good for code architecture, bad for modding accessibility.

### Option E: Vehicles as Equipment

```
squads/
  tank-squad/
    squad.xml
    equipment/
      panzer-iv.xml           # Vehicle treated as squad equipment
```

*Why rejected:* Vehicles are too complex for equipment slot. Need crew management, destruction handling, abandonment mechanics. Equipment model doesn't fit.

### Why We Chose the Current Structure

- **Clear mental model:** Weapons are equipment, units are people, vehicles are platforms
- **Natural inheritance:** Each chain has logical parent-child relationships
- **Bail-out supported:** Vehicle destruction → spawn crew units (different XML files)
- **Capture supported:** Assign new crew units to existing vehicle
- **Modder-friendly:** Can look at any folder and understand what it is
- **Scales well:** Add new weapon type → add to weapons/. Add new unit → add to units/. No ambiguity.

---

## Breaking Changes

- **Old saves:** Will not load (different structure)
- **Old mods:** Will not load (different paths)
- **Old code:** Path loading logic needs complete rewrite

Acceptable: Pre-alpha, no user base to break

---

## Summary

**Structure:**
- Three inheritance chains: weapons, units, vehicles
- Hierarchical within each chain
- Squads compose from all three
- Clear separation of concerns

**Benefits:**
- Modders add content via inheritance
- Variants easy (override specific properties)
- Vehicle bail-out/capture supported
- Mixed squads possible
- No ambiguity about what inherits from what

**Ready to implement**
